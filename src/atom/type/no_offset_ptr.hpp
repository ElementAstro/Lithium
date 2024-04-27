/*
 * no_offset_ptr.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: No Offset Pointer

**************************************************/

#ifndef ATOM_TYPE_NoOffsetPtr_HPP
#define ATOM_TYPE_NoOffsetPtr_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

template <typename T>
class UnshiftedPtr {
public:
    UnshiftedPtr() { new (&storage) T; }

    UnshiftedPtr(const T& value) { new (&storage) T(value); }

    UnshiftedPtr(const UnshiftedPtr& other) { new (&storage) T(get()); }

    UnshiftedPtr(UnshiftedPtr&& other) { new (&storage) T(std::move(get())); }

    ~UnshiftedPtr() { get().~T(); }

    UnshiftedPtr& operator=(const UnshiftedPtr& other) {
        if (this != &other) {
            get() = other.get();
        }
        return *this;
    }

    UnshiftedPtr& operator=(UnshiftedPtr&& other) {
        if (this != &other) {
            get() = std::move(other.get());
        }
        return *this;
    }

    T* operator->() { return &get(); }

    const T* operator->() const { return &get(); }

    T& operator*() { return get(); }

    const T& operator*() const { return get(); }

private:
    T& get() { return reinterpret_cast<T&>(storage); }

    const T& get() const { return reinterpret_cast<const T&>(storage); }

    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
};

template <typename T>
class NoOffsetPtr {
public:
    constexpr NoOffsetPtr() noexcept = default;
    constexpr NoOffsetPtr(std::nullptr_t) noexcept {}

    constexpr explicit NoOffsetPtr(T* ptr) noexcept : ptr_(ptr) {}

    constexpr NoOffsetPtr(const NoOffsetPtr&) noexcept = default;
    constexpr NoOffsetPtr& operator=(const NoOffsetPtr&) noexcept = default;

    constexpr NoOffsetPtr(NoOffsetPtr&&) noexcept = default;
    constexpr NoOffsetPtr& operator=(NoOffsetPtr&&) noexcept = default;

    constexpr T& operator*() const noexcept { return *ptr_; }
    constexpr T* operator->() const noexcept { return ptr_; }

    constexpr explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    constexpr T* get() const noexcept { return ptr_; }

    constexpr void reset(T* ptr = nullptr) noexcept { ptr_ = ptr; }

    constexpr void swap(NoOffsetPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

    NoOffsetPtr& operator++() = delete;
    NoOffsetPtr& operator--() = delete;
    NoOffsetPtr operator++(int) = delete;
    NoOffsetPtr operator--(int) = delete;
    NoOffsetPtr& operator+=(std::ptrdiff_t) = delete;
    NoOffsetPtr& operator-=(std::ptrdiff_t) = delete;
    NoOffsetPtr operator+(std::ptrdiff_t) const = delete;
    NoOffsetPtr operator-(std::ptrdiff_t) const = delete;
    std::ptrdiff_t operator-(const NoOffsetPtr&) const = delete;

private:
    T* ptr_ = nullptr;
};

template <typename T, typename U>
constexpr bool operator==(const NoOffsetPtr<T>& lhs,
                          const NoOffsetPtr<U>& rhs) noexcept {
    return lhs.get() == rhs.get();
}

template <typename T>
constexpr bool operator==(const NoOffsetPtr<T>& lhs, std::nullptr_t) noexcept {
    return lhs.get() == nullptr;
}

template <typename T>
constexpr bool operator==(std::nullptr_t, const NoOffsetPtr<T>& rhs) noexcept {
    return nullptr == rhs.get();
}

template <typename T, typename U>
constexpr bool operator!=(const NoOffsetPtr<T>& lhs,
                          const NoOffsetPtr<U>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename T>
constexpr bool operator!=(const NoOffsetPtr<T>& lhs, std::nullptr_t) noexcept {
    return !(lhs == nullptr);
}

template <typename T>
constexpr bool operator!=(std::nullptr_t, const NoOffsetPtr<T>& rhs) noexcept {
    return !(nullptr == rhs);
}

template <typename T>
constexpr void swap(NoOffsetPtr<T>& lhs, NoOffsetPtr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif
