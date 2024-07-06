/*
 * shared_memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Inter-process shared memory for local driver communication.

*************************************************/

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

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace atom::connection {
/**
 * @brief A shared memory class for inter-process communication.
 *
 * @tparam T The type of data to be stored in shared memory. Must be
 * TriviallyCopyable.
 */
template <TriviallyCopyable T>
class SharedMemory : public NonCopyable {
public:
    /**
     * @brief Construct a new SharedMemory object.
     *
     * @param name The name of the shared memory segment.
     * @param create If true, create a new shared memory segment; if false, open
     * an existing one.
     */
    explicit SharedMemory(std::string_view name, bool create = true);

    /**
     * @brief Destroy the SharedMemory object and release associated resources.
     */
    ~SharedMemory() final;

    /**
     * @brief Write data to the shared memory.
     *
     * @param data The data to write.
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     */
    void write(const T& data, std::chrono::milliseconds timeout =
                                  std::chrono::milliseconds(0));

    /**
     * @brief Read data from the shared memory.
     *
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     * @return The data read from shared memory.
     */
    [[nodiscard]] auto read(std::chrono::milliseconds timeout =
                                std::chrono::milliseconds(0)) const -> T;

    /**
     * @brief Clear the contents of the shared memory.
     */
    void clear();

    /**
     * @brief Check if the shared memory is currently occupied.
     *
     * @return true if occupied, false otherwise.
     */
    [[nodiscard]] auto isOccupied() const -> bool;

    /**
     * @brief Get the name of the shared memory segment.
     *
     * @return The name of the shared memory segment.
     */
    [[nodiscard]] auto getName() const ATOM_NOEXCEPT -> std::string_view;

    /**
     * @brief Get the size of the shared memory segment.
     *
     * @return The size of the shared memory segment in bytes.
     */
    [[nodiscard]] auto getSize() const ATOM_NOEXCEPT -> std::size_t;

    /**
     * @brief Check if this object created the shared memory segment.
     *
     * @return true if this object created the segment, false if it opened an
     * existing one.
     */
    [[nodiscard]] auto isCreator() const ATOM_NOEXCEPT -> bool;

    /**
     * @brief Write a partial amount of data to the shared memory.
     *
     * @tparam U The type of data to write. Must be TriviallyCopyable.
     * @param data The data to write.
     * @param offset The offset in bytes where to start writing.
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     */
    template <typename U>
    void writePartial(
        const U& data, std::size_t offset,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    /**
     * @brief Read a partial amount of data from the shared memory.
     *
     * @tparam U The type of data to read. Must be TriviallyCopyable.
     * @param offset The offset in bytes where to start reading.
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     * @return The data read from shared memory.
     */
    template <typename U>
    [[nodiscard]] auto readPartial(std::size_t offset,
                                   std::chrono::milliseconds timeout =
                                       std::chrono::milliseconds(0)) const -> U;

    /**
     * @brief Attempt to read data from the shared memory, returning an
     * optional.
     *
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     * @return std::optional<T> containing the data if successful, or
     * std::nullopt if the operation failed.
     */
    [[nodiscard]] auto tryRead(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
        -> std::optional<T>;

    /**
     * @brief Write a span of bytes to the shared memory.
     *
     * @param data The span of bytes to write.
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     */
    void writeSpan(
        std::span<const std::byte> data,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    /**
     * @brief Read a span of bytes from the shared memory.
     *
     * @param data The span to fill with read data.
     * @param timeout Maximum time to wait for the operation. 0 means no
     * timeout.
     * @return The number of bytes actually read.
     */
    [[nodiscard]] auto readSpan(
        std::span<std::byte> data,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
        -> std::size_t;

private:
    std::string name_;  ///< The name of the shared memory segment.
#ifdef _WIN32
    HANDLE handle_;  ///< Windows-specific handle to the shared memory.
#else
    int fd_;  ///< Unix-specific file descriptor for the shared memory.
#endif
    void* buffer_;              ///< Pointer to the shared memory buffer.
    std::atomic_flag* flag_;    ///< Atomic flag for synchronization.
    mutable std::mutex mutex_;  ///< Mutex for thread-safety.
    bool is_creator_;  ///< Whether this object created the shared memory
                       ///< segment.

    /**
     * @brief Helper function to perform operations with proper locking.
     *
     * @tparam Func The type of the function to execute.
     * @param func The function to execute while holding the lock.
     * @param timeout Maximum time to wait for acquiring the lock.
     * @return The result of the executed function.
     */
    template <typename Func>
    auto withLock(Func&& func, std::chrono::milliseconds timeout) const;
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
auto SharedMemory<T>::withLock(Func&& func,
                               std::chrono::milliseconds timeout) const {
    std::unique_lock lock(mutex_);
    auto startTime = std::chrono::steady_clock::now();
    while (flag_->test_and_set(std::memory_order_acquire)) {
        if (timeout != std::chrono::milliseconds(0) &&
            std::chrono::steady_clock::now() - startTime >= timeout) {
            THROW_TIMEOUT_EXCEPTION("Failed to acquire mutex within timeout.");
        }
        std::this_thread::yield();
    }
    auto result = std::forward<Func>(func)();
    flag_->clear(std::memory_order_release);
    return result;
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