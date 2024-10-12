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

/**
 * @brief A memory pool for efficient memory allocation and deallocation.
 *
 * This class provides a memory pool that allocates memory in chunks to reduce
 * the overhead of frequent allocations and deallocations. It is thread-safe
 * and uses a mutex to protect shared resources.
 *
 * @tparam T The type of objects to allocate.
 * @tparam BlockSize The size of each memory block in bytes.
 */
template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
public:
    /**
     * @brief Constructs a MemoryPool object.
     */
    MemoryPool() = default;

    /**
     * @brief Destructs the MemoryPool object.
     */
    ~MemoryPool() override = default;

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * @param n The number of objects to allocate.
     * @return A pointer to the allocated memory.
     * @throws std::bad_alloc if the allocation fails.
     */
    auto allocate(size_t n) -> T*;

    /**
     * @brief Deallocates memory for n objects of type T.
     *
     * @param p A pointer to the memory to deallocate.
     * @param n The number of objects to deallocate.
     */
    void deallocate(T* p, size_t n);

    /**
     * @brief Checks if this memory resource is equal to another.
     *
     * @param other The other memory resource to compare with.
     * @return True if the memory resources are equal, false otherwise.
     */
    [[nodiscard]] auto do_is_equal(
        const std::pmr::memory_resource& other) const noexcept -> bool override;

private:
    /**
     * @brief A struct representing a chunk of memory.
     */
    struct Chunk {
        size_t size;  ///< The size of the chunk.
        size_t used;  ///< The amount of memory used in the chunk.
        std::unique_ptr<std::byte[]> memory;  ///< The memory block.

        /**
         * @brief Constructs a Chunk object.
         *
         * @param s The size of the chunk.
         */
        explicit Chunk(size_t s);
    };

    /**
     * @brief Gets the maximum size of a memory block.
     *
     * @return The maximum size of a memory block.
     */
    [[nodiscard]] auto maxSize() const -> size_t;

    /**
     * @brief Gets the available space in the current chunk.
     *
     * @return The available space in the current chunk.
     */
    [[nodiscard]] auto chunkSpace() const -> size_t;

    /**
     * @brief Allocates memory from the pool.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory.
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
     * @brief Allocates memory from a new chunk.
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
     * @brief Checks if a pointer is from the pool.
     *
     * @param p The pointer to check.
     * @return True if the pointer is from the pool, false otherwise.
     */
    auto isFromPool(T* p) -> bool;

protected:
    /**
     * @brief Allocates memory with a specified alignment.
     *
     * @param bytes The number of bytes to allocate.
     * @param alignment The alignment of the memory.
     * @return A pointer to the allocated memory.
     * @throws std::bad_alloc if the allocation fails.
     */
    auto do_allocate(size_t bytes, size_t alignment) -> void* override;

    /**
     * @brief Deallocates memory with a specified alignment.
     *
     * @param p A pointer to the memory to deallocate.
     * @param bytes The number of bytes to deallocate.
     * @param alignment The alignment of the memory.
     */
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;

private:
    std::vector<Chunk> pool_;  ///< The pool of memory chunks.
    std::mutex mutex_;         ///< Mutex to protect shared resources.
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
