/*
 * Connection.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Connection

**************************************************/

#include "Connection.hpp"
#include "Session.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

Connection::Connection(const std::shared_ptr<AsyncWebSocket>& socket,
           const std::shared_ptr<Session>& hubSession, v_int64 connectionId)
    : m_socket(socket),
      m_hubSession(hubSession),
      m_connectionId(connectionId),
      m_messageQueue(std::make_shared<MessageQueue>()),
      m_pingTime(-1),
      m_failedPings(0),
      m_lastPingTimestamp(-1) {}

oatpp::async::CoroutineStarter Connection::sendMessageAsync(
    const oatpp::Object<MessageDto>& message) {
    class SendMessageCoroutine
        : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;

    public:
        SendMessageCoroutine(oatpp::async::Lock* lock,
                             const std::shared_ptr<AsyncWebSocket>& websocket,
                             const oatpp::String& message)
            : m_lock(lock), m_websocket(websocket), m_message(message) {}

        Action act() override {
            return oatpp::async::synchronize(
                       m_lock, m_websocket->sendOneFrameTextAsync(m_message))
                .next(finish());
        }
    };

    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    if (m_socket) {
        return SendMessageCoroutine::start(
            &m_writeLock, m_socket, m_objectMapper->writeToString(message));
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::sendErrorAsync(
    const oatpp::Object<ErrorDto>& error, bool fatal) {
    class SendErrorCoroutine
        : public oatpp::async::Coroutine<SendErrorCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;
        bool m_fatal;

    public:
        SendErrorCoroutine(oatpp::async::Lock* lock,
                           const std::shared_ptr<AsyncWebSocket>& websocket,
                           const oatpp::String& message, bool fatal)
            : m_lock(lock),
              m_websocket(websocket),
              m_message(message),
              m_fatal(fatal) {}

        Action act() override {
            /* synchronized async pipeline */
            auto call = oatpp::async::synchronize(
                m_lock, m_websocket->sendOneFrameTextAsync(m_message));

            if (m_fatal) {
                return call.next(m_websocket->sendCloseAsync()).next(finish());
                //.next(new oatpp::async::Error("API Error"));
            }

            return call.next(finish());
        }
    };

    auto message = MessageDto::createShared();
    message->code = MessageCodes::OUTGOING_ERROR;
    message->payload = error;

    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    if (m_socket) {
        return SendErrorCoroutine::start(&m_writeLock, m_socket,
                                         m_objectMapper->writeToString(message),
                                         fatal);
    }

    return nullptr;
}

bool Connection::queueMessage(const oatpp::Object<MessageDto>& message) {
    class SendMessageCoroutine
        : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        std::shared_ptr<oatpp::data::mapping::ObjectMapper> m_mapper;
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        std::shared_ptr<MessageQueue> m_queue;

    public:
        SendMessageCoroutine(
            const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& mapper,
            oatpp::async::Lock* lock,
            const std::shared_ptr<AsyncWebSocket>& websocket,
            const std::shared_ptr<MessageQueue>& queue)
            : m_mapper(mapper),
              m_lock(lock),
              m_websocket(websocket),
              m_queue(queue) {}

        Action act() override {
            std::unique_lock<std::mutex> lock(m_queue->mutex);
            if (m_queue->queue.empty()) {
                m_queue->active = false;
                return finish();
            }
            auto msg = m_queue->queue.back();
            m_queue->queue.pop_back();
            lock.unlock();

            auto json = m_mapper->writeToString(msg);
            return oatpp::async::synchronize(
                       m_lock, m_websocket->sendOneFrameTextAsync(json))
                .next(repeat());
        }

        Action handleError(oatpp::async::Error* error) override {
            return yieldTo(&SendMessageCoroutine::act);
        }
    };

    if (message) {
        std::lock_guard<std::mutex> lock(m_messageQueue->mutex);
        if (m_messageQueue->queue.size() <
            m_hubSession->getConfig()->maxQueuedMessages) {
            m_messageQueue->queue.push_front(message);
            if (!m_messageQueue->active) {
                m_messageQueue->active = true;
                std::lock_guard<std::mutex> socketLock(m_socketMutex);
                if (m_socket) {
                    m_asyncExecutor->execute<SendMessageCoroutine>(
                        m_objectMapper, &m_writeLock, m_socket, m_messageQueue);
                }
            }
            return true;
        }
    }
    return false;
}

