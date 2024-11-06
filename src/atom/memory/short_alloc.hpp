#ifndef ATOM_MEMORY_SHORT_ALLOC_HPP
#define ATOM_MEMORY_SHORT_ALLOC_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>

#include "atom/macro.hpp"

namespace atom::memory {

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
    alignas(alignment) std::array<char, N> buf_{};
    char* ptr_;
    mutable std::mutex mutex_;

public:
    Arena() ATOM_NOEXCEPT : ptr_(buf_.data()) {}
    ~Arena() {
        std::lock_guard<std::mutex> lock(mutex_);
        ptr_ = nullptr;
    }

    Arena(const Arena&) = delete;
    auto operator=(const Arena&) -> Arena& = delete;

    /**
     * @brief Allocate memory from the arena.
     *
     * @param n The number of bytes to allocate.
     * @return void* Pointer to the allocated memory.
     * @throw std::bad_alloc If there is not enough memory to fulfill the
     * request.
     */
    auto allocate(std::size_t n) -> void* {
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t space = N - used();
        void* result = ptr_;
        void* alignedPtr = std::align(alignment, n, result, space);
        if (alignedPtr == nullptr) {
            throw std::bad_alloc();
        }
        ptr_ = static_cast<char*>(alignedPtr) + n;
        return alignedPtr;
    }

    /**
     * @brief Deallocate memory back to the arena.
     *
     * Note: This method only supports deallocating the most recently allocated
     * block.
     *
     * @param p Pointer to the memory to deallocate.
     * @param n The number of bytes to deallocate.
     */
    void deallocate(void* p, std::size_t n) ATOM_NOEXCEPT {
        std::lock_guard<std::mutex> lock(mutex_);
        if (static_cast<char*>(p) + n == ptr_) {
            ptr_ = static_cast<char*>(p);
        }
    }

    /**
     * @brief Get the total size of the arena.
     *
     * @return constexpr std::size_t The size of the arena.
     */
    static ATOM_CONSTEXPR auto size() ATOM_NOEXCEPT -> std::size_t { return N; }

    /**
     * @brief Get the amount of used memory in the arena.
     *
     * @return std::size_t The number of bytes used.
     */
    ATOM_NODISCARD auto used() const ATOM_NOEXCEPT -> std::size_t {
        return static_cast<std::size_t>(ptr_ - buf_.data());
    }

    /**
     * @brief Get the remaining memory in the arena.
     *
     * @return std::size_t The number of bytes remaining.
     */
    ATOM_NODISCARD auto remaining() const ATOM_NOEXCEPT -> std::size_t {
        return N - used();
    }

    /**
     * @brief Reset the arena to its initial state.
     */
    void reset() ATOM_NOEXCEPT {
        std::lock_guard<std::mutex> lock(mutex_);
        ptr_ = buf_.data();
    }

private:
    auto pointerInBuffer(char* p) ATOM_NOEXCEPT -> bool {
        return buf_.data() <= p && p <= buf_.data() + N;
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
    static ATOM_CONSTEXPR auto ALIGNMENT = Align;
    static ATOM_CONSTEXPR auto SIZE = N;
    using arena_type = Arena<N, Align>;

private:
    arena_type& a_;

public:
    explicit ShortAlloc(arena_type& a) ATOM_NOEXCEPT : a_(a) {}

    template <class U>
    explicit ShortAlloc(const ShortAlloc<U, N, ALIGNMENT>& a) ATOM_NOEXCEPT
        : a_(a.a_) {}

    auto allocate(std::size_t n) -> T* {
        if (n > SIZE / sizeof(T)) {
            throw std::bad_alloc();
        }
        void* ptr = a_.allocate(n * sizeof(T));
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t n) ATOM_NOEXCEPT {
        a_.deallocate(p, n * sizeof(T));
    }

    template <class U, class... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <class U>
    void destroy(U* p) {
        p->~U();
    }

    template <class U>
    struct Rebind {
        using other = ShortAlloc<U, N, Align>;
    };

    template <class T1, std::size_t N1, std::size_t A1, class U, std::size_t M,
              std::size_t A2>
    friend auto operator==(const ShortAlloc<T1, N1, A1>& x,
                           const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool;

    template <class U, std::size_t M, std::size_t A2>
    friend class ShortAlloc;
};

template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline auto operator==(const ShortAlloc<T, N, A1>& x,
                       const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool {
    return N == M && A1 == A2 && &x.a_ == &y.a_;
}

template <class T, std::size_t N, std::size_t A1, class U, std::size_t M,
          std::size_t A2>
inline auto operator!=(const ShortAlloc<T, N, A1>& x,
                       const ShortAlloc<U, M, A2>& y) ATOM_NOEXCEPT->bool {
    return !(x == y);
}

/**
 * @brief Allocate a unique_ptr with a custom deleter using a specific
 * allocator.
 *
 * @tparam Alloc The allocator type.
 * @tparam T The type of object to allocate.
 * @tparam Args The types of arguments to forward to the constructor.
 * @param alloc The allocator instance.
 * @param args The arguments to pass to the constructor of T.
 * @return std::unique_ptr<T, std::function<void(T*)>> The allocated unique_ptr
 * with a custom deleter.
 */
template <typename Alloc, typename T, typename... Args>
auto allocateUnique(Alloc& alloc, Args&&... args)
    -> std::unique_ptr<T, std::function<void(T*)>> {
    using AllocTraits = std::allocator_traits<Alloc>;

    T* p = AllocTraits::allocate(alloc, 1);
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

}  // namespace atom::memory

#endif  // ATOM_MEMORY_SHORT_ALLOC_HPP
