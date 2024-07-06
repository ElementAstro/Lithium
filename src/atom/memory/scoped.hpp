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

#include <memory>
#include <type_traits>
#include <utility>

/**
 * @brief A smart pointer that owns and manages another object through a pointer
 * and disposes of that object when the ScopedPtr goes out of scope.
 *
 * @tparam T The type of the managed object.
 * @tparam D The type of the deleter used to destroy the managed object.
 * Defaults to std::default_delete<T>.
 */
template <typename T, typename D = std::default_delete<T>>
class ScopedPtr {
public:
    /**
     * @brief Construct a new ScopedPtr object.
     *
     * @param ptr Pointer to the object to manage. Defaults to nullptr.
     */
    explicit ScopedPtr(T* ptr = nullptr) noexcept : ptr_(ptr) {}

    /**
     * @brief Construct a new ScopedPtr object with a custom deleter.
     *
     * @param ptr Pointer to the object to manage.
     * @param d Custom deleter to use.
     */
    ScopedPtr(T* ptr, D d) noexcept : ptr_(ptr), deleter_(std::move(d)) {}

    // Deleting copy constructor and copy assignment operator
    ScopedPtr(const ScopedPtr&) = delete;
    auto operator=(const ScopedPtr&) -> ScopedPtr& = delete;

    /**
     * @brief Move constructor.
     *
     * @param other ScopedPtr to move from.
     */
    ScopedPtr(ScopedPtr&& other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)),
          deleter_(std::move(other.deleter_)) {}

    /**
     * @brief Move assignment operator.
     *
     * @param other ScopedPtr to move from.
     * @return ScopedPtr& Reference to this object.
     */
    auto operator=(ScopedPtr&& other) noexcept -> ScopedPtr& {
        if (this != &other) {
            reset(std::exchange(other.ptr_, nullptr));
            deleter_ = std::move(other.deleter_);
        }
        return *this;
    }

    /**
     * @brief Destroy the ScopedPtr object and delete the managed object.
     */
    ~ScopedPtr() { reset(); }

    /**
     * @brief Reset the managed object.
     *
     * @param ptr New pointer to manage. Defaults to nullptr.
     */
    void reset(T* ptr = nullptr) noexcept {
        if (ptr_ != ptr) {
            if (ptr_) {
                deleter_(std::exchange(ptr_, ptr));
            } else {
                ptr_ = ptr;
            }
        }
    }

    /**
     * @brief Release ownership of the managed object.
     *
     * @return T* Pointer to the previously managed object.
     */
    [[nodiscard]] auto release() noexcept -> T* {
        return std::exchange(ptr_, nullptr);
    }

    /**
     * @brief Get the managed object.
     *
     * @return T* Pointer to the managed object.
     */
    [[nodiscard]] auto get() const noexcept -> T* { return ptr_; }

    /**
     * @brief Dereference operator.
     *
     * @return T& Reference to the managed object.
     */
    [[nodiscard]] auto operator*() const noexcept -> T& { return *ptr_; }

    /**
     * @brief Arrow operator.
     *
     * @return T* Pointer to the managed object.
     */
    [[nodiscard]] auto operator->() const noexcept -> T* { return ptr_; }

    /**
     * @brief Check if the ScopedPtr is managing an object.
     *
     * @return true If the ScopedPtr is managing an object.
     * @return false If the ScopedPtr is not managing an object.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    /**
     * @brief Swap the contents of two ScopedPtr objects.
     *
     * @param other ScopedPtr to swap with.
     */
    void swap(ScopedPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(deleter_, other.deleter_);
    }

    /**
     * @brief Create a new ScopedPtr from the given pointer.
     *
     * @param ptr Pointer to manage.
     * @return ScopedPtr<T, D> New ScopedPtr object.
     */
    [[nodiscard]] static auto makeScoped(T* ptr) -> ScopedPtr {
        return ScopedPtr(ptr);
    }

    /**
     * @brief Get the deleter object.
     *
     * @return const D& Reference to the deleter object.
     */
    [[nodiscard]] auto getDeleter() const noexcept -> const D& {
        return deleter_;
    }

    /**
     * @brief Get the deleter object.
     *
     * @return D& Reference to the deleter object.
     */
    [[nodiscard]] auto getDeleter() noexcept -> D& { return deleter_; }

private:
    T* ptr_ = nullptr;
    [[no_unique_address]] D deleter_ = D();
};

/**
 * @brief Non-member swap function.
 *
 * @tparam T Type of the managed object.
 * @tparam D Type of the deleter.
 * @param lhs First ScopedPtr to swap.
 * @param rhs Second ScopedPtr to swap.
 */
template <typename T, typename D>
void swap(ScopedPtr<T, D>& lhs, ScopedPtr<T, D>& rhs) noexcept {
    lhs.swap(rhs);
}

/**
 * @brief Create a ScopedPtr that manages a new object.
 *
 * @tparam T Type of the object to create.
 * @tparam Args Types of the arguments to forward to the constructor of T.
 * @param args Arguments to forward to the constructor of T.
 * @return ScopedPtr<T> New ScopedPtr managing the created object.
 */
template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
[[nodiscard]] auto makeScoped(Args&&... args) -> ScopedPtr<T> {
    return ScopedPtr<T>(new T(std::forward<Args>(args)...));
}

#endif  // ATOM_MEMORY_SCOPED_HPP