void Connection::ping(v_int64 timestampMicroseconds) {
    class PingCoroutine : public oatpp::async::Coroutine<PingCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;

    public:
        PingCoroutine(oatpp::async::Lock* lock,
                      const std::shared_ptr<AsyncWebSocket>& websocket,
                      const oatpp::String& message)
            : m_lock(lock), m_websocket(websocket), m_message(message) {}

        Action act() override {
            return oatpp::async::synchronize(
                       m_lock, m_websocket->sendOneFrameTextAsync(m_message))
                .next(finish());
        }
    };

    auto message = MessageDto::createShared(
        MessageCodes::OUTGOING_PING, oatpp::Int64(timestampMicroseconds));

    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    if (m_socket) {
        m_asyncExecutor->execute<PingCoroutine>(
            &m_writeLock, m_socket, m_objectMapper->writeToString(message));
    }
}

void Connection::kick() {
    class KickCoroutine : public oatpp::async::Coroutine<KickCoroutine> {
    private:
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        oatpp::String m_message;

    public:
        KickCoroutine(oatpp::async::Lock* lock,
                      const std::shared_ptr<AsyncWebSocket>& websocket,
                      const oatpp::String& message)
            : m_lock(lock), m_websocket(websocket), m_message(message) {}

        Action act() override {
            return oatpp::async::synchronize(
                       m_lock, m_websocket->sendOneFrameTextAsync(m_message))
                .next(yieldTo(&KickCoroutine::onMessageSent));
        }

        Action onMessageSent() {
            m_websocket->getConnection().invalidate();
            return finish();
        }
    };

    auto message =
        MessageDto::createShared(MessageCodes::OUTGOING_CLIENT_KICKED,
                                 oatpp::String("you were kicked."));

    std::lock_guard<std::mutex> socketLock(m_socketMutex);
    if (m_socket) {
        m_asyncExecutor->execute<KickCoroutine>(
            &m_writeLock, m_socket, m_objectMapper->writeToString(message));
    }
}

std::shared_ptr<Session> Connection::getHubSession() { return m_hubSession; }

v_int64 Connection::getConnectionId() { return m_connectionId; }

void Connection::invalidateSocket() {
    {
        std::lock_guard<std::mutex> socketLock(m_socketMutex);
        if (m_socket) {
            m_socket->getConnection().invalidate();
            m_socket.reset();
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_messageQueue->mutex);
        m_messageQueue->queue.clear();
    }
}

void Connection::checkPingsRules(const v_int64 currentPingSessionTimestamp) {
    std::lock_guard<std::mutex> pingLock(m_pingMutex);

    if (m_lastPingTimestamp != currentPingSessionTimestamp) {
        m_failedPings++;
    }

    OATPP_LOGD("Connection", "failed pings=%d", m_failedPings)

    if (m_failedPings >= m_hubSession->getConfig()->maxFailedPings) {
        OATPP_LOGD("Connection",
                   "maxFailedPings exceeded. ConnectionId=%lld. Connection dropped.",
                   m_connectionId);
        invalidateSocket();
    }
}

