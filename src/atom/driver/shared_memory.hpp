/*
 * shared_memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Inter-process shared memory for local driver communication.

*************************************************/

#pragma once

#define ATOM_DRIVER_SHARED_MEMORY

#include <type_traits>
#include <cstring>
#include <mutex>
#include <chrono>

#include "atom/log/loguru.hpp"

#if defined(_WIN32) || defined(_WIN64) // Windows
#include <windows.h>
#else // Unix-like
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class SharedMemory
{
public:
    template <typename T>
    SharedMemory(const std::string &name, bool create = true)
        : name_(name), buffer_(nullptr), mutex_{}, is_creator_(create)
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

#if defined(_WIN32) || defined(_WIN64) // Windows
        if (is_creator_)
        {
            handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(T) + sizeof(bool), name.c_str());
            if (handle_ == nullptr)
            {
                LOG_F(ERROR, "Failed to create file mapping.");
                throw std::runtime_error("Failed to create file mapping.");
            }
        }
        else
        {
            handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
            if (handle_ == nullptr)
            {
                LOG_F(ERROR, "Failed to open file mapping.");
                throw std::runtime_error("Failed to open file mapping.");
            }
        }

        buffer_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T) + sizeof(bool));
        if (buffer_ == nullptr)
        {
            CloseHandle(handle_);
            LOG_F(ERROR, "Failed to map view of file.");
            throw std::runtime_error("Failed to map view of file.");
        }
#else // Unix-like
        if (is_creator_)
        {
            fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (fd_ == -1)
            {
                LOG_F(ERROR, "Failed to create shared memory.");
                throw std::runtime_error("Failed to create shared memory.");
            }

            ftruncate(fd_, sizeof(T) + sizeof(bool));
        }
        else
        {
            fd_ = shm_open(name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
            if (fd_ == -1)
            {
                LOG_F(ERROR, "Failed to open shared memory.");
                throw std::runtime_error("Failed to open shared memory.");
            }
        }

        buffer_ = mmap(nullptr, sizeof(T) + sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        close(fd_);

        if (buffer_ == MAP_FAILED)
        {
            if (is_creator_)
            {
                shm_unlink(name.c_str());
            }
            LOG_F(ERROR, "Failed to map shared memory.");
            throw std::runtime_error("Failed to map shared memory.");
        }
#endif
        mutex_ = reinterpret_cast<std::atomic_flag *>(buffer_);
        new (mutex_) std::atomic_flag();
    }

    ~SharedMemory()
    {
#if defined(_WIN32) || defined(_WIN64) // Windows
        UnmapViewOfFile(buffer_);
        CloseHandle(handle_);
#else // Unix-like
        munmap(buffer_, sizeof(T) + sizeof(bool));
        if (is_creator_)
        {
            shm_unlink(name_.c_str());
        }
#endif
    }

    template <typename T>
    void Write(const T &data, std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

        std::unique_lock<std::mutex> lock(mutex_);

        auto start_time = std::chrono::steady_clock::now();
        while (mutex_->test_and_set())
        {
            if (timeout.count() > 0)
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time) >= timeout)
                {
                    LOG_F(ERROR, "Failed to acquire mutex within timeout.");
                    throw std::runtime_error("Failed to acquire mutex within timeout.");
                }
            }
            else
            {
                std::this_thread::yield();
            }
        }

        std::memcpy(static_cast<char *>(buffer_) + sizeof(bool), &data, sizeof(T));

        DLOG_F(INFO, "Data written to shared memory.");
        *reinterpret_cast<bool *>(buffer_) = true;

        mutex_->clear();
    }

    template <typename T>
    T Read(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

        std::unique_lock<std::mutex> lock(mutex_);

        auto start_time = std::chrono::steady_clock::now();
        while (!(*reinterpret_cast<bool *>(buffer_)))
        {
            if (timeout.count() > 0)
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time) >= timeout)
                {
                    LOG_F(ERROR, "Failed to acquire mutex within timeout.");
                    throw std::runtime_error("Failed to acquire mutex within timeout.");
                }
            }
            else
            {
                std::this_thread::yield();
            }
        }

        T data;
        std::memcpy(&data, static_cast<const char *>(buffer_) + sizeof(bool), sizeof(T));
        *reinterpret_cast<bool *>(buffer_) = false;

        DLOG_F(INFO, "Data read from shared memory.");

        mutex_->clear();

        return data;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        *reinterpret_cast<bool *>(buffer_) = false;

        DLOG_F(INFO, "Shared memory cleared.");
    }

    bool IsOccupied() const
    {
        return mutex_->test_and_set();
    }

private:
    std::string name_;
#if defined(_WIN32) || defined(_WIN64) // Windows
    HANDLE handle_;
#else // Unix-like
    int fd_;
#endif
    void *buffer_;
    std::atomic_flag *mutex_;
    mutable std::mutex mutex_;
    bool is_creator_;
};

/*
#include <iostream>
#include <thread>
#include "shared_memory.hpp"

struct Data
{
    int value;
};

void writerProcess()
{
    // 创建共享内存对象
    SharedMemory<Data> sharedMemory("my_shared_memory", true);

    // 写入数据
    Data data;
    data.value = 42;
    sharedMemory.Write(data);

    // 等待读取进程读取数据
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void readerProcess()
{
    // 打开共享内存对象
    SharedMemory<Data> sharedMemory("my_shared_memory", false);

    // 读取数据
    Data data = sharedMemory.Read();

    // 输出数据
    std::cout << "Value read from shared memory: " << data.value << std::endl;

    // 清空共享内存
    sharedMemory.Clear();
}

int main()
{
    std::thread writerThread(writerProcess);
    std::thread readerThread(readerProcess);

    writerThread.join();
    readerThread.join();

    return 0;
}

*/