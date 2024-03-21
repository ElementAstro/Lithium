/*
 * Session.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Connection Session

**************************************************/

#include "Session.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

Session::Session(const oatpp::String& id,
                 const oatpp::Object<HubConfigDto>& config)
    : m_id(id),
      m_config(config),
      m_connectionIdCounter(0),
      m_synchronizedEventId(0),
      m_pingCurrentTimestamp(-1),
      m_pingBestTime(-1),
      m_pingBestConnectionId(-1),
      m_pingBestConnectionSinceTimestamp(-1) {}

oatpp::String Session::getId() { return m_id; }

oatpp::Object<HubConfigDto> Session::getConfig() { return m_config; }

void Session::addConnection(const std::shared_ptr<Connection>& connection, bool isHost) {
    {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        m_connections.insert({connection->getConnectionId(), connection});
        if (isHost) {
            m_host = connection;
        } else {
            if (m_host) {
                m_host->queueMessage(MessageDto::createShared(
                    MessageCodes::OUTGOING_HOST_CLIENT_JOINED,
                    oatpp::Int64(connection->getConnectionId())));
            }
        }
    }

    auto hello = HelloMessageDto::createShared();
    hello->connectionId = connection->getConnectionId();
    hello->isHost = isHost;

    connection->queueMessage(
        MessageDto::createShared(MessageCodes::OUTGOING_HELLO, hello));
}

void Session::setHost(const std::shared_ptr<Connection>& connection) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    m_host = connection;
}

std::shared_ptr<Connection> Session::getHost() {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    return m_host;
}

bool Session::isHostConnection(v_int64 connectionId) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    return m_host && m_host->getConnectionId() == connectionId;
}

void Session::removeConnectionById(v_int64 connectionId, bool& isEmpty) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    if (m_host && m_host->getConnectionId() == connectionId) {
        m_host.reset();
    }
    m_connections.erase(connectionId);
    isEmpty = m_connections.empty();
    if (m_host) {
        m_host->queueMessage(MessageDto::createShared(
            MessageCodes::OUTGOING_HOST_CLIENT_LEFT, oatpp::Int64(connectionId)));
    }
}

std::vector<std::shared_ptr<Connection>> Session::getAllConnections() {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    std::vector<std::shared_ptr<Connection>> result;
    for (auto& pair : m_connections) {
        result.emplace_back(pair.second);
    }
    return result;
}

std::vector<std::shared_ptr<Connection>> Session::getConnections(
    const oatpp::Vector<oatpp::Int64>& connectionIds) {
    if (!connectionIds) {
        return {};
    }

    std::vector<std::shared_ptr<Connection>> result;

    std::lock_guard<std::mutex> lock(m_connectionsMutex);

    for (auto& id : *connectionIds) {
        if (id) {
            auto it = m_connections.find(*id);
            if (it != m_connections.end()) {
                result.emplace_back(it->second);
            }
        }
    }

    return result;
}

void Session::broadcastSynchronizedEvent(v_int64 senderId,
                                         const oatpp::String& eventData) {
    std::lock_guard<std::mutex> lock(m_connectionsMutex);

    auto event = OutgoingSynchronizedMessageDto::createShared();
    event->eventId = m_synchronizedEventId++;
    event->connectionId = senderId;
    event->data = eventData;

    auto message = MessageDto::createShared(
        MessageCodes::OUTGOING_SYNCHRONIZED_EVENT, event);
    for (auto& connection : m_connections) {
        connection.second->queueMessage(message);
    }
}

v_int64 Session::generateNewConnectionId() { return m_connectionIdCounter++; }

void Session::checkAllConnectionsPings() {
    v_int64 currentTimestamp;
    {
        std::lock_guard<std::mutex> lock(m_pingMutex);
        currentTimestamp = m_pingCurrentTimestamp;
    }

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    for (auto& connection : m_connections) {
        connection.second->checkPingsRules(currentTimestamp);
    }
}

void Session::pingAllConnections() {
    auto timestamp = oatpp::base::Environment::getMicroTickCount();

    {
        std::lock_guard<std::mutex> lock(m_pingMutex);
        m_pingCurrentTimestamp = timestamp;
    }

    std::lock_guard<std::mutex> lock(m_connectionsMutex);
    for (auto& connection : m_connections) {
        connection.second->ping(timestamp);
    }
}

v_int64 Session::reportConnectionPong(v_int64 connectionId, v_int64 timestamp) {
    std::lock_guard<std::mutex> lock(m_pingMutex);
    if (timestamp != m_pingCurrentTimestamp) {
        return -1;
    }

    v_int64 pingTime =
        oatpp::base::Environment::getMicroTickCount() - timestamp;

    if (m_pingBestTime < 0 || m_pingBestTime > pingTime) {
        m_pingBestTime = pingTime;
        if (m_pingBestConnectionId != connectionId) {
            m_pingBestConnectionId = connectionId;
            m_pingBestConnectionSinceTimestamp = timestamp;
            OATPP_LOGD("Session", "new best connection=%lld, ping=%lld", connectionId,
                       pingTime)
        }
    }

    return pingTime;
}
