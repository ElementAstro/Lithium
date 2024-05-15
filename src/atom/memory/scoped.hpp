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

#include <memory>  // for std::default_delete
#include <type_traits>
#include <utility>  // for std::exchange

template <typename T, typename D = std::default_delete<T>>
class scoped_ptr {
public:
    // Constructors
    explicit scoped_ptr(T* ptr = nullptr) noexcept : ptr_(ptr) {}

    // Deleting copy constructor and copy assignment operator
    scoped_ptr(const scoped_ptr&) = delete;
    scoped_ptr& operator=(const scoped_ptr&) = delete;

    // Move constructor
    scoped_ptr(scoped_ptr&& other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)) {}

    // Move assignment operator
    scoped_ptr& operator=(scoped_ptr&& other) noexcept {
        if (this != &other) {
            reset(std::exchange(other.ptr_, nullptr));
        }
        return *this;
    }

    // Destructor
    ~scoped_ptr() { reset(); }

    // Reset the managed object
    void reset(T* ptr = nullptr) noexcept {
        if (ptr_ != ptr) {
            D()(std::exchange(ptr_, ptr));
        }
    }

    // Release ownership of the managed object
    [[nodiscard]] T* release() noexcept { return std::exchange(ptr_, nullptr); }

    // Get the managed object
    [[nodiscard]] T* get() const noexcept { return ptr_; }

    // Dereference operators
    [[nodiscard]] T& operator*() const noexcept { return *ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }

    // Conversion to bool for checking if non-null
    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    // Swap two scoped_ptr objects
    void swap(scoped_ptr& other) noexcept { std::swap(ptr_, other.ptr_); }

    // Make a new scoped_ptr from the given pointer
    [[nodiscard]] static scoped_ptr make_scoped(T* ptr) {
        return scoped_ptr(ptr);
    }

private:
    T* ptr_ = nullptr;
};

// Non-member swap function
template <typename T, typename D>
void swap(scoped_ptr<T, D>& lhs, scoped_ptr<T, D>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif