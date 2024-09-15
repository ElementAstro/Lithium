#include "asyncio.hpp"

#include <fstream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::io {
FileWriter::FileWriter(handle_type h) : coro_handle(h) {}
FileWriter::~FileWriter() {
    if (coro_handle) {
        coro_handle.destroy();
    }
}

auto FileWriter::promise_type::get_return_object() {
    return FileWriter{handle_type::from_promise(*this)};
}
auto FileWriter::promise_type::initial_suspend() noexcept {
    return std::suspend_never{};
}
auto FileWriter::promise_type::final_suspend() noexcept {
    return std::suspend_never{};
}
void FileWriter::promise_type::return_void() {}
void FileWriter::promise_type::unhandled_exception() { std::terminate(); }

bool FileWriter::await_ready() const noexcept { return false; }
void FileWriter::await_suspend(std::coroutine_handle<> h) {
    coro_handle = handle_type::from_address(h.address());
}
void FileWriter::await_resume() const {}

FileWriter async_write(const std::string& filename, const std::string& data) {
#ifdef USE_ASIO
    asio::io_context io_context;
    asio::posix::stream_descriptor file(
        io_context, ::open(filename.c_str(), O_WRONLY | O_CREAT, 0644));
    if (!file.is_open()) {
        THROW_FAIL_TO_OPEN_FILE(errno, std::generic_category(),
                                "Failed to open file");
    }

    asio::async_write(
        file, asio::buffer(data), [](std::error_code ec, std::size_t) {
            if (ec) {
                LOG_F(ERROR, "async_write failed with error: {}", ec.message());
            }
        });

    io_context.run();
#else
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open file");
    }

#ifdef _WIN32
    OVERLAPPED overlapped = {0};
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL,
                              CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        THROW_FAIL_TO_CREATE_FILE(GetLastError(), std::system_category(),
                                  "Failed to create file");
    }

    WriteFileEx(hFile, data.c_str(), data.size(), &overlapped,
                [](DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransfered*/,
                   LPOVERLAPPED /*lpOverlapped*/) {
                    if (dwErrorCode != 0) {
                        LOG_F(ERROR, "WriteFileEx failed with error: {}",
                              dwErrorCode);
                    }
                });

    SleepEx(INFINITE, TRUE);
    CloseHandle(hFile);
#else
    aiocb cb = {0};
    cb.aio_fildes = open(filename.c_str(), O_WRONLY | O_CREAT, 0644);
    if (cb.aio_fildes == -1) {
        THROW_FAIL_TO_OPEN_FILE(errno, std::generic_category(),
                                "Failed to open file");
    }

    cb.aio_buf = data.c_str();
    cb.aio_nbytes = data.size();
    cb.aio_offset = 0;

    if (aio_write(&cb) == -1) {
        THROW_FAIL_TO_WRITE_FILE(errno, std::generic_category(),
                                 "Failed to initiate aio_write");
    }

    while (aio_error(&cb) == EINPROGRESS) {
        // Wait for the write to complete
    }

    if (aio_return(&cb) == -1) {
        THROW_FAIL_TO_WRITE_FILE(errno, std::generic_category(),
                                 "aio_write failed");
    }

    close(cb.aio_fildes);
#endif
#endif

    co_return;
}

FileWriter async_read(const std::string& filename, std::string& data,
                      std::size_t size) {
#ifdef USE_ASIO
    asio::io_context io_context;
    asio::posix::stream_descriptor file(io_context,
                                        ::open(filename.c_str(), O_RDONLY));
    if (!file.is_open()) {
        THROW_FAIL_TO_OPEN_FILE(errno, std::generic_category(),
                                "Failed to open file");
    }

    data.resize(size);
    asio::async_read(
        file, asio::buffer(data), [](std::error_code ec, std::size_t) {
            if (ec) {
                LOG_F(ERROR, "async_read failed with error: {}", ec.message());
            }
        });

    io_context.run();
#else
#ifdef _WIN32
    OVERLAPPED overlapped = {0};
    HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL,
                              OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        THROW_FAIL_TO_OPEN_FILE(GetLastError(), std::system_category(),
                                "Failed to open file");
    }

    data.resize(size);
    ReadFileEx(hFile, &data[0], size, &overlapped,
               [](DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransfered*/,
                  LPOVERLAPPED /*lpOverlapped*/) {
                   if (dwErrorCode != 0) {
                       LOG_F(ERROR, "ReadFileEx failed with error: {}",
                             dwErrorCode);
                   }
               });

    SleepEx(INFINITE, TRUE);
    CloseHandle(hFile);
#else
    aiocb cb = {0};
    cb.aio_fildes = open(filename.c_str(), O_RDONLY);
    if (cb.aio_fildes == -1) {
        THROW_FAIL_TO_OPEN_FILE(errno, std::generic_category(),
                                "Failed to open file");
    }

    data.resize(size);
    cb.aio_buf = &data[0];
    cb.aio_nbytes = size;
    cb.aio_offset = 0;

    if (aio_read(&cb) == -1) {
        THROW_FAIL_TO_WRITE_FILE(errno, std::generic_category(),
                                 "Failed to initiate aio_read");
    }

    while (aio_error(&cb) == EINPROGRESS) {
        // Wait for the read to complete
    }

    if (aio_return(&cb) == -1) {
        THROW_FAIL_TO_WRITE_FILE(errno, std::generic_category(),
                                 "aio_read failed");
    }

    close(cb.aio_fildes);
#endif
#endif

    co_return;
}

FileWriter async_delete(const std::string& filename) {
#ifdef USE_ASIO
    asio::io_context io_context;
    io_context.post([filename]() {
        if (std::remove(filename.c_str()) != 0) {
            LOG_F(ERROR, "Failed to delete file: {}", filename);
        }
    });
    io_context.run();
#else
    if (std::remove(filename.c_str()) != 0) {
        THROW_FAIL_TO_DELETE_FILE(errno, std::generic_category(),
                                  "Failed to delete file");
    }
#endif

    co_return;
}

FileWriter async_copy(const std::string& src_filename,
                      const std::string& dest_filename) {
#ifdef USE_ASIO
    asio::io_context io_context;
    io_context.post([src_filename, dest_filename]() {
        std::ifstream src(src_filename, std::ios::binary);
        std::ofstream dest(dest_filename, std::ios::binary);
        if (!src.is_open() || !dest.is_open()) {
            LOG_F(ERROR, "Failed to open source or destination file");
            return;
        }
        dest << src.rdbuf();
    });
    io_context.run();
#else
    std::ifstream src(src_filename, std::ios::binary);
    std::ofstream dest(dest_filename, std::ios::binary);
    if (!src.is_open() || !dest.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open source or destination file");
    }
    dest << src.rdbuf();
#endif

    co_return;
}
}  // namespace atom::io
