#include "async_io.hpp"

#include <fstream>

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"

namespace atom::async::io {

AsyncFile::AsyncFile(asio::io_context& io_context)
    : io_context_(io_context), timer_(io_context) {
    LOG_F(INFO, "AsyncFile constructor called");
}

void AsyncFile::asyncRead(
    const std::string& filename,
    const std::function<void(const std::string&)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncRead called with filename: {}", filename);
    io_context_.post([filename, callback]() {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open file for reading: {}", filename);
            callback("");
            return;
        }

        std::string content;
        std::array<char, 1024> buffer;
        while (file.read(buffer.data(), buffer.size())) {
            content.append(buffer.data(), file.gcount());
            std::this_thread::sleep_for(
                std::chrono::milliseconds(kSimulateSlowReadingMs));
            LOG_F(INFO, "Reading progress: {} bytes read.", content.size());
        }
        content.append(buffer.data(), file.gcount());
        file.close();
        LOG_F(INFO, "File read successfully: {}", filename);
        callback(content);
    });
}

void AsyncFile::asyncWrite(const std::string& filename,
                           const std::string& content,
                           const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncWrite called with filename: {}", filename);
    io_context_.post([filename, content, callback]() {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open file for writing: {}", filename);
            callback(false);
            return;
        }

        file.write(content.data(),
                   static_cast<std::streamsize>(content.size()));
        file.close();
        LOG_F(INFO, "File written successfully: {}", filename);
        callback(true);
    });
}

void AsyncFile::asyncDelete(const std::string& filename,
                            const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncDelete called with filename: {}", filename);
    io_context_.post([filename, callback]() {
        if (atom::io::removeFile(filename)) {
            LOG_F(INFO, "File deleted: {}", filename);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to delete file: {}", filename);
            callback(false);
        }
    });
}

void AsyncFile::asyncCopy(const std::string& src, const std::string& dest,
                          const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncCopy called with src: {}, dest: {}", src,
          dest);
    io_context_.post([src, dest, callback]() {
        if (atom::io::copyFile(src, dest)) {
            LOG_F(INFO, "File copied from {} to {}", src, dest);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to copy file from {} to {}", src, dest);
            callback(false);
        }
    });
}

void AsyncFile::asyncReadWithTimeout(
    const std::string& filename, int timeoutMs,
    const std::function<void(const std::string&)>& callback) {
    LOG_F(INFO,
          "AsyncFile::asyncReadWithTimeout called with filename: {}, "
          "timeoutMs: {}",
          filename, timeoutMs);
    bool completed = false;
    asyncRead(filename, [&completed, callback](const std::string& content) {
        if (!completed) {
            completed = true;
            callback(content);
        }
    });

    timer_.expires_after(std::chrono::milliseconds(timeoutMs));
    timer_.async_wait(
        [&completed, filename, callback](const std::error_code& errorCode) {
            if (!completed && !errorCode) {
                completed = true;
                LOG_F(WARNING, "Operation timed out: {}", filename);
                callback("");  // Timeout with empty result
            }
        });
}

void AsyncFile::asyncBatchRead(
    const std::vector<std::string>& files,
    const std::function<void(const std::vector<std::string>&)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncBatchRead called with {} files", files.size());
    auto results = std::make_shared<std::vector<std::string>>(files.size());
    auto remaining = std::make_shared<int>(files.size());

    for (size_t i = 0; i < files.size(); ++i) {
        asyncRead(files[i], [results, remaining, callback,
                             i](const std::string& content) {
            (*results)[i] = content;
            if (--(*remaining) == 0) {
                LOG_F(INFO, "All files read successfully");
                callback(*results);  // All reads are complete
            }
        });
    }
}

