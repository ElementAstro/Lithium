/*
 * memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: A simple implementation of memory pool

**************************************************/

#ifndef ATOM_MEMORY_MEMORY_POOL_HPP
#define ATOM_MEMORY_MEMORY_POOL_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <vector>

#include "atom/type/noncopyable.hpp"

template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
public:
    MemoryPool() = default;
    ~MemoryPool() = default;

    T* allocate(size_t n) {
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

    void deallocate(T* p, size_t n) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t num_bytes = n * sizeof(T);
        if (is_from_pool(p)) {
            deallocate_to_pool(p, num_bytes);
        } else {
            deallocate_to_chunk(p, num_bytes);
        }
    }

    bool do_is_equal(
        const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    struct Chunk {
        size_t size;
        size_t used;
        std::unique_ptr<std::byte[]> memory;

        explicit Chunk(size_t s) : size(s), used(0), memory(new std::byte[s]) {}
    };

    size_t max_size() const { return BlockSize - sizeof(T); }
    size_t chunk_space() const { return BlockSize - sizeof(Chunk); }

    T* allocate_from_pool(size_t num_bytes) {
        if (pool_.empty() ||
            pool_.back().used + num_bytes > pool_.back().size) {
            return nullptr;
        }

        auto& chunk = pool_.back();
        T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
        chunk.used += num_bytes;
        return p;
    }

    void deallocate_to_pool(T* p, size_t num_bytes) {
        auto it =
            std::find_if(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
                return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
                       reinterpret_cast<std::byte*>(p) <
                           chunk.memory.get() + chunk.size;
            });
        assert(it != pool_.end());
        it->used -= num_bytes;
    }

    T* allocate_from_chunk(size_t num_bytes) {
        pool_.emplace_back(std::max(num_bytes, chunk_space()));
        auto& chunk = pool_.back();
        T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
        chunk.used += num_bytes;
        return p;
    }

    void deallocate_to_chunk(T* p, size_t num_bytes) {
        auto it =
            std::find_if(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
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

    bool is_from_pool(T* p) {
        return std::any_of(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
            return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
                   reinterpret_cast<std::byte*>(p) <
                       chunk.memory.get() + chunk.size;
        });
    }

protected:
    void* do_allocate(size_t bytes, size_t alignment) override {
        std::lock_guard<std::mutex> lock(mutex_);
        // Compute the total space needed with alignment padding
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

    void do_deallocate(void* p, [[maybe_unused]] size_t bytes,
                       [[maybe_unused]] size_t alignment) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::free(p);  // Direct deallocation of the original pointer
    }

    std::vector<Chunk> pool_;
    std::mutex mutex_;
};

#endif  // ATOM_MEMORY_MEMORY_POOL_HPP
