#ifndef ASYNC_SERVER_ROOMS_PEER_HPP
#define ASYNC_SERVER_ROOMS_PEER_HPP

#include "dto/Config.hpp"
#include "dto/DTOs.hpp"
#include "rooms/File.hpp"
#include "utils/Statistics.hpp"

#include "oatpp-websocket/AsyncWebSocket.hpp"

#include "oatpp/async/Executor.hpp"
#include "oatpp/async/Lock.hpp"
#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/macro/component.hpp"

class Room;  // FWD

class Peer : public oatpp::websocket::AsyncWebSocket::Listener {
    /**
     * Buffer for messages. Needed for multi-frame messages.
     */
    oatpp::data::stream::BufferOutputStream m_messageBuffer_;

    /**
     * Lock for synchronization of writes to the web socket.
     */
    oatpp::async::Lock m_writeLock_;

    std::shared_ptr<AsyncWebSocket> m_socket_;
    std::shared_ptr<Room> m_room_;
    oatpp::String m_nickname_;
    v_int64 m_peerId_;

    std::atomic<v_int32> m_pingPoingCounter_;
    std::list<std::shared_ptr<File>> m_files_;

    /* Inject application components */

    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                    m_objectMapper);
    OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_appConfig);
    OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);

    auto onApiError(const oatpp::String& errorMessage)
        -> oatpp::async::CoroutineStarter;

    auto validateFilesList(const MessageDto::FilesList& filesList)
        -> oatpp::async::CoroutineStarter;
    auto handleFilesMessage(const oatpp::Object<MessageDto>& message)
        -> oatpp::async::CoroutineStarter;
    auto handleFileChunkMessage(const oatpp::Object<MessageDto>& message)
        -> oatpp::async::CoroutineStarter;
    auto handleQTextMessage(const std::string& message)
        -> oatpp::async::CoroutineStarter;
    auto handleTextMessage(const oatpp::Object<MessageDto>& message)
        -> oatpp::async::CoroutineStarter;
    auto handleMessage(const oatpp::Object<MessageDto>& message)
        -> oatpp::async::CoroutineStarter;

public:
    Peer(const std::shared_ptr<AsyncWebSocket>& socket,
         const std::shared_ptr<Room>& room, const oatpp::String& nickname,
         v_int64 peerId)
        : m_socket_(socket),
          m_room_(room),
          m_nickname_(nickname),
          m_peerId_(peerId),
          m_pingPoingCounter_(0) {}

    /**
     * Send message to peer (to user).
     * @param message
     */
    void sendMessageAsync(const oatpp::Object<MessageDto>& message);

    /**
     * Send Websocket-Ping.
     * @return - `true` - ping was sent.
     * `false` peer has not responded to the last ping, it means we have to
     * disconnect him.
     */
    auto sendPingAsync() -> bool;

    /**
     * Get room of the peer.
     * @return
     */
    auto getRoom() -> std::shared_ptr<Room>;

    /**
     * Get peer nickname.
     * @return
     */
    auto getNickname() -> oatpp::String;

    /**
     * Get peer peerId.
     * @return
     */
    auto getPeerId() -> v_int64;

    /**
     * Add file shared by user. (for indexing purposes)
     */
    void addFile(const std::shared_ptr<File>& file);

    /**
     * List of shared by user files.
     * @return
     */
    auto getFiles() -> const std::list<std::shared_ptr<File>>&;

    /**
     * Remove circle `std::shared_ptr` dependencies
     */
    void invalidateSocket();

    auto onPing(const std::shared_ptr<AsyncWebSocket>& socket,
                const oatpp::String& message) -> CoroutineStarter override;
    auto onPong(const std::shared_ptr<AsyncWebSocket>& socket,
                const oatpp::String& message) -> CoroutineStarter override;
    auto onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code,
                 const oatpp::String& message) -> CoroutineStarter override;
    auto readMessage(const std::shared_ptr<AsyncWebSocket>& socket,
                     v_uint8 opcode, p_char8 data,
                     oatpp::v_io_size size) -> CoroutineStarter override;
};

#endif  // ASYNC_SERVER_ROOMS_PEER_HPP
