/*
 * short_alloc.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Short Alloc from Howard Hinnant

**************************************************/

#ifndef ATOM_EXPERIMENT_SHORT_ALLOC_HPP
#define ATOM_EXPERIMENT_SHORT_ALLOC_HPP

#include <cassert>
#include <cstddef>

/**
 * @brief A fixed-size memory arena for allocating memory with aligned
 * addresses.
 *
 * @tparam N The size of the arena buffer.
 * @tparam alignment The alignment requirement for memory allocation.
 */
template <std::size_t N, std::size_t alignment = alignof(std::max_align_t)>
class arena {
    alignas(alignment) char buf_[N]; /**< The buffer for memory allocation. */
    char *ptr_; /**< Pointer to the next available memory location. */

public:
    ~arena() { ptr_ = nullptr; }     /**< Destructor. */
    arena() noexcept : ptr_(buf_) {} /**< Default constructor. */
    arena(const arena &) = delete;   /**< Copy constructor (deleted). */
    arena &operator=(const arena &) =
        delete; /**< Copy assignment operator (deleted). */

    /**
     * @brief Allocates memory from the arena with the specified alignment.
     *
     * @tparam ReqAlign The required alignment for the allocated memory.
     * @param n The number of bytes to allocate.
     * @return Pointer to the allocated memory.
     */
    template <std::size_t ReqAlign>
    char *allocate(std::size_t n);

    /**
     * @brief Deallocates memory previously allocated from the arena.
     *
     * @param p Pointer to the memory block to deallocate.
     * @param n The number of bytes to deallocate.
     */
    void deallocate(char *p, std::size_t n) noexcept;

    /**
     * @brief Gets the total size of the arena buffer.
     *
     * @return The size of the arena buffer.
     */
    static constexpr std::size_t size() noexcept { return N; }

    /**
     * @brief Gets the amount of memory used in the arena.
     *
     * @return The number of bytes used in the arena.
     */
    std::size_t used() const noexcept {
        return static_cast<std::size_t>(ptr_ - buf_);
    }

    /**
     * @brief Resets the arena, making all allocated memory available for reuse.
     */
    void reset() noexcept { ptr_ = buf_; }

private:
    /**
     * @brief Rounds up the size to the nearest multiple of the alignment.
     *
     * @param n The size to round up.
     * @return The rounded-up size.
     */
    static constexpr std::size_t align_up(std::size_t n) noexcept {
        return (n + (alignment - 1)) & ~(alignment - 1);
    }

    /**
     * @brief Checks if a pointer is within the arena buffer.
     *
     * @param p Pointer to check.
     * @return True if the pointer is within the arena buffer, otherwise false.
     */
    bool pointer_in_buffer(char *p) noexcept {
        return buf_ <= p && p <= buf_ + N;
    }
};

/**
 * @brief A memory allocator adapter for using the arena as a memory pool.
 *
 * @tparam T The type of objects to allocate.
 * @tparam N The size of the arena buffer.
 * @tparam Align The alignment requirement for memory allocation.
 */