void AsyncFile::asyncStat(
    const std::string& filename,
    const std::function<void(bool, std::uintmax_t, std::time_t)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncStat called with filename: {}", filename);
    io_context_.post([filename, callback]() {
        std::error_code errorCode;
        auto fileSize = atom::io::fileSize(filename);
        if (fileSize == 0) {
            LOG_F(ERROR, "Failed to get file size: {}", filename);
            callback(false, 0, 0);
            return;
        }

        auto lastWriteTime =
            std::filesystem::last_write_time(filename, errorCode);
        if (errorCode) {
            LOG_F(ERROR, "Failed to get file last modification time: {}",
                  filename);
            callback(false, 0, 0);
            return;
        }

        LOG_F(INFO, "File stat fetched: {}", filename);
        callback(true, fileSize,
                 std::chrono::system_clock::to_time_t(
                     std::chrono::file_clock::to_sys(lastWriteTime)));
    });
}

void AsyncFile::asyncMove(const std::string& src, const std::string& dest,
                          const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncMove called with src: {}, dest: {}", src,
          dest);
    io_context_.post([src, dest, callback]() {
        if (atom::io::moveFile(src, dest)) {
            LOG_F(INFO, "File moved from {} to {}", src, dest);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to move file from {} to {}", src, dest);
            callback(false);
        }
    });
}

void AsyncFile::asyncChangePermissions(
    const std::string& filename, std::filesystem::perms perms,
    const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncChangePermissions called with filename: {}",
          filename);
    io_context_.post([filename, perms, callback]() {
        std::error_code errorCode;
        std::filesystem::permissions(filename, perms, errorCode);
        if (!errorCode) {
            LOG_F(INFO, "Permissions changed for file: {}", filename);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to change file permissions: {}",
                  errorCode.message());
            callback(false);
        }
    });
}

void AsyncFile::asyncCreateDirectory(
    const std::string& path, const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncCreateDirectory called with path: {}", path);
    io_context_.post([path, callback]() {
        if (atom::io::createDirectory(path)) {
            LOG_F(INFO, "Directory created: {}", path);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to create directory: {}", path);
            callback(false);
        }
    });
}

void AsyncFile::asyncExists(const std::string& filename,
                            const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncFile::asyncExists called with filename: {}", filename);
    io_context_.post([filename, callback]() {
        bool exists = atom::io::isFileExists(filename);
        LOG_F(INFO, "File existence check: {} - {}", filename, exists);
        callback(exists);
    });
}

AsyncDirectory::AsyncDirectory(asio::io_context& io_context)
    : io_context_(io_context) {
    LOG_F(INFO, "AsyncDirectory constructor called");
}

void AsyncDirectory::asyncCreate(const std::string& path,
                                 const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncDirectory::asyncCreate called with path: {}", path);
    io_context_.post([path, callback]() {
        if (atom::io::createDirectory(path)) {
            LOG_F(INFO, "Directory created: {}", path);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to create directory: {}", path);
            callback(false);
        }
    });
}

void AsyncDirectory::asyncRemove(const std::string& path,
                                 const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncDirectory::asyncRemove called with path: {}", path);
    io_context_.post([path, callback]() {
        if (atom::io::removeDirectory(path)) {
            LOG_F(INFO, "Directory removed: {}", path);
            callback(true);
        } else {
            LOG_F(ERROR, "Failed to remove directory: {}", path);
            callback(false);
        }
    });
}

void AsyncDirectory::asyncListContents(
    const std::string& path,
    const std::function<void(std::vector<std::string>)>& callback) {
    LOG_F(INFO, "AsyncDirectory::asyncListContents called with path: {}", path);
    io_context_.post([path, callback]() {
        std::vector<std::string> contents;
        std::error_code errorCode;

        for (const auto& entry :
             std::filesystem::directory_iterator(path, errorCode)) {
            if (errorCode) {
                LOG_F(ERROR, "Failed to list contents of directory: {} - {}",
                      path, errorCode.message());
                callback({});
                return;
            }
            contents.push_back(entry.path().string());
        }

        LOG_F(INFO, "Listed contents of directory: {}", path);
        callback(contents);
    });
}

void AsyncDirectory::asyncExists(const std::string& path,
                                 const std::function<void(bool)>& callback) {
    LOG_F(INFO, "AsyncDirectory::asyncExists called with path: {}", path);
    io_context_.post([path, callback]() {
        bool exists = atom::io::isFolderExists(path);
        LOG_F(INFO, "Directory existence check: {} - {}", path, exists);
        callback(exists);
    });
}

}  // namespace atom::async::io
