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

#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>

#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
#else  // Unix-like
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace atom::connection {
/**
 * @brief 实现共享内存,可用于进程间通信
 */
template <typename T>
class SharedMemory {
public:
    /**
     * @brief 构造函数,创建指定大小的共享内存
     * @param name 共享内存名称
     * @param create 是否创建新的共享内存
     */
    explicit SharedMemory(std::string_view name, bool create = true);

    /**
     * @brief 析构函数,释放共享内存
     */
    ~SharedMemory();

    /**
     * @brief 将数据写入到共享内存中
     * @param data 要写入的数据
     * @param timeout 超时时间,默认为0,表示不设超时时间
     */
    void write(const T& data, std::chrono::milliseconds timeout =
                                  std::chrono::milliseconds(0));

    /**
     * @brief 从共享内存中读取数据
     * @param timeout 超时时间,默认为0,表示不设超时时间
     * @return 读取到的数据
     */
    [[nodiscard]] T read(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const;

    /**
     * @brief 清空共享内存中的数据
     */
    void clear();

    /**
     * @brief 判断共享内存是否被占用
     * @return 如果共享内存已被占用,返回true；否则返回false
     */
    [[nodiscard]] bool isOccupied() const;

    /**
     * @brief 获取共享内存的名称
     * @return 共享内存的名称
     */
    [[nodiscard]] std::string_view getName() const noexcept;

    /**
     * @brief 获取共享内存的大小
     * @return 共享内存的大小,单位为字节
     */
    [[nodiscard]] std::size_t getSize() const noexcept;

    /**
     * @brief 判断当前进程是否是共享内存的创建者
     * @return 如果当前进程是共享内存的创建者,返回true；否则返回false
     */
    [[nodiscard]] bool isCreator() const noexcept;

private:
    std::string name_;                  ///< 共享内存名称
#if defined(_WIN32) || defined(_WIN64)  // Windows
    HANDLE handle_;                     ///< 共享内存句柄
#else                                   // Unix-like
    int fd_;  ///< 共享内存文件描述符
#endif
    void* buffer_;            ///< 共享内存指针
    std::atomic_flag* flag_;  ///< 互斥锁,用于保证并发访问时数据的一致性
    mutable std::mutex mutex_;  ///< 互斥量,用于保护互斥锁的修改
    bool is_creator_;           ///< 是否是创建者
};

template <typename T>
SharedMemory<T>::SharedMemory(std::string_view name, bool create)
    : name_(name), buffer_(nullptr), flag_(), is_creator_(create) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be a trivially copyable type.");
    static_assert(std::is_standard_layout_v<T>,
                  "T must be a standard layout type.");

#if defined(_WIN32) || defined(_WIN64)  // Windows
    if (is_creator_) {
        handle_ = CreateFileMappingA(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
            sizeof(T) + sizeof(std::atomic_flag), name.data());
        if (handle_ == nullptr) {
            LOG_F(ERROR, "Failed to create file mapping.");
            throw std::runtime_error("Failed to create file mapping.");
        }
    } else {
        handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.data());
        if (handle_ == nullptr) {
            LOG_F(ERROR, "Failed to open file mapping.");
            throw std::runtime_error("Failed to open file mapping.");
        }
    }

    buffer_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0,
                            sizeof(T) + sizeof(std::atomic_flag));
    if (buffer_ == nullptr) {
        CloseHandle(handle_);
        LOG_F(ERROR, "Failed to map view of file.");
        throw std::runtime_error("Failed to map view of file.");
    }
