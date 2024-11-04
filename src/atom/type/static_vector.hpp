/*
 * static_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: A static vector (Optimized with C++20 features)

**************************************************/

#ifndef ATOM_TYPE_STATIC_VECTOR_HPP
#define ATOM_TYPE_STATIC_VECTOR_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "atom/macro.hpp"
#include "error/exception.hpp"

/**
 * @brief A static vector implementation with a fixed capacity.
 *
 * @tparam T The type of elements stored in the vector.
 * @tparam Capacity The maximum number of elements the vector can hold.
 */
template <typename T, std::size_t Capacity>
    requires(Capacity > 0)  // Ensure positive capacity
class StaticVector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /**
     * @brief Default constructor. Constructs an empty StaticVector.
     */
    constexpr StaticVector() noexcept = default;

    /**
     * @brief Constructs a StaticVector from an initializer list.
     *
     * @param init The initializer list to initialize the StaticVector with.
     */
    constexpr StaticVector(std::initializer_list<T> init) {
        if (init.size() > Capacity) {
            throw std::length_error("Initializer list size exceeds capacity");
        }
        std::ranges::copy(init, begin());
        m_size_ = init.size();
    }

    /**
     * @brief Copy constructor. Constructs a StaticVector by copying another
     * StaticVector.
     *
     * @param other The StaticVector to copy from.
     */
    constexpr StaticVector(const StaticVector& other) noexcept {
        std::ranges::copy(other, begin());
        m_size_ = other.m_size_;
    }

    /**
     * @brief Move constructor. Constructs a StaticVector by moving another
     * StaticVector.
     *
     * @param other The StaticVector to move from.
     */
    constexpr StaticVector(StaticVector&& other) noexcept {
        std::ranges::move(other, begin());
        m_size_ = other.m_size_;
        other.m_size_ = 0;
    }

    /**
     * @brief Copy assignment operator. Copies the contents of another
     * StaticVector.
     *
     * @param other The StaticVector to copy from.
     * @return A reference to the assigned StaticVector.
     */
    constexpr auto operator=(const StaticVector& other) noexcept
        -> StaticVector& {
        if (this != &other) {
            std::ranges::copy(other, begin());
            m_size_ = other.m_size_;
        }
        return *this;
    }

    /**
     * @brief Move assignment operator. Moves the contents of another
     * StaticVector.
     *
     * @param other The StaticVector to move from.
     * @return A reference to the assigned StaticVector.
     */
    constexpr auto operator=(StaticVector&& other) noexcept -> StaticVector& {
        if (this != &other) {
            std::ranges::move(other, begin());
            m_size_ = other.m_size_;
            other.m_size_ = 0;
        }
        return *this;
    }

    /**
     * @brief Adds an element to the end of the StaticVector by copying.
     *
     * @param value The value to add.
     */
    constexpr void pushBack(const T& value) {
        if (m_size_ >= Capacity) {
            throw std::overflow_error("StaticVector capacity exceeded");
        }
        m_data_[m_size_++] = value;
    }

    /**
     * @brief Adds an element to the end of the StaticVector by moving.
     *
     * @param value The value to add.
     */
    constexpr void pushBack(T&& value) {
        if (m_size_ >= Capacity) {
            throw std::overflow_error("StaticVector capacity exceeded");
        }
        m_data_[m_size_++] = std::move(value);
    }

    /**
     * @brief Constructs an element in place at the end of the StaticVector.
     *
     * @tparam Args The types of the arguments to construct the element with.
     * @param args The arguments to construct the element with.
     * @return A reference to the constructed element.
     */
    template <typename... Args>
    constexpr auto emplaceBack(Args&&... args) -> reference {
        if (m_size_ >= Capacity) {
            throw std::overflow_error("StaticVector capacity exceeded");
        }
        return m_data_[m_size_++] = T(std::forward<Args>(args)...);
    }

    /**
     * @brief Removes the last element from the StaticVector.
     */
    constexpr void popBack() {
        if (m_size_ == 0) {
            throw std::underflow_error("StaticVector is empty");
        }
        --m_size_;
    }

    /**
     * @brief Clears the StaticVector, removing all elements.
     */
    constexpr void clear() noexcept { m_size_ = 0; }

    /**
     * @brief Checks if the StaticVector is empty.
     *
     * @return True if the StaticVector is empty, false otherwise.
     */
    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return m_size_ == 0;
    }

    /**
     * @brief Returns the number of elements in the StaticVector.
     *
     * @return The number of elements in the StaticVector.
     */
    [[nodiscard]] constexpr auto size() const noexcept -> size_type {
        return m_size_;
    }

    /**
     * @brief Returns the capacity of the StaticVector.
     *
     * @return The capacity of the StaticVector.
     */
    [[nodiscard]] constexpr auto capacity() const noexcept -> size_type {
        return Capacity;
    }

    /**
     * @brief Accesses an element by index.
     *
     * @param index The index of the element to access.
     * @return A reference to the element at the specified index.
     */
    [[nodiscard]] constexpr auto operator[](size_type index) noexcept
        -> reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    /**
     * @brief Accesses an element by index.
     *
     * @param index The index of the element to access.
     * @return A const reference to the element at the specified index.
     */
    [[nodiscard]] constexpr auto operator[](size_type index) const noexcept
        -> const_reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    /**
     * @brief Accesses an element by index with bounds checking.
     *
     * @param index The index of the element to access.
     * @return A reference to the element at the specified index.
     * @throws std::out_of_range if the index is out of bounds.
     */
    [[nodiscard]] constexpr auto at(size_type index) -> reference {
        if (index >= m_size_) {
            throw std::out_of_range("StaticVector::at: index out of range");
        }
        return m_data_[index];
    }

    /**
     * @brief Accesses an element by index with bounds checking.
     *
     * @param index The index of the element to access.
     * @return A const reference to the element at the specified index.
     * @throws std::out_of_range if the index is out of bounds.
     */
    [[nodiscard]] constexpr auto at(size_type index) const -> const_reference {
        if (index >= m_size_) {
            throw std::out_of_range("StaticVector::at: index out of range");
        }
        return m_data_[index];
    }

    /**
     * @brief Accesses the first element.
     *
     * @return A reference to the first element.
     */
    [[nodiscard]] constexpr auto front() noexcept -> reference {
        if (m_size_ == 0) {
            throw std::underflow_error("StaticVector is empty");
        }
        return m_data_[0];
    }

    /**
     * @brief Accesses the first element.
     *
     * @return A const reference to the first element.
     */
    [[nodiscard]] constexpr auto front() const noexcept -> const_reference {
        if (m_size_ == 0) {
            throw std::underflow_error("StaticVector is empty");
        }
        return m_data_[0];
    }

    /**
     * @brief Accesses the last element.
     *
     * @return A reference to the last element.
     */
    [[nodiscard]] constexpr auto back() noexcept -> reference {
        if (m_size_ == 0) {
            throw std::underflow_error("StaticVector is empty");
        }
        return m_data_[m_size_ - 1];
    }

    /**
     * @brief Accesses the last element.
     *
     * @return A const reference to the last element.
     */
    [[nodiscard]] constexpr auto back() const noexcept -> const_reference {
        if (m_size_ == 0) {
            throw std::underflow_error("StaticVector is empty");
        }
        return m_data_[m_size_ - 1];
    }

    /**
     * @brief Returns a pointer to the underlying data.
     *
     * @return A pointer to the underlying data.
     */
    [[nodiscard]] constexpr auto data() noexcept -> pointer {
        return m_data_.data();
    }

    /**
     * @brief Returns a const pointer to the underlying data.
     *
     * @return A const pointer to the underlying data.
     */
    [[nodiscard]] constexpr auto data() const noexcept -> const_pointer {
        return m_data_.data();
    }

    /**
     * @brief Returns an iterator to the beginning of the StaticVector.
     *
     * @return An iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto begin() noexcept -> iterator { return data(); }

    /**
     * @brief Returns a const iterator to the beginning of the StaticVector.
     *
     * @return A const iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
        return data();
    }

    /**
     * @brief Returns an iterator to the end of the StaticVector.
     *
     * @return An iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto end() noexcept -> iterator {
        return data() + m_size_;
    }

    /**
     * @brief Returns a const iterator to the end of the StaticVector.
     *
     * @return A const iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
        return data() + m_size_;
    }

    /**
     * @brief Returns a reverse iterator to the beginning of the StaticVector.
     *
     * @return A reverse iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator {
        return reverse_iterator(end());
    }

    /**
     * @brief Returns a const reverse iterator to the beginning of the
     * StaticVector.
     *
     * @return A const reverse iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto rbegin() const noexcept
        -> const_reverse_iterator {
        return const_reverse_iterator(end());
    }

    /**
     * @brief Returns a reverse iterator to the end of the StaticVector.
     *
     * @return A reverse iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator {
        return reverse_iterator(begin());
    }

    /**
     * @brief Returns a const reverse iterator to the end of the StaticVector.
     *
     * @return A const reverse iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto rend() const noexcept
        -> const_reverse_iterator {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief Returns a const iterator to the beginning of the StaticVector.
     *
     * @return A const iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator {
        return begin();
    }

    /**
     * @brief Returns a const iterator to the end of the StaticVector.
     *
     * @return A const iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
        return end();
    }

    /**
     * @brief Returns a const reverse iterator to the beginning of the
     * StaticVector.
     *
     * @return A const reverse iterator to the beginning of the StaticVector.
     */
    [[nodiscard]] constexpr auto crbegin() const noexcept
        -> const_reverse_iterator {
        return rbegin();
    }

    /**
     * @brief Returns a const reverse iterator to the end of the StaticVector.
     *
     * @return A const reverse iterator to the end of the StaticVector.
     */
    [[nodiscard]] constexpr auto crend() const noexcept
        -> const_reverse_iterator {
        return rend();
    }

    /**
     * @brief Swaps the contents of the StaticVector with another StaticVector.
     *
     * @param other The StaticVector to swap with.
     */
    constexpr void swap(StaticVector& other) noexcept {
        std::ranges::swap(m_data_, other.m_data_);
        std::swap(m_size_, other.m_size_);
    }

    /**
     * @brief Equality operator.
     *
     * @param lhs The left-hand side StaticVector.
     * @param rhs The right-hand side StaticVector.
     * @return True if the StaticVectors are equal, false otherwise.
     */
    [[nodiscard]] constexpr auto operator==(
        const StaticVector& other) const noexcept -> bool {
        return m_size_ == other.m_size_ &&
               std::ranges::equal(m_data_, other.m_data_);
    }

    /**
     * @brief Three-way comparison operator.
     *
     * @param lhs The left-hand side StaticVector.
     * @param rhs The right-hand side StaticVector.
     * @return The result of the three-way comparison.
     */
    [[nodiscard]] constexpr auto operator<=>(
        const StaticVector& other) const noexcept {
        return std::lexicographical_compare_three_way(
            begin(), end(), other.begin(), other.end());
    }

private:
    std::array<T, Capacity> m_data_{};
    size_type m_size_{0};
};

// Swap function for StaticVector
template <typename T, std::size_t Capacity>
constexpr void swap(StaticVector<T, Capacity>& lhs,
                    StaticVector<T, Capacity>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif  // ATOM_TYPE_STATIC_VECTOR_HPP