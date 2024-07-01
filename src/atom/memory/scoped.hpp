/*
 * scoped.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-5

Description: A simple implementation of scoped pointer

**************************************************/

#ifndef ATOM_MEMORY_SCOPED_HPP
#define ATOM_MEMORY_SCOPED_HPP

#include <memory>   // for std::default_delete
#include <utility>  // for std::exchange

#include "macro.hpp"

template <typename T, typename D = std::default_delete<T>>
class ScopedPtr {
public:
    // Constructors
    explicit ScopedPtr(T* ptr = nullptr) ATOM_NOEXCEPT : ptr_(ptr) {}

    // Deleting copy constructor and copy assignment operator
    ScopedPtr(const ScopedPtr&) = delete;
    auto operator=(const ScopedPtr&) -> ScopedPtr& = delete;

    // Move constructor
    ScopedPtr(ScopedPtr&& other) ATOM_NOEXCEPT
        : ptr_(std::exchange(other.ptr_, nullptr)) {}

    // Move assignment operator
    auto operator=(ScopedPtr&& other) ATOM_NOEXCEPT->ScopedPtr& {
        if (this != &other) {
            reset(std::exchange(other.ptr_, nullptr));
        }
        return *this;
    }

    // Destructor
    ~ScopedPtr() { reset(); }

    // Reset the managed object
    void reset(T* ptr = nullptr) ATOM_NOEXCEPT {
        if (ptr_ != ptr) {
            D()(std::exchange(ptr_, ptr));
        }
    }

    // Release ownership of the managed object
    [[nodiscard]] auto release() ATOM_NOEXCEPT -> T* {
        return std::exchange(ptr_, nullptr);
    }

    // Get the managed object
    [[nodiscard]] auto get() const ATOM_NOEXCEPT -> T* { return ptr_; }

    // Dereference operators
    [[nodiscard]] auto operator*() const ATOM_NOEXCEPT->T& { return *ptr_; }
    [[nodiscard]] auto operator->() const ATOM_NOEXCEPT->T* { return ptr_; }

    // Conversion to bool for checking if non-null
    [[nodiscard]] explicit operator bool() const ATOM_NOEXCEPT {
        return ptr_ != nullptr;
    }

    // Swap two scoped_ptr objects
    void swap(ScopedPtr& other) ATOM_NOEXCEPT { std::swap(ptr_, other.ptr_); }

    // Make a new scoped_ptr from the given pointer
    [[nodiscard]] static auto makeScoped(T* ptr) -> ScopedPtr {
        return ScopedPtr(ptr);
    }

private:
    T* ptr_ = nullptr;
};

// Non-member swap function
template <typename T, typename D>
void swap(ScopedPtr<T, D>& lhs, ScopedPtr<T, D>& rhs) ATOM_NOEXCEPT {
    lhs.swap(rhs);
}

#endif
