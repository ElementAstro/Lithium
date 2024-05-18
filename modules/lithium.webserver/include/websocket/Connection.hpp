/*
 * Connection.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Connection

**************************************************/

#ifndef LITHIUM_WEBSOCKET_CONNECTION_HPP
#define LITHIUM_WEBSOCKET_CONNECTION_HPP

#include "Constants.hpp"

#include "config/Config.hpp"
#include "config/HubsConfig.hpp"

#include "data/DTOs.hpp"

#include "oatpp-websocket/AsyncWebSocket.hpp"

#include "oatpp/network/ConnectionProvider.hpp"

#include "oatpp/core/async/Executor.hpp"
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/data/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"

class Session;  // FWD

class Connection : public oatpp::websocket::AsyncWebSocket::Listener {
private:
    struct MessageQueue {
        std::list<oatpp::Object<MessageDto>> queue;
        std::mutex mutex;
        bool active = false;
    };

private:
    /**
     * Buffer for messages. Needed for multi-frame messages.
     */
    oatpp::data::stream::BufferOutputStream m_messageBuffer;

    /**
     * Lock for synchronization of writes to the web socket.
     */
    oatpp::async::Lock m_writeLock;

private:
    std::shared_ptr<AsyncWebSocket> m_socket;
    std::mutex m_socketMutex;
    std::shared_ptr<Session> m_hubSession;
    v_int64 m_connectionId;
    std::shared_ptr<MessageQueue> m_messageQueue;

private:
    v_int64 m_pingTime;
    v_int64 m_failedPings;
    v_int64 m_lastPingTimestamp;
    std::mutex m_pingMutex;

private:
    /* Inject application components */
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                    m_objectMapper, Constants::COMPONENT_REST_API);

private:
    CoroutineStarter handlePong(const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleBroadcast(const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleDirectMessage(
        const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleSynchronizedEvent(
        const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleKickMessage(
        const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleClientMessage(
        const oatpp::Object<MessageDto>& message);
    CoroutineStarter handleMessage(const oatpp::Object<MessageDto>& message);

public:
    Connection(const std::shared_ptr<AsyncWebSocket>& socket,
         const std::shared_ptr<Session>& hubSession, v_int64 connectionId);

    /**
     * Send message to connection.
     * @param message
     */
    oatpp::async::CoroutineStarter sendMessageAsync(
        const oatpp::Object<MessageDto>& message);

    /**
     * Send error message to connection.
     * @param error
     * @param fatal
     */
    oatpp::async::CoroutineStarter sendErrorAsync(
        const oatpp::Object<ErrorDto>& error, bool fatal = false);

    /**
     * Queue message to send to connection.
     * @param message
     * @return
     */
    bool queueMessage(const oatpp::Object<MessageDto>& message);

    /**
     * Ping connection.
     */
    void ping(const v_int64 timestampMicroseconds);

    /**
     * Kick this connection.
     */
    void kick();

    /**
     * Check ping rules.
     * @param currentPingSessionTimestamp
     */
    void checkPingsRules(const v_int64 currentPingSessionTimestamp);

    /**
     * Get the hub session the connection associated with.
     * @return
     */
    std::shared_ptr<Session> getHubSession();

    /**
     * Get connection connectionId.
     * @return
     */
    v_int64 getConnectionId();

    /**
     * Remove circle `std::shared_ptr` dependencies
     */
    void invalidateSocket();

public:  // WebSocket Listener methods
    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket,
                            const oatpp::String& message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket,
                            const oatpp::String& message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket,
                             v_uint16 code,
                             const oatpp::String& message) override;
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket,
                                 v_uint8 opcode, p_char8 data,
                                 oatpp::v_io_size size) override;
};

#endif  // LITHIUM_WEBSOCKET_CONNECTION_HPP