template <class T, std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class short_alloc {
public:
    using value_type = T; /**< The type of objects to allocate. */
    static auto constexpr alignment =
        Align; /**< The alignment requirement for memory allocation. */
    static auto constexpr size = N; /**< The size of the arena buffer. */
    using arena_type =
        arena<size, alignment>; /**< The type of the underlying arena. */

private:
    arena_type &a_; /**< Reference to the underlying arena. */

public:
    short_alloc(const short_alloc &) =
        default; /**< Copy constructor (defaulted). */
    short_alloc &operator=(const short_alloc &) =
        delete; /**< Copy assignment operator (deleted). */

    /**
     * @brief Constructs the short_alloc with the specified arena.
     *
     * @param a Reference to the arena to use for allocation.
     */
    explicit short_alloc(arena_type &a) noexcept : a_(a) {
        static_assert(size % alignment == 0,
                      "size N needs to be a multiple of alignment Align");
    }

    /**
     * @brief Constructs the short_alloc with the specified arena of a different
     * type.
     *
     * @tparam U The type of objects allocated by the other short_alloc.
     * @param a Reference to the other short_alloc's arena.
     */
    template <class U>
    explicit short_alloc(const short_alloc<U, N, alignment> &a) noexcept
        : a_(a.a_) {}

    /**
     * @brief Allocates memory for an object of type T.
     *
     * @param n Number of objects to allocate memory for.
     * @return Pointer to the allocated memory.
     */
    T *allocate(std::size_t n) {
        return reinterpret_cast<T *>(
            a_.template allocate<alignof(T)>(n * sizeof(T)));
    }

    /**
     * @brief Deallocates memory previously allocated for objects of type T.
     *
     * @param p Pointer to the memory block to deallocate.
     * @param n Number of objects to deallocate memory for.
     */
    void deallocate(T *p, std::size_t n) noexcept {
        a_.deallocate(reinterpret_cast<char *>(p), n * sizeof(T));
    }

    /**
     * @brief Nested type alias for rebind.
     *
     * @tparam _Up The new type to rebind to.
     */
    template <class _Up>
    struct rebind {
        using other = short_alloc<_Up, N, alignment>;
    };

    /**
     * @brief Equality comparison operator.
     *
     * @tparam T1 The type of objects allocated by the left short_alloc.
     * @tparam N1 The size of the arena buffer for the left short_alloc.
     * @tparam A1 The alignment requirement for the left short_alloc.
     * @tparam U The type of objects allocated by the right short_alloc.
     * @tparam M The size of the arena buffer for the right short_alloc.
     * @tparam A2 The alignment requirement for the right short_alloc.
     * @param x The left short_alloc.
     * @param y The right short_alloc.
     * @return True if the short_allocs are equal, otherwise false.
     */
    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M,
              std::size_t A2>
    friend bool operator==(const short_alloc<T1, N1, A1> &x,
                           const short_alloc<U, M, A2> &y) noexcept;

    /**
     * @brief Friendship declaration for the rebind struct.
     *
     * @tparam U The type of objects allocated by the other short_alloc.
     * @tparam M The size of the arena buffer for the other short_alloc.
     * @tparam A2 The alignment requirement for the other short_alloc.
     */
    template <class U, std::size_t M, std::size_t A2>
    friend class short_alloc;
};

/**
 * @brief Equality comparison operator for short_alloc.
 *
 * @tparam T The type of objects allocated by the left short_alloc.
 * @tparam N The size of the arena buffer for the left short_alloc.
 * @tparam A1 The alignment requirement for the left short_alloc.
 * @tparam U The type of objects allocated by the right short_alloc.
 * @tparam M The size of the arena buffer for the right short_alloc.
 * @tparam A2 The alignment requirement for the right short_alloc.
 * @param x The left short_alloc.
 * @param y The right short_alloc.
 * @return True if the short_allocs are equal, otherwise false.
 */
template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline bool operator==(const short_alloc<T, N, A1> &x,
                       const short_alloc<U, M, A2> &y) noexcept {
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

/**
 * @brief Inequality comparison operator for short_alloc.
 *
 * @tparam T The type of objects allocated by the left short_alloc.
 * @tparam N The size of the arena buffer for the left short_alloc.
 * @tparam A1 The alignment requirement for the left short_alloc.
 * @tparam U The type of objects allocated by the right short_alloc.
 * @tparam M The size of the arena buffer for the right short_alloc.
 * @tparam A2 The alignment requirement for the right short_alloc.
 * @param x The left short_alloc.
 * @param y The right short_alloc.
 * @return True if the short_allocs are not equal, otherwise false.
 */
template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline bool operator!=(const short_alloc<T, N, A1> &x,
                       const short_alloc<U, M, A2> &y) noexcept {
    return !(x == y);
}

#endif  // ATOM_EXPERIMENT_SHORT_ALLOC_HPP
