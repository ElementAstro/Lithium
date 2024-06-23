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
#include <cstdint>
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
    ~MemoryPool() = default;

    /**
     * @brief Allocates memory for n objects of type T.
     *
     * @param n The number of objects to allocate memory for.
     * @return A pointer to the allocated memory.
     * @throw std::bad_alloc If the memory allocation fails.
     */
    T* allocate(size_t n);

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
    bool do_is_equal(
        const std::pmr::memory_resource& other) const noexcept override;

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
    size_t max_size() const;

    /**
     * @brief Gets the available space in a chunk for allocation.
     *
     * @return The available space in bytes.
     */
    size_t chunk_space() const;

    /**
     * @brief Allocates memory from the pool if available.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory, or nullptr if allocation
     * fails.
     */
    T* allocate_from_pool(size_t num_bytes);

    /**
     * @brief Deallocates memory back to the pool.
     *
     * @param p A pointer to the memory to deallocate.
     * @param num_bytes The number of bytes to deallocate.
     */
    void deallocate_to_pool(T* p, size_t num_bytes);

    /**
     * @brief Allocates a new chunk of memory and returns a pointer to it.
     *
     * @param num_bytes The number of bytes to allocate.
     * @return A pointer to the allocated memory.
     */
    T* allocate_from_chunk(size_t num_bytes);

    /**
     * @brief Deallocates memory back to a chunk.
     *
     * @param p A pointer to the memory to deallocate.
     * @param num_bytes The number of bytes to deallocate.
     */
    void deallocate_to_chunk(T* p, size_t num_bytes);

    /**
     * @brief Checks if the memory pointer belongs to the pool.
     *
     * @param p A pointer to the memory to check.
     * @return true If the memory belongs to the pool.
     * @return false Otherwise.
     */
    bool is_from_pool(T* p);

protected:
    /**
     * @brief Allocates memory with the specified alignment.
     *
     * @param bytes The number of bytes to allocate.
     * @param alignment The alignment requirement.
     * @return A pointer to the allocated memory.
     * @throw std::bad_alloc If the memory allocation fails.
     */
    void* do_allocate(size_t bytes, size_t alignment) override;

    /**
     * @brief Deallocates memory previously allocated with do_allocate.
     *
     * @param p A pointer to the memory to deallocate.
     * @param bytes The number of bytes to deallocate.
     * @param alignment The alignment requirement.
     */
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;

    std::vector<Chunk> pool_;  ///< The memory pool containing chunks.
    std::mutex mutex_;         ///< A mutex for thread-safe access to the pool.
};

#include "memory.tpp"

#endif  // ATOM_MEMORY_MEMORY_POOL_HPP