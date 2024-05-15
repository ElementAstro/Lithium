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
    scoped_ptr(scoped_ptr&& other) noexcept : ptr_(other.release()) {}

    // Move assignment operator
    scoped_ptr& operator=(scoped_ptr&& other) noexcept {
        reset(other.release());
        return *this;
    }

    // Destructor
    ~scoped_ptr() { reset(); }

    // Reset the managed object
    void reset(T* ptr = nullptr) noexcept {
        if (ptr_ != ptr) {
            D()(ptr_);
            ptr_ = ptr;
        }
    }

    // Release ownership of the managed object
    T* release() noexcept { return std::exchange(ptr_, nullptr); }

    // Get the managed object
    T* get() const noexcept { return ptr_; }

    // Dereference operators
    T& operator*() const noexcept { return *ptr_; }

    T* operator->() const noexcept { return ptr_; }

    // Conversion to bool for checking if non-null
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    T* ptr_ = nullptr;
};

#endif