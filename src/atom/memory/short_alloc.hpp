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
#include <functional>
#include <memory>
#include <type_traits>
#include "macro.hpp"

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
class Arena {
    alignas(alignment) char buf_[N]{};
    char* ptr_;

public:
    ~Arena() { ptr_ = nullptr; }
    Arena() ATOM_NOEXCEPT : ptr_(buf_) {}
    Arena(const Arena&) = delete;
    auto operator=(const Arena&) -> Arena& = delete;

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
    auto allocate(std::size_t n) -> void* {
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
    void deallocate(void* p, std::size_t n) ATOM_NOEXCEPT {
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
    static constexpr auto size() ATOM_NOEXCEPT -> std::size_t { return N; }

    /**
     * @brief Returns the amount of memory used in the arena.
     *
     * @return The amount of memory used in the arena in bytes.
     */
    auto used() const ATOM_NOEXCEPT -> std::size_t {
        return static_cast<std::size_t>(ptr_ - buf_);
    }

    /**
     * @brief Resets the arena to its initial state, deallocating all memory.
     */
    void reset() ATOM_NOEXCEPT { ptr_ = buf_; }

private:
    /**
     * @brief Checks if a pointer is within the buffer of the arena.
     *
     * @param p The pointer to check.
     * @return true if the pointer is within the arena's buffer, false
     * otherwise.
     */
    auto pointerInBuffer(char* p) ATOM_NOEXCEPT -> bool {
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
class ShortAlloc {
public:
    using value_type = T;
    static constexpr auto ALIGNMENT = Align;
    static constexpr auto SIZE = N;
    using arena_type = Arena<N, Align>;

private:
    arena_type& a_;

public:
    /**
     * @brief Constructs a short_alloc object using the specified arena.
     *
     * @param a The arena to use for allocations.
     */
    explicit ShortAlloc(arena_type& a) ATOM_NOEXCEPT : a_(a) {}

    /**
     * @brief Constructs a short_alloc object by copying another short_alloc
     * object.
     *
     * @tparam U The type of objects allocated by the other allocator.
     * @param a The other short_alloc object to copy from.
     */
    template <class U>
    explicit ShortAlloc(const ShortAlloc<U, N, ALIGNMENT>& a) ATOM_NOEXCEPT
        : a_(a.a_) {}

    /**
     * @brief Allocates memory for an object of type T.
     *
     * @param n The number of objects to allocate.
     * @return A pointer to the allocated memory.
     */
    auto allocate(std::size_t n) -> T* {
        if (n > SIZE / sizeof(T)) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(a_.allocate(n * sizeof(T)));
    }

    /**
     * @brief Deallocates memory previously allocated with allocate().
     *
     * @param p A pointer to the memory to deallocate.
     * @param n The number of objects that were deallocated.
     */
    void deallocate(T* p, std::size_t n) ATOM_NOEXCEPT {
        a_.deallocate(p, n * sizeof(T));
    }

    /**
     * @brief Constructs an object of type T in allocated memory.
     *
     * @tparam U The type of the object to construct.
     * @param p A pointer to the allocated memory.
     * @param args The arguments to pass to the constructor of T.
     */
    template <class U, class... Args>
    void construct(U* p, Args&&... args) {
        new (p) U(std::forward<Args>(args)...);
    }

    /**
     * @brief Destroys an object of type T in allocated memory.
     *
     * @tparam U The type of the object to destroy.
     * @param p A pointer to the allocated memory.
     */
    template <class U>
    void destroy(U* p) {
        p->~U();
    }

    /**
     * @brief Defines an alternative allocator for a different type.
     *
     * @tparam U The type of objects to allocate with the rebinded allocator.
     */
    template <class U>
    struct Rebind {
        using other = ShortAlloc<U, N, Align>;
    };

    // Friendship declarations
    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M,
              std::size_t A2>
    friend auto operator==(const ShortAlloc<T1, N1, A1>& x,
                           const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool;

    template <class U, std::size_t M, std::size_t A2>
    friend class ShortAlloc;
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
inline auto operator==(const ShortAlloc<T, N, A1>& x,
                       const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool {
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

/**
 * @brief Checks if two short_alloc objects are not equal.
 *
 * Two short_alloc objects are considered not equal if they have
 * different sizes, alignments, or are using different underlying arenas.
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
inline auto operator!=(const ShortAlloc<T, N, A1>& x,
                       const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool {
    return !(x == y);
}

/**
 * @brief A utility function to create a new object using the allocator.
 *
 * @tparam Alloc The allocator type.
 * @tparam T The type of the object to create.
 * @tparam Args The types of the arguments to pass to the constructor.
 * @param alloc The allocator to use.
 * @param args The arguments to pass to the constructor.
 * @return A unique_ptr to the created object.
 */
template <typename Alloc, typename T, typename... Args>
auto allocate_unique(Alloc& alloc, Args&&... args)
    -> std::unique_ptr<T, std::function<void(T*)>> {
    using AllocTraits = std::allocator_traits<Alloc>;

    auto p = AllocTraits::allocate(alloc, 1);
    try {
        AllocTraits::construct(alloc, p, std::forward<Args>(args)...);
    } catch (...) {
        AllocTraits::deallocate(alloc, p, 1);
        throw;
    }

    auto deleter = [&alloc](T* ptr) {
        AllocTraits::destroy(alloc, ptr);
        AllocTraits::deallocate(alloc, ptr, 1);
    };

    return std::unique_ptr<T, std::function<void(T*)>>(p, deleter);
}

#endif  // ATOM_MEMORY_SHORT_ALLOC_HPP