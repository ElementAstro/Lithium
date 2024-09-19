#ifndef ATOM_CONNECTION_SHARED_MEMORY_HPP
#define ATOM_CONNECTION_SHARED_MEMORY_HPP

#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <optional>
#include <span>
#include <thread>
#include <type_traits>

#include "atom/async/async.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/noncopyable.hpp"
#include "macro.hpp"

#ifdef _WIN32
#include <windows.h>
#undef min
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace atom::connection {

template <TriviallyCopyable T>
class SharedMemory : public NonCopyable {
public:
    explicit SharedMemory(std::string_view name, bool create = true);
    ~SharedMemory() override;

    void write(const T& data, std::chrono::milliseconds timeout =
                                  std::chrono::milliseconds(0));
    ATOM_NODISCARD auto read(std::chrono::milliseconds timeout =
                                 std::chrono::milliseconds(0)) const -> T;
    void clear();
    ATOM_NODISCARD auto isOccupied() const -> bool;
    ATOM_NODISCARD auto getName() const ATOM_NOEXCEPT -> std::string_view;
    ATOM_NODISCARD auto getSize() const ATOM_NOEXCEPT -> std::size_t;
    ATOM_NODISCARD auto isCreator() const ATOM_NOEXCEPT -> bool;

    template <typename U>
    void writePartial(
        const U& data, std::size_t offset,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    template <typename U>
    ATOM_NODISCARD auto readPartial(
        std::size_t offset, std::chrono::milliseconds timeout =
                                std::chrono::milliseconds(0)) const -> U;
    ATOM_NODISCARD auto tryRead(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
        -> std::optional<T>;
    void writeSpan(
        std::span<const std::byte> data,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    ATOM_NODISCARD auto readSpan(
        std::span<std::byte> data,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
        -> std::size_t;

private:
    std::string name_;
#ifdef _WIN32
    HANDLE handle_;
#else
    int fd_;
#endif
    void* buffer_;
    std::atomic_flag* flag_;
    mutable std::mutex mutex_;
    bool is_creator_;

    template <typename Func>
    auto withLock(Func&& func, std::chrono::milliseconds timeout) const
        -> decltype(std::forward<Func>(func)());
};

template <TriviallyCopyable T>
SharedMemory<T>::SharedMemory(std::string_view name, bool create)
    : name_(name), buffer_(nullptr), flag_(), is_creator_(create) {
#ifdef _WIN32
    handle_ = create
                  ? CreateFileMappingA(
                        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                        sizeof(T) + sizeof(std::atomic_flag), name.data())
                  : OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.data());
    if (handle_ == nullptr) {
        THROW_FAIL_TO_OPEN_FILE("Failed to create/open file mapping.");
    }
    buffer_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0,
                            sizeof(T) + sizeof(std::atomic_flag));
    if (buffer_ == nullptr) {
        CloseHandle(handle_);
        THROW_UNLAWFUL_OPERATION("Failed to map view of file.");
    }
#else
    fd_ = shm_open(name.data(), create ? (O_CREAT | O_RDWR) : O_RDWR,
                   S_IRUSR | S_IWUSR);
    if (fd_ == -1)
        THROW_FAIL_TO_OPEN_FILE("Failed to create/open shared memory.");
    if (create && ftruncate(fd_, sizeof(T) + sizeof(std::atomic_flag)) == -1) {
        close(fd_);
        shm_unlink(name.data());
        THROW_UNLAWFUL_OPERATION("Failed to resize shared memory.");
    }
    buffer_ = mmap(nullptr, sizeof(T) + sizeof(std::atomic_flag),
                   PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    close(fd_);
    if (buffer_ == MAP_FAILED) {
        if (create)
            shm_unlink(name.data());
        THROW_UNLAWFUL_OPERATION("Failed to map shared memory.");
    }
#endif
    flag_ = new (buffer_) std::atomic_flag();
}

template <TriviallyCopyable T>
SharedMemory<T>::~SharedMemory() {
#ifdef _WIN32
    UnmapViewOfFile(buffer_);
    CloseHandle(handle_);
#else
    munmap(buffer_, sizeof(T) + sizeof(std::atomic_flag));
    if (is_creator_)
        shm_unlink(name_.c_str());
#endif
}

template <TriviallyCopyable T>
template <typename Func>
auto SharedMemory<T>::withLock(Func&& func, std::chrono::milliseconds timeout)
    const -> decltype(std::forward<Func>(func)()) {
    std::unique_lock lock(mutex_);
    auto startTime = std::chrono::steady_clock::now();
    while (flag_->test_and_set(std::memory_order_acquire)) {
        if (timeout != std::chrono::milliseconds(0) &&
            std::chrono::steady_clock::now() - startTime >= timeout) {
            THROW_TIMEOUT_EXCEPTION("Failed to acquire mutex within timeout.");
        }
        std::this_thread::yield();
    }
    if constexpr (std::is_void_v<decltype(std::forward<Func>(func)())>) {
        std::forward<Func>(func)();
        flag_->clear(std::memory_order_release);
        return;
    } else {
        auto result = std::forward<Func>(func)();
        flag_->clear(std::memory_order_release);
        return result;
    }
}

template <TriviallyCopyable T>
void SharedMemory<T>::write(const T& data, std::chrono::milliseconds timeout) {
    withLock(
        [&]() {
            std::memcpy(static_cast<char*>(buffer_) + sizeof(std::atomic_flag),
                        &data, sizeof(T));
            DLOG_F(INFO, "Data written to shared memory.");
        },
        timeout);
}

template <TriviallyCopyable T>
auto SharedMemory<T>::read(std::chrono::milliseconds timeout) const -> T {
    return withLock(
        [&]() {
            T data;
            std::memcpy(
                &data,
                static_cast<const char*>(buffer_) + sizeof(std::atomic_flag),
                sizeof(T));
            DLOG_F(INFO, "Data read from shared memory.");
            return data;
        },
        timeout);
}

template <TriviallyCopyable T>
void SharedMemory<T>::clear() {
    withLock(
        [&]() {
            std::memset(static_cast<char*>(buffer_) + sizeof(std::atomic_flag),
                        0, sizeof(T));
            DLOG_F(INFO, "Shared memory cleared.");
        },
        std::chrono::milliseconds(0));
}

template <TriviallyCopyable T>
auto SharedMemory<T>::isOccupied() const -> bool {
    return flag_->test(std::memory_order_acquire);
}

template <TriviallyCopyable T>
auto SharedMemory<T>::getName() const ATOM_NOEXCEPT -> std::string_view {
    return name_;
}

template <TriviallyCopyable T>
auto SharedMemory<T>::getSize() const ATOM_NOEXCEPT -> std::size_t {
    return sizeof(T);
}

template <TriviallyCopyable T>
auto SharedMemory<T>::isCreator() const ATOM_NOEXCEPT -> bool {
    return is_creator_;
}

template <TriviallyCopyable T>
template <typename U>
void SharedMemory<T>::writePartial(const U& data, std::size_t offset,
                                   std::chrono::milliseconds timeout) {
    static_assert(std::is_trivially_copyable_v<U>,
                  "U must be trivially copyable");
    if (offset + sizeof(U) > sizeof(T)) {
        THROW_INVALID_ARGUMENT("Partial write out of bounds");
    }
    withLock(
        [&]() {
            std::memcpy(
                static_cast<char*>(buffer_) + sizeof(std::atomic_flag) + offset,
                &data, sizeof(U));
            DLOG_F(INFO, "Partial data written to shared memory.");
        },
        timeout);
}

template <TriviallyCopyable T>
template <typename U>
auto SharedMemory<T>::readPartial(
    std::size_t offset, std::chrono::milliseconds timeout) const -> U {
    static_assert(std::is_trivially_copyable_v<U>,
                  "U must be trivially copyable");
    if (offset + sizeof(U) > sizeof(T)) {
        THROW_INVALID_ARGUMENT("Partial read out of bounds");
    }
    return withLock(
        [&]() {
            U data;
            std::memcpy(&data,
                        static_cast<const char*>(buffer_) +
                            sizeof(std::atomic_flag) + offset,
                        sizeof(U));
            DLOG_F(INFO, "Partial data read from shared memory.");
            return data;
        },
        timeout);
}

template <TriviallyCopyable T>
auto SharedMemory<T>::tryRead(std::chrono::milliseconds timeout) const
    -> std::optional<T> {
    try {
        return read(timeout);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

template <TriviallyCopyable T>
void SharedMemory<T>::writeSpan(std::span<const std::byte> data,
                                std::chrono::milliseconds timeout) {
    if (data.size_bytes() > sizeof(T)) {
        THROW_INVALID_ARGUMENT("Span write out of bounds");
    }
    withLock(
        [&]() {
            std::memcpy(static_cast<char*>(buffer_) + sizeof(std::atomic_flag),
                        data.data(), data.size_bytes());
            DLOG_F(INFO, "Span data written to shared memory.");
        },
        timeout);
}

template <TriviallyCopyable T>
auto SharedMemory<T>::readSpan(std::span<std::byte> data,
                               std::chrono::milliseconds timeout) const
    -> std::size_t {
    return withLock(
        [&]() {
            std::size_t bytesToRead = std::min(data.size_bytes(), sizeof(T));
            std::memcpy(
                data.data(),
                static_cast<const char*>(buffer_) + sizeof(std::atomic_flag),
                bytesToRead);
            DLOG_F(INFO, "Span data read from shared memory.");
            return bytesToRead;
        },
        timeout);
}

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_SHARED_MEMORY_HPP
