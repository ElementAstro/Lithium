
#ifndef WSINSTANCE_HPP
#define WSINSTANCE_HPP

#include "config.h"

#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncWebSocket.hpp"
#else
#include "oatpp-websocket/WebSocket.hpp"
#endif

#if ENABLE_ASYNC
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/async/Executor.hpp"
#endif

#include "oatpp/core/macro/component.hpp"

#include "atom/server/commander.hpp"
#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"

#include "LithiumApp.hpp"

#include <memory>

class WsHub; // FWD

class WsInstance : public oatpp::websocket::AsyncWebSocket::Listener
{
public:
    WsInstance(const std::shared_ptr<AsyncWebSocket> &socket,
                     const std::shared_ptr<WsHub> &hub,
                     const oatpp::String &connection_name,
                     v_int32 userId);

    // ----------------------------------------------------------------------
    // The WsInstance methods
    // ---------------------------------------------------------------------

    /**
     * Send message to WsInstance (to user).
     * @param message
     */
    void sendMessage(const oatpp::String &message);

    void sendBinaryMessage(const void *binary_message, int size);

    /**
     * Get hub of the WsInstance.
     * @return
     */
    std::shared_ptr<WsHub> getHub();

    /**
     * Get WsInstance connection_name.
     * @return
     */
    oatpp::String getName();

    /**
     * Get WsInstance userId.
     * @return
     */
    v_int32 getId();

public: // WebSocket Listener methods
    CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
    CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
    CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;
    CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

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
    std::shared_ptr<WsHub> m_hub;
    oatpp::String m_connection_name;
    v_int32 m_userId;

    std::shared_ptr<CommandDispatcher<void, json>> m_CommandDispatcher;

	std::shared_ptr<SerializationEngine> m_SerializationEngine;

	std::shared_ptr<DeserializationEngine> m_DeserializationEngine;
};

#endif // WsINSTANCE_HPP