#else  // Unix-like
    if (is_creator_) {
        fd_ = shm_open(name.data(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd_ == -1) {
            LOG_F(ERROR, "Failed to create shared memory.");
            throw std::runtime_error("Failed to create shared memory.");
        }

        if (ftruncate(fd_, sizeof(T) + sizeof(std::atomic_flag)) == -1) {
            close(fd_);
            shm_unlink(name.data());
            LOG_F(ERROR, "Failed to resize shared memory.");
            throw std::runtime_error("Failed to resize shared memory.");
        }
    } else {
        fd_ = shm_open(name.data(), O_RDWR, S_IRUSR | S_IWUSR);
        if (fd_ == -1) {
            LOG_F(ERROR, "Failed to open shared memory.");
            throw std::runtime_error("Failed to open shared memory.");
        }
    }

    buffer_ = mmap(nullptr, sizeof(T) + sizeof(std::atomic_flag),
                   PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    close(fd_);

    if (buffer_ == MAP_FAILED) {
        if (is_creator_) {
            shm_unlink(name.data());
        }
        LOG_F(ERROR, "Failed to map shared memory.");
        throw std::runtime_error("Failed to map shared memory.");
    }
#endif
    flag_ = new (buffer_) std::atomic_flag();
}

template <typename T>
SharedMemory<T>::~SharedMemory() {
#if defined(_WIN32) || defined(_WIN64)  // Windows
    UnmapViewOfFile(buffer_);
    CloseHandle(handle_);
#else  // Unix-like
    munmap(buffer_, sizeof(T) + sizeof(std::atomic_flag));
    if (is_creator_) {
        shm_unlink(name_.c_str());
    }
#endif
}

template <typename T>
void SharedMemory<T>::write(const T& data, std::chrono::milliseconds timeout) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be a trivially copyable type.");
    static_assert(std::is_standard_layout_v<T>,
                  "T must be a standard layout type.");

    std::unique_lock<std::mutex> lock(mutex_);

    auto start_time = std::chrono::steady_clock::now();
    while (flag_->test_and_set(std::memory_order_acquire)) {
        if (timeout != std::chrono::milliseconds(0)) {
            auto elapsed_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start_time);
            if (elapsed_time >= timeout) {
                LOG_F(ERROR, "Failed to acquire mutex within timeout.");
                throw std::runtime_error(
                    "Failed to acquire mutex within timeout.");
            }
        }
        std::this_thread::yield();
    }

    std::memcpy(static_cast<char*>(buffer_) + sizeof(std::atomic_flag), &data,
                sizeof(T));

    DLOG_F(INFO, "Data written to shared memory.");

    flag_->clear(std::memory_order_release);
}

template <typename T>
[[nodiscard]] T SharedMemory<T>::read(std::chrono::milliseconds timeout) const {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be a trivially copyable type.");
    static_assert(std::is_standard_layout_v<T>,
                  "T must be a standard layout type.");

    std::unique_lock<std::mutex> lock(mutex_);

    auto start_time = std::chrono::steady_clock::now();
    while (flag_->test_and_set(std::memory_order_acquire)) {
        if (timeout != std::chrono::milliseconds(0)) {
            auto elapsed_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start_time);
            if (elapsed_time >= timeout) {
                LOG_F(ERROR, "Failed to acquire mutex within timeout.");
                throw std::runtime_error(
                    "Failed to acquire mutex within timeout.");
            }
        }
        std::this_thread::yield();
    }

    T data;
    std::memcpy(&data,
                static_cast<const char*>(buffer_) + sizeof(std::atomic_flag),
                sizeof(T));

    DLOG_F(INFO, "Data read from shared memory.");

    flag_->clear(std::memory_order_release);

    return data;
}

template <typename T>
void SharedMemory<T>::clear() {
    std::unique_lock<std::mutex> lock(mutex_);
    std::memset(static_cast<char*>(buffer_) + sizeof(std::atomic_flag), 0,
                sizeof(T));
    DLOG_F(INFO, "Shared memory cleared.");
}

template <typename T>
[[nodiscard]] bool SharedMemory<T>::isOccupied() const {
    return flag_->test(std::memory_order_acquire);
}

template <typename T>
[[nodiscard]] std::string_view SharedMemory<T>::getName() const noexcept {
    return name_;
}

template <typename T>
[[nodiscard]] std::size_t SharedMemory<T>::getSize() const noexcept {
    return sizeof(T);
}

template <typename T>
[[nodiscard]] bool SharedMemory<T>::isCreator() const noexcept {
    return is_creator_;
}
}  // namespace atom::connection

#endif