/*
 * short_alloc.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Short Alloc from Howard Hinnant

**************************************************/

#ifndef ATOM_MEMORY_SHORT_ALLOC_HPP
#define ATOM_MEMORY_SHORT_ALLOC_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>

/**
 * @brief A fixed-size memory arena for allocating objects with a specific
 * alignment.
 *
 * This class provides a fixed-size memory arena for allocating objects of
 * arbitrary types with a specific alignment. Memory is allocated from a
 * pre-allocated buffer of size N. Allocations are aligned to the specified
 * alignment. This class does not support dynamic resizing of the underlying
 * buffer.
 *
 * @tparam N The size of the fixed-size memory arena in bytes.
 * @tparam alignment The alignment requirement for memory allocations. Defaults
 * to alignof(std::max_align_t).
 */
template <std::size_t N, std::size_t alignment = alignof(std::max_align_t)>
class arena {
    alignas(alignment) char buf_[N];
    char* ptr_;

public:
    ~arena() { ptr_ = nullptr; }
    arena() noexcept : ptr_(buf_) {}
    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;

    /**
     * @brief Allocates memory for an object with the specified size and
     * alignment.
     *
     * @param n The size of the object to allocate.
     * @return A pointer to the allocated memory, or nullptr if the allocation
     * fails.
     * @throws std::bad_alloc if the allocation fails due to insufficient space
     * in the arena.
     */
    void* allocate(std::size_t n) {
        std::size_t space = N - used();
        void* result = ptr_;
        if (!std::align(alignment, n, result, space)) {
            throw std::bad_alloc();
        }
        ptr_ = static_cast<char*>(result) + n;
        return result;
    }

    /**
     * @brief Deallocates memory previously allocated with allocate().
     *
     * @param p A pointer to the memory to deallocate.
     * @param n The size of the object that was deallocated.
     */
    void deallocate(void* p, std::size_t n) noexcept {
        // Only assert if the deallocation is exactly at the top of the stack
        if (static_cast<char*>(p) + n == ptr_) {
            ptr_ = static_cast<char*>(p);
        }
    }

    /**
     * @brief Returns the total size of the arena.
     *
     * @return The total size of the arena in bytes.
     */
    static constexpr std::size_t size() noexcept { return N; }

    /**
     * @brief Returns the amount of memory used in the arena.
     *
     * @return The amount of memory used in the arena in bytes.
     */
    std::size_t used() const noexcept {
        return static_cast<std::size_t>(ptr_ - buf_);
    }

    /**
     * @brief Resets the arena to its initial state, deallocating all memory.
     */
    void reset() noexcept { ptr_ = buf_; }

private:
    /**
     * @brief Checks if a pointer is within the buffer of the arena.
     *
     * @param p The pointer to check.
     * @return true if the pointer is within the arena's buffer, false
     * otherwise.
     */
    bool pointer_in_buffer(char* p) noexcept {
        return buf_ <= p && p <= buf_ + N;
    }
};

/**
 * @brief A simple allocator that uses a fixed-size memory arena for
 * allocations.
 *
 * This allocator provides a way to use a fixed-size memory arena for dynamic
 * allocations, eliminating the need for dynamic memory allocation from the
 * heap. It is useful in scenarios where memory allocation performance or
 * fragmentation is a concern.
 *
 * @tparam T The type of objects to allocate.
 * @tparam N The size of the fixed-size memory arena in bytes.
 * @tparam Align The alignment requirement for memory allocations. Defaults to
 * alignof(std::max_align_t).
 */
template <class T, std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class short_alloc {
public:
    using value_type = T;
    static constexpr auto alignment = Align;
    static constexpr auto size = N;
    using arena_type = arena<N, Align>;

private:
    arena_type& a_;

public:
    /**
     * @brief Constructs a short_alloc object using the specified arena.
     *
     * @param a The arena to use for allocations.
     */
    short_alloc(arena_type& a) noexcept : a_(a) {}

    /**
     * @brief Constructs a short_alloc object by copying another short_alloc
     * object.
     *
     * @tparam U The type of objects allocated by the other allocator.
     * @param a The other short_alloc object to copy from.
     */
    template <class U>
    short_alloc(const short_alloc<U, N, alignment>& a) noexcept : a_(a.a_) {}

    /**
     * @brief Allocates memory for an object of type T.
     *
     * @param n The number of objects to allocate.
     * @return A pointer to the allocated memory.
     */
    T* allocate(std::size_t n) {
        return static_cast<T*>(a_.allocate(n * sizeof(T)));
    }

    /**
     * @brief Deallocates memory previously allocated with allocate().
     *
     * @param p A pointer to the memory to deallocate.
     * @param n The number of objects that were deallocated.
     */
    void deallocate(T* p, std::size_t n) noexcept {
        a_.deallocate(p, n * sizeof(T));
    }

    /**
     * @brief Defines an alternative allocator for a different type.
     *
     * @tparam U The type of objects to allocate with the rebinded allocator.
     */
    template <class U>
    struct rebind {
        using other = short_alloc<U, N, Align>;
    };

    // Friendship declarations
    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M,
              std::size_t A2>
    friend bool operator==(const short_alloc<T1, N1, A1>& x,
                           const short_alloc<U, M, A2>& y) noexcept;

    template <class U, std::size_t M, std::size_t A2>
    friend class short_alloc;
};

/**
 * @brief Checks if two short_alloc objects are equal.
 *
 * Two short_alloc objects are considered equal if they have the same size,
 * alignment, and are using the same underlying arena.
 *
 * @tparam T The type of objects allocated by the first allocator.
 * @tparam N The size of the fixed-size memory arena for the first allocator.
 * @tparam A1 The alignment requirement for memory allocations by the first
 * allocator.
 * @tparam U The type of objects allocated by the second allocator.
 * @tparam M The size of the fixed-size memory arena for the second allocator.
 * @tparam A2 The alignment requirement for memory allocations by the second
 * allocator.
 * @param x The first short_alloc object.
 * @param y The second short_alloc object.
 * @return true if the two short_alloc objects are equal, false otherwise.
 */
template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline bool operator==(const short_alloc<T, N, A1>& x,
                       const short_alloc<U, M, A2>& y) noexcept {
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

/**
 * @brief Checks if two short_alloc objects are not equal.
 *
 * Two short_alloc objects are considered not equal if they have different
 * sizes, alignments, or are using different underlying arenas.
 *
 * @tparam T The type of objects allocated by the first allocator.
 * @tparam N The size of the fixed-size memory arena for the first allocator.
 * @tparam A1 The alignment requirement for memory allocations by the first
 * allocator.
 * @tparam U The type of objects allocated by the second allocator.
 * @tparam M The size of the fixed-size memory arena for the second allocator.
 * @tparam A2 The alignment requirement for memory allocations by the second
 * allocator.
 * @param x The first short_alloc object.
 * @param y The second short_alloc object.
 * @return true if the two short_alloc objects are not equal, false otherwise.
 */
template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline bool operator!=(const short_alloc<T, N, A1>& x,
                       const short_alloc<U, M, A2>& y) noexcept {
    return !(x == y);
}

#endif  // ATOM_MEMORY_SHORT_ALLOC_HPP
