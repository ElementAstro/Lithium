#include "File.hpp"

#include "dto/DTOs.hpp"
#include "rooms/Peer.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

void File::Subscriber::WaitListListener::onNewItem(
    oatpp::async::CoroutineWaitList& list) {
    std::lock_guard lock(m_subscriber->m_chunkLock);
    if (m_subscriber->m_chunk || !m_subscriber->m_valid) {
        list.notifyAll();
    }
}

File::Subscriber::Subscriber(v_int64 id, const std::shared_ptr<File>& file)
    : m_id(id),
      m_file(file),
      m_valid(true),
      m_progress(0),
      m_waitListListener(this) {
    m_waitList.setListener(&m_waitListListener);
    LOG_F(INFO, "Subscriber created with ID: {}", m_id);
}

File::Subscriber::~Subscriber() {
    LOG_F(INFO, "Subscriber with ID: {} is being destroyed.", m_id);
    m_file->unsubscribe(m_id);
}

void File::Subscriber::provideFileChunk(const oatpp::String& data) {
    std::lock_guard<std::mutex> lock(m_chunkLock);
    if (m_chunk != nullptr) {
        LOG_F(ERROR, "File chunk collision for Subscriber ID: {}", m_id);
        THROW_RUNTIME_ERROR("File chunk collision.");
    }
    m_chunk = data;
    LOG_F(INFO, "Provided file chunk to Subscriber ID: {}", m_id);
    m_waitList.notifyAll();
}

void File::Subscriber::requestChunk(v_int64 size) {
    if (m_valid) {
        auto message = MessageDto::createShared();
        message->code = MessageCodes::CODE_FILE_REQUEST_CHUNK;

        message->files = MessageDto::FilesList::createShared();

        auto file = FileDto::createShared();
        file->clientFileId = m_file->m_clientFileId;
        file->serverFileId = m_file->m_serverFileId;
        file->subscriberId = m_id;

        file->chunkPosition = m_progress;
        file->chunkSize = size;

        message->files->push_back(file);

        LOG_F(INFO, "Requesting chunk of size {} for Subscriber ID: {}", size,
              m_id);
        m_file->m_host->sendMessageAsync(message);
    }
}

oatpp::async::CoroutineStarter File::Subscriber::waitForChunkAsync() {
    class WaitCoroutine : public oatpp::async::Coroutine<WaitCoroutine> {
    private:
        Subscriber* m_subscriber;

    public:
        WaitCoroutine(Subscriber* subscriber) : m_subscriber(subscriber) {}

        Action act() override {
            std::lock_guard<std::mutex> lock(m_subscriber->m_chunkLock);
            if (m_subscriber->m_chunk || !m_subscriber->m_valid) {
                LOG_F(INFO,
                      "Chunk available or subscriber invalid for Subscriber "
                      "ID: {}",
                      m_subscriber->getId());
                return finish();
            }
            LOG_F(INFO, "Waiting for chunk for Subscriber ID: {}",
                  m_subscriber->getId());
            return Action::createWaitListAction(&m_subscriber->m_waitList);
        }
    };

    return WaitCoroutine::start(this);
}

oatpp::v_io_size File::Subscriber::readChunk(void* buffer, v_buff_size count,
                                             oatpp::async::Action& action) {
    std::lock_guard<std::mutex> lock(m_chunkLock);

    if (!m_valid) {
        LOG_F(ERROR, "Attempt to read from invalid Subscriber ID: {}", m_id);
        THROW_RUNTIME_ERROR("File is not valid any more.");
    }

    if (m_progress < m_file->getFileSize()) {
        if (m_chunk) {
            v_int64 chunkSize = m_chunk->size();
            if (chunkSize > count) {
                LOG_F(ERROR, "Invalid chunk size for Subscriber ID: {}", m_id);
                THROW_RUNTIME_ERROR("Invalid chunk size");
            }
            std::memcpy(buffer, m_chunk->data(), chunkSize);
            m_progress += chunkSize;
            LOG_F(INFO, "Read chunk of size {} for Subscriber ID: {}",
                  chunkSize, m_id);
            m_chunk = nullptr;
            return chunkSize;
        }

        requestChunk(count);
        action =
            waitForChunkAsync().next(oatpp::async::Action::createActionByType(
                oatpp::async::Action::TYPE_REPEAT));
        LOG_F(INFO, "Retrying read for Subscriber ID: {}", m_id);
        return oatpp::IOError::RETRY_READ;
    }

    LOG_F(INFO, "Completed reading for Subscriber ID: {}", m_id);
    return 0;
}

v_int64 File::Subscriber::getId() { return m_id; }

void File::Subscriber::invalidate() {
    std::lock_guard<std::mutex> lock(m_chunkLock);
    m_valid = false;
    LOG_F(INFO, "Subscriber ID: {} invalidated.", m_id);
    m_waitList.notifyAll();
}

bool File::Subscriber::isValid() {
    std::lock_guard<std::mutex> lock(m_chunkLock);
    return m_valid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File

File::File(const std::shared_ptr<Peer>& host, v_int64 clientFileId,
           v_int64 serverFileId, const oatpp::String& fileName,
           v_int64 fileSize)
    : m_host(host),
      m_clientFileId(clientFileId),
      m_serverFileId(serverFileId),
      m_fileName(fileName),
      m_fileSize(fileSize),
      m_subscriberIdCounter(1) {
    LOG_F(INFO, "File created: {} (ID: {})", m_fileName->c_str(),
          m_serverFileId);
}

void File::unsubscribe(v_int64 id) {
    std::lock_guard<std::mutex> lock(m_subscribersLock);
    auto it = m_subscribers.find(id);
    if (it != m_subscribers.end()) {
        LOG_F(INFO, "Unsubscribing Subscriber ID: {}", id);
        m_subscribers.erase(it);
    } else {
        LOG_F(WARNING,
              "Attempted to unsubscribe non-existing Subscriber ID: {}", id);
    }
}

std::shared_ptr<File::Subscriber> File::subscribe() {
    std::lock_guard<std::mutex> lock(m_subscribersLock);
    auto s = std::make_shared<Subscriber>(m_subscriberIdCounter++,
                                          shared_from_this());
    m_subscribers[s->getId()] = s.get();
    LOG_F(INFO, "Subscriber ID: {} subscribed to File ID: {}", s->getId(),
          m_serverFileId);
    return s;
}

void File::provideFileChunk(v_int64 subscriberId, const oatpp::String& data) {
    std::lock_guard<std::mutex> lock(m_subscribersLock);
    auto it = m_subscribers.find(subscriberId);

    if (it != m_subscribers.end()) {
        LOG_F(INFO, "Providing file chunk to Subscriber ID: {}", subscriberId);
        it->second->provideFileChunk(data);
    } else {
        LOG_F(WARNING, "Subscriber ID: {} not found.", subscriberId);
    }
}

std::shared_ptr<Peer> File::getHost() { return m_host; }

v_int64 File::getClientFileId() { return m_clientFileId; }

v_int64 File::getServerFileId() { return m_serverFileId; }

oatpp::String File::getFileName() { return m_fileName; }

v_int64 File::getFileSize() { return m_fileSize; }

void File::clearSubscribers() {
    std::lock_guard<std::mutex> lock(m_subscribersLock);
    LOG_F(INFO, "Clearing all subscribers for File ID: {}", m_serverFileId);
    for (auto& subscriber : m_subscribers) {
        subscriber.second->invalidate();
    }
    m_subscribers.clear();
}

size_t File::getSubscriberCount() {
    std::lock_guard<std::mutex> lock(m_subscribersLock);
    return m_subscribers.size();
}