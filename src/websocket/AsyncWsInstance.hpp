/*
 * AsyncWsInstance.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Instance

**************************************************/

#ifndef ASYNC_WS_INSTANCE_HPP
#define ASYNC_WS_INSTANCE_HPP

#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/core/async/Executor.hpp"
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/macro/component.hpp"

#include <memory>

class AsyncWsHub;  // FWD

class AsyncWsInstance : public oatpp::websocket::AsyncWebSocket::Listener {
public:
    AsyncWsInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                    const std::shared_ptr<AsyncWsHub> &hub,
                    const oatpp::String &connection_name, v_int32 userId);

    // ----------------------------------------------------------------------
    // The AsyncWsInstance methods
    // ---------------------------------------------------------------------

    /**
     * Send message to AsyncWsInstance (to user).
     * @param message
     */
    void sendMessage(const oatpp::String &message);

    void sendBinaryMessage(const void *binary_message, int size);

    /**
     * Get hub of the AsyncWsInstance.
     * @return
     */
    std::shared_ptr<AsyncWsHub> getHub();

    /**
     * Get AsyncWsInstance connection_name.
     * @return
     */
    oatpp::String getName();

    /**
     * Get AsyncWsInstance userId.
     * @return
     */
    v_int32 getId();

public:  // WebSocket Listener methods
    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket,
                            const oatpp::String &message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket,
                            const oatpp::String &message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket,
                             v_uint16 code,
                             const oatpp::String &message) override;
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket,
                                 v_uint8 opcode, p_char8 data,
                                 oatpp::v_io_size size) override;

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
    /**
     * Inject async executor object.
     */
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<AsyncWsHub> m_hub;
    oatpp::String m_connection_name;
    v_int32 m_userId;
};

#endif  // WsINSTANCE_HPP
