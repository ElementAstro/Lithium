// FILE: memory.hpp
#ifndef ATOM_MEMORY_MEMORY_POOL_HPP
#define ATOM_MEMORY_MEMORY_POOL_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <vector>

#include "atom/type/noncopyable.hpp"

// 自定义异常类
namespace atom::memory {

class MemoryPoolException : public std::runtime_error {
public:
    explicit MemoryPoolException(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace atom::memory

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
    MemoryPool();

    /**
     * @brief Destructs the MemoryPool object.
     */
    ~MemoryPool() override;

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * @param n The number of objects to allocate.
     * @return A pointer to the allocated memory.
     * @throws atom::memory::MemoryPoolException if the allocation fails.
     */
    T* allocate(size_t n);

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

    /**
     * @brief Resets the memory pool, freeing all allocated memory.
     */
    void reset();

    /**
     * @brief Gets the total memory allocated by the pool.
     *
     * @return The total memory allocated in bytes.
     */
    [[nodiscard]] auto getTotalAllocated() const -> size_t;

    /**
     * @brief Gets the total memory available in the pool.
     *
     * @return The total available memory in bytes.
     */
    [[nodiscard]] auto getTotalAvailable() const -> size_t;

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
    [[nodiscard]] size_t maxSize() const;

    /**
     * @brief Gets the available space in the current chunk.
     *
     * @return The available space in the current chunk.
     */
    [[nodiscard]] size_t chunkSpace() const;

    /**
     * @brief Allocates memory from the pool.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory.
     */
    T* allocateFromPool(size_t num_bytes);

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
    T* allocateFromChunk(size_t num_bytes);

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
    bool isFromPool(T* p);

protected:
    /**
     * @brief Allocates memory with a specified alignment.
     *
     * @param bytes The number of bytes to allocate.
     * @param alignment The alignment of the memory.
     * @return A pointer to the allocated memory.
     * @throws atom::memory::MemoryPoolException if the allocation fails.
     */
    void* do_allocate(size_t bytes, size_t alignment) override;

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
    std::atomic<size_t> total_allocated_;  ///< Total memory allocated.
    std::atomic<size_t> total_available_;  ///< Total memory available.
};

// Implementation

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::Chunk::Chunk(size_t s)
    : size(s), used(0), memory(std::make_unique<std::byte[]>(s)) {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
    : pool_(), total_allocated_(0), total_available_(0) {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() {
    reset();
}

template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::maxSize() const {
    return BlockSize;
}

template <typename T, size_t BlockSize>
size_t MemoryPool<T, BlockSize>::chunkSpace() const {
    return BlockSize;
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate(size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t numBytes = n * sizeof(T);
    if (numBytes > maxSize()) {
        throw atom::memory::MemoryPoolException(
            "Requested size exceeds maximum block size.");
    }

    if (T* p = allocateFromPool(numBytes)) {
        total_allocated_ += numBytes;
        total_available_ -= numBytes;
        return p;
    }

    return allocateFromChunk(numBytes);
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocate(T* p, size_t n) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t numBytes = n * sizeof(T);
    if (isFromPool(p)) {
        deallocateToPool(p, numBytes);
        total_allocated_ -= numBytes;
        total_available_ += numBytes;
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
void MemoryPool<T, BlockSize>::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.clear();
    total_allocated_ = 0;
    total_available_ = 0;
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::getTotalAllocated() const -> size_t {
    return total_allocated_.load();
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::getTotalAvailable() const -> size_t {
    return total_available_.load();
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::allocateFromPool(size_t num_bytes) -> T* {
    if (pool_.empty()) {
        return nullptr;
    }

    Chunk& current = pool_.back();
    if (current.used + num_bytes > current.size) {
        return nullptr;
    }

    T* p = reinterpret_cast<T*>(current.memory.get() + current.used);
    current.used += num_bytes;
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocateToPool(T* p, size_t num_bytes) {
    for (auto it = pool_.begin(); it != pool_.end(); ++it) {
        auto* ptr = reinterpret_cast<std::byte*>(p);
        if (ptr >= it->memory.get() && ptr < it->memory.get() + it->size) {
            it->used -= num_bytes;
            return;
        }
    }
    throw atom::memory::MemoryPoolException(
        "Pointer does not belong to any pool chunk.");
}

template <typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocateFromChunk(size_t num_bytes) {
    size_t chunkSize = std::max(num_bytes, chunkSpace());
    pool_.emplace_back(chunkSize);
    Chunk& newChunk = pool_.back();
    T* p = reinterpret_cast<T*>(newChunk.memory.get() + newChunk.used);
    newChunk.used += num_bytes;
    total_available_ += (newChunk.size - newChunk.used);
    return p;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::deallocateToChunk(T* p, size_t num_bytes) {
    for (auto it = pool_.begin(); it != pool_.end(); ++it) {
        auto* ptr = reinterpret_cast<std::byte*>(p);
        if (ptr >= it->memory.get() && ptr < it->memory.get() + it->size) {
            it->used -= num_bytes;
            if (it->used == 0) {
                pool_.erase(it);
            }
            return;
        }
    }
    throw atom::memory::MemoryPoolException(
        "Pointer does not belong to any pool chunk.");
}

template <typename T, size_t BlockSize>
auto MemoryPool<T, BlockSize>::isFromPool(T* p) -> bool {
    auto* ptr = reinterpret_cast<std::byte*>(p);
    for (const auto& chunk : pool_) {
        if (ptr >= chunk.memory.get() &&
            ptr < chunk.memory.get() + chunk.size) {
            return true;
        }
    }
    return false;
}

template <typename T, size_t BlockSize>
void* MemoryPool<T, BlockSize>::do_allocate(size_t bytes, size_t alignment) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total_bytes = bytes;
    void* p = std::malloc(bytes + alignment);
    if (!p) {
        throw atom::memory::MemoryPoolException(
            "Failed to allocate memory with std::malloc.");
    }

    void* aligned = p;
    size_t space = bytes + alignment;
    if (std::align(alignment, bytes, aligned, space) == nullptr) {
        std::free(p);
        throw atom::memory::MemoryPoolException("Failed to align memory.");
    }

    total_allocated_ += bytes;
    total_available_ += (bytes + alignment - space);

    return aligned;
}

template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::do_deallocate(void* p, size_t bytes,
                                             size_t /*alignment*/) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::free(p);
    total_allocated_ -= bytes;
    // Note: total_available_ is not updated here as we cannot determine the
    // alignment adjustment
}

#endif  // ATOM_MEMORY_MEMORY_POOL_HPP