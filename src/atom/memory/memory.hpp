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

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <vector>

#include "atom/type/noncopyable.hpp"

/**
 * @brief A memory pool class for efficient memory allocation and deallocation.
 *
 * This class provides a simple memory pool implementation that supports
 * allocating and deallocating memory for objects of type T. It inherits from
 * std::pmr::memory_resource to be used with C++17 polymorphic memory resources.
 *
 * @tparam T The type of objects this memory pool allocates.
 * @tparam BlockSize The size of each memory block in bytes. Defaults to 4096.
 */
template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
public:
    /**
     * @brief Constructs a MemoryPool object.
     */
    MemoryPool() = default;

    /**
     * @brief Destructs the MemoryPool object and releases all allocated memory.
     */
    ~MemoryPool() override = default;

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * @param n The number of objects to allocate memory for.
     * @return A pointer to the allocated memory.
     * @throw std::bad_alloc If the memory allocation fails.
     */
    auto allocate(size_t n) -> T*;

    /**
     * @brief Deallocates memory for n objects of type T.
     *
     * @param p A pointer to the memory to deallocate.
     * @param n The number of objects the memory was allocated for.
     */
    void deallocate(T* p, size_t n);

    /**
     * @brief Compares this memory resource to another for equality.
     *
     * @param other The other memory resource to compare to.
     * @return true If the memory resources are equal.
     * @return false Otherwise.
     */
    [[nodiscard]] auto do_is_equal(
        const std::pmr::memory_resource& other) const noexcept -> bool override;

private:
    /**
     * @brief A struct representing a chunk of memory in the pool.
     */
    struct Chunk {
        size_t size;  ///< The size of the chunk in bytes.
        size_t used;  ///< The number of bytes currently used in the chunk.
        std::unique_ptr<std::byte[]> memory;  ///< The memory block.

        /**
         * @brief Constructs a Chunk object with the given size.
         *
         * @param s The size of the chunk in bytes.
         */
        explicit Chunk(size_t s);
    };

    /**
     * @brief Gets the maximum size of a single allocation.
     *
     * @return The maximum size in bytes.
     */
    [[nodiscard]] auto maxSize() const -> size_t;

    /**
     * @brief Gets the available space in a chunk for allocation.
     *
     * @return The available space in bytes.
     */
    [[nodiscard]] auto chunkSpace() const -> size_t;

    /**
     * @brief Allocates memory from the pool if available.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory, or nullptr if allocation
     * fails.
     */
    auto allocateFromPool(size_t num_bytes) -> T*;

    /**
     * @brief Deallocates memory back to the pool.
     *
     * @param p A pointer to the memory to deallocate.
     * @param num_bytes The number of bytes to deallocate.
     */
    void deallocateToPool(T* p, size_t num_bytes);

    /**
     * @brief Allocates a new chunk of memory and returns a pointer to it.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory.
     */
    auto allocateFromChunk(size_t num_bytes) -> T*;

    /**
     * @brief Deallocates memory back to a chunk.
     *
     * @param p A pointer to the memory to deallocate.
     * @param num_bytes The number of bytes to deallocate.
     */
    void deallocateToChunk(T* p, size_t num_bytes);

    /**
     * @brief Checks if the memory pointer belongs to the pool.
     *
     * @param p A pointer to the memory to check.
     * @return true If the memory belongs to the pool.
     * @return false Otherwise.
     */
    auto isFromPool(T* p) -> bool;

protected:
    /**
     * @brief Allocates memory with the specified alignment.
     *
     * @param bytes The number of bytes to allocate.
     * @param alignment The alignment requirement.
     * @return A pointer to the allocated memory.
     * @throw std::bad_alloc If the memory allocation fails.
     */
    auto do_allocate(size_t bytes, size_t alignment) -> void* override;

    /**
     * @brief Deallocates memory previously allocated with do_allocate.
     *
     * @param p A pointer to the memory to deallocate.
     * @param bytes The number of bytes to deallocate.
     * @param alignment The alignment requirement.
     */
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;

private:
    std::vector<Chunk> pool_;  ///< The memory pool containing chunks.
    std::mutex mutex_;         ///< A mutex for thread-safe access to the pool.
};

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
auto MemoryPool<T, BlockSize>::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept -> bool {
    return this == &other;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::Chunk::Chunk(size_t s)
    : size(s), used(), memory(new std::byte[s]) {}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::maxSize() const -> size_t {
    return BlockSize - sizeof(T);
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::chunkSpace() const -> size_t {
    return BlockSize - sizeof(Chunk);
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