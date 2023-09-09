#pragma once

#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

int readFdError(int fd);
#ifdef _WIN32
void* attachSharedBuffer(HANDLE fileHandle, size_t& size);
void detachSharedBuffer(void* ptr);
#else
void *attachSharedBuffer(int fd, size_t &size);
void detachSharedBuffer(int fd, void *ptr, size_t size);
#endif