oatpp::async::CoroutineStarter Connection::handlePong(
    const oatpp::Object<MessageDto>& message) {
    auto timestamp = message->payload.retrieve<oatpp::Int64>();

    if (!timestamp) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE, "Message MUST contain 'payload.'"));
    }

    v_int64 pt = m_hubSession->reportConnectionPong(m_connectionId, timestamp);

    {
        std::lock_guard<std::mutex> pingLock(m_pingMutex);
        m_pingTime = pt;
        if (m_pingTime >= 0) {
            m_failedPings = 0;
            m_lastPingTimestamp = timestamp;
        }
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleBroadcast(
    const oatpp::Object<MessageDto>& message) {
    auto connections = m_hubSession->getAllConnections();

    for (auto connection : connections) {
        if (connection->getConnectionId() != m_connectionId) {
            auto payload = OutgoingMessageDto::createShared();
            payload->connectionId = m_connectionId;
            payload->data = message->payload.retrieve<oatpp::String>();

            connection->queueMessage(MessageDto::createShared(
                MessageCodes::OUTGOING_MESSAGE, payload));
        }
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleDirectMessage(
    const oatpp::Object<MessageDto>& message) {
    auto dm = message->payload.retrieve<oatpp::Object<DirectMessageDto>>();

    if (!dm) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE, "Message MUST contain 'payload.'"));
    }

    if (!dm->connectionIds || dm->connectionIds->empty()) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE,
            "Payload MUST contain array of connectionIds of recipients."));
    }

    auto connections = m_hubSession->getConnections(dm->connectionIds);

    for (auto connection : connections) {
        if (connection->getConnectionId() != m_connectionId) {
            auto payload = OutgoingMessageDto::createShared();
            payload->connectionId = m_connectionId;
            payload->data = dm->data;

            connection->queueMessage(MessageDto::createShared(
                MessageCodes::OUTGOING_MESSAGE, payload));
        }
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleSynchronizedEvent(
    const oatpp::Object<MessageDto>& message) {
    m_hubSession->broadcastSynchronizedEvent(
        m_connectionId, message->payload.retrieve<oatpp::String>());
    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleKickMessage(
    const oatpp::Object<MessageDto>& message) {
    auto host = m_hubSession->getHost();
    if (host == nullptr) {
        return sendErrorAsync(ErrorDto::createShared(ErrorCodes::INVALID_STATE,
                                                     "There is no hub host."));
    }

    if (host->getConnectionId() != m_connectionId) {
        return sendErrorAsync(
            ErrorDto::createShared(ErrorCodes::OPERATION_NOT_PERMITTED,
                                   "Only Host connection can kick others."));
    }

    auto ids = message->payload.retrieve<oatpp::Vector<oatpp::Int64>>();

    if (!ids || ids->empty()) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE,
            "Payload MUST contain array of connectionIds to kick from session.'"));
    }

    auto connections = m_hubSession->getConnections(ids);

    for (auto connection : connections) {
        if (connection->getConnectionId() != m_connectionId) {
            connection->kick();
        }
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleClientMessage(
    const oatpp::Object<MessageDto>& message) {
    auto host = m_hubSession->getHost();
    if (host == nullptr) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::INVALID_STATE,
            "There is no hub host. No one will receive this message."));
    }

    if (host->getConnectionId() == m_connectionId) {
        return sendErrorAsync(
            ErrorDto::createShared(ErrorCodes::OPERATION_NOT_PERMITTED,
                                   "Host can't send message to itself."));
    }

    auto payload = OutgoingMessageDto::createShared();
    payload->connectionId = m_connectionId;
    payload->data = message->payload.retrieve<oatpp::String>();

    host->queueMessage(
        MessageDto::createShared(MessageCodes::OUTGOING_MESSAGE, payload));

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::handleMessage(
    const oatpp::Object<MessageDto>& message) {
    if (!message->code) {
        return sendErrorAsync(ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE, "Message MUST contain 'code' field."));
    }

    switch (*message->code) {
        case MessageCodes::INCOMING_PONG:
            return handlePong(message);
        case MessageCodes::INCOMING_BROADCAST:
            return handleBroadcast(message);
        case MessageCodes::INCOMING_DIRECT_MESSAGE:
            return handleDirectMessage(message);
        case MessageCodes::INCOMING_SYNCHRONIZED_EVENT:
            return handleSynchronizedEvent(message);
        case MessageCodes::INCOMING_HOST_KICK_CLIENTS:
            return handleKickMessage(message);
        case MessageCodes::INCOMING_CLIENT_MESSAGE:
            return handleClientMessage(message);

        default:
            return sendErrorAsync(
                ErrorDto::createShared(ErrorCodes::OPERATION_NOT_PERMITTED,
                                       "Invalid operation code."));
    }

    return nullptr;
}

oatpp::async::CoroutineStarter Connection::onPing(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const oatpp::String& message) {
    return oatpp::async::synchronize(&m_writeLock,
                                     socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter Connection::onPong(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const oatpp::String& message) {
    return nullptr;  // do nothing
}

oatpp::async::CoroutineStarter Connection::onClose(
    const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code,
    const oatpp::String& message) {
    OATPP_LOGD("Connection", "onClose received.")
    return nullptr;  // do nothing
}

oatpp::async::CoroutineStarter Connection::readMessage(
    const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data,
    oatpp::v_io_size size) {
    if (m_messageBuffer.getCurrentPosition() + size >
        m_hubSession->getConfig()->maxMessageSizeBytes) {
        auto err = ErrorDto::createShared(
            ErrorCodes::BAD_MESSAGE,
            "Fatal Error. Serialized message size shouldn't exceed " +
                oatpp::utils::conversion::int64ToStdStr(
                    m_hubSession->getConfig()->maxMessageSizeBytes) +
                " bytes.");
        return sendErrorAsync(err, true);
    }

    if (size == 0) {  // message transfer finished

        auto wholeMessage = m_messageBuffer.toString();
        m_messageBuffer.setCurrentPosition(0);

        oatpp::Object<MessageDto> message;

        try {
            message = m_objectMapper->readFromString<oatpp::Object<MessageDto>>(
                wholeMessage);
        } catch (const std::runtime_error& e) {
            auto err = ErrorDto::createShared(
                ErrorCodes::BAD_MESSAGE, "Fatal Error. Can't parse message.");
            return sendErrorAsync(err, true);
        }

        return handleMessage(message);

    } else if (size > 0) {  // message frame received
        m_messageBuffer.writeSimple(data, size);
    }

    return nullptr;  // do nothing
}