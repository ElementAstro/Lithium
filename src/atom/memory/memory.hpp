#ifndef ATOM_MEMORY_MEMORY_POOL_HPP
#define ATOM_MEMORY_MEMORY_POOL_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <vector>

#include "atom/type/noncopyable.hpp"

template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
public:
    MemoryPool() = default;
    ~MemoryPool() override = default;

    auto allocate(size_t n) -> T*;
    void deallocate(T* p, size_t n);

    [[nodiscard]] auto do_is_equal(
        const std::pmr::memory_resource& other) const noexcept -> bool override;

private:
    struct Chunk {
        size_t size;
        size_t used;
        std::unique_ptr<std::byte[]> memory;
        explicit Chunk(size_t s);
    };

    [[nodiscard]] auto maxSize() const -> size_t;
    [[nodiscard]] auto chunkSpace() const -> size_t;

    auto allocateFromPool(size_t num_bytes) -> T*;
    void deallocateToPool(T* p, size_t num_bytes);

    auto allocateFromChunk(size_t num_bytes) -> T*;
    void deallocateToChunk(T* p, size_t num_bytes);

    auto isFromPool(T* p) -> bool;

protected:
    auto do_allocate(size_t bytes, size_t alignment) -> void* override;
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;

private:
    std::vector<Chunk> pool_;
    std::mutex mutex_;
};

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::Chunk::Chunk(size_t s)
    : size(s), used(0), memory(new std::byte[s]) {}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::maxSize() const -> size_t {
    return BlockSize;
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::chunkSpace() const -> size_t {
    return BlockSize;
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::allocate(size_t n) -> T* {
    std::lock_guard lock(mutex_);
    size_t numBytes = n * sizeof(T);
    if (numBytes > maxSize()) {
        throw std::bad_alloc();
    }

    if (auto p = allocateFromPool(numBytes)) {
        return p;
    }
    return allocateFromChunk(numBytes);
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n) {
    std::lock_guard lock(mutex_);
    size_t numBytes = n * sizeof(T);
    if (isFromPool(p)) {
        deallocateToPool(p, numBytes);
    } else {
        deallocateToChunk(p, numBytes);
    }
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::allocateFromPool(size_t num_bytes) -> T* {
    if (pool_.empty() || pool_.back().used + num_bytes > pool_.back().size) {
        return nullptr;
    }

    auto& chunk = pool_.back();
    T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
    chunk.used += num_bytes;
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocateToPool(T* p, size_t num_bytes) {
    auto it = std::find_if(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
        return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
               reinterpret_cast<std::byte*>(p) <
                   chunk.memory.get() + chunk.size;
    });
    assert(it != pool_.end());
    it->used -= num_bytes;
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::allocateFromChunk(size_t num_bytes) -> T* {
    pool_.emplace_back(std::max(num_bytes, chunkSpace()));
    auto& chunk = pool_.back();
    T* p = reinterpret_cast<T*>(chunk.memory.get() + chunk.used);
    chunk.used += num_bytes;
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocateToChunk(T* p, size_t num_bytes) {
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
auto MemoryPool<T, BlockSize>::isFromPool(T* p) -> bool {
    return std::any_of(pool_.begin(), pool_.end(), [p](const Chunk& chunk) {
        return chunk.memory.get() <= reinterpret_cast<std::byte*>(p) &&
               reinterpret_cast<std::byte*>(p) <
                   chunk.memory.get() + chunk.size;
    });
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept -> bool {
    return this == &other;
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::do_allocate(size_t bytes,
                                           size_t alignment) -> void* {
    std::lock_guard lock(mutex_);
    size_t space = bytes;
    void* p = std::malloc(bytes + alignment);
    if (!p) {
        throw std::bad_alloc();
    }

    void* aligned = p;
    if (std::align(alignment, bytes, aligned, space) == nullptr) {
        std::free(p);
        throw std::bad_alloc();
    }
    return aligned;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::do_deallocate(void* p, size_t /*bytes*/,
                                             size_t /*alignment*/) {
    std::lock_guard lock(mutex_);
    std::free(p);
}

#endif  // ATOM_MEMORY_MEMORY_POOL_HPP
