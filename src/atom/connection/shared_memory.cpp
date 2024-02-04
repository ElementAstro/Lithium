/*
 * shared_memory.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Inter-process shared memory for local driver communication.

*************************************************/

#include "shared_memory.hpp"

SharedMemory::~SharedMemory()
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

void SharedMemory::clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    *reinterpret_cast<bool *>(buffer_) = false;

    DLOG_F(INFO, "Shared memory cleared.");
}

bool SharedMemory::isOccupied() const
{
    return mutex_->test_and_set();
}