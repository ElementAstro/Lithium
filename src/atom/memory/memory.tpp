/*
 * memory.tpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: A simple implementation of memory pool

**************************************************/

#include <algorithm>
#include <cassert>
#include <new>
#include "memory.hpp"

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate(size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t num_bytes = n * sizeof(T);
    if (num_bytes > max_size()) {
        throw std::bad_alloc();
    }

    if (auto p = allocate_from_pool(num_bytes)) {
        return p;
    }
    return allocate_from_chunk(num_bytes);
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t num_bytes = n * sizeof(T);
    if (is_from_pool(p)) {
        deallocate_to_pool(p, num_bytes);
    } else {
        deallocate_to_chunk(p, num_bytes);
    }
}

template <typename T, size_t BlockSize>
bool MemoryPool<T, BlockSize>::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::Chunk::Chunk(size_t s)
    : size(s), used(0), memory(new std::byte[s]) {}

template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::max_size() const {
    return BlockSize - sizeof(T);
}

template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::chunk_space() const {
    return BlockSize - sizeof(Chunk);
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate_from_pool(size_t num_bytes) {
    if (pool_.empty() || pool_.back().used + num_bytes > pool_.back().size) {
        return nullptr;
    }

    auto& chunk = pool_.back();
    T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
    chunk.used += num_bytes;
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate_to_pool(T* p, size_t num_bytes) {
    auto it = std::find_if(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
        return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
               reinterpret_cast<std::byte*>(p) <
                   chunk.memory.get() + chunk.size;
    });
    assert(it != pool_.end());
    it->used -= num_bytes;
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate_from_chunk(size_t num_bytes) {
    pool_.emplace_back(std::max(num_bytes, chunk_space()));
    auto& chunk = pool_.back();
    T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
    chunk.used += num_bytes;
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate_to_chunk(T* p, size_t num_bytes) {
    auto it = std::find_if(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
        return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
               reinterpret_cast<std::byte*>(p) <
                   chunk.memory.get() + chunk.size;
    });
    assert(it != pool_.end());
    it->used -= num_bytes;
    if (it->used == 0) {
        pool_.erase(it);
    }
}

template <typename T, size_t BlockSize>
bool MemoryPool<T, BlockSize>::is_from_pool(T* p) {
    return std::any_of(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
        return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
               reinterpret_cast<std::byte*>(p) <
                   chunk.memory.get() + chunk.size;
    });
}

template <typename T, size_t BlockSize>
void* MemoryPool<T, BlockSize>::do_allocate(size_t bytes, size_t alignment) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t space = bytes;
    void* p = std::malloc(bytes + alignment);
    if (!p) {
        throw std::bad_alloc();
    }

    void* aligned = p;
    if (!std::align(alignment, bytes, aligned, space)) {
        std::free(p);
        throw std::bad_alloc();
    }
    return aligned;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::do_deallocate(void* p, size_t bytes,
                                             size_t alignment) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::free(p);
}