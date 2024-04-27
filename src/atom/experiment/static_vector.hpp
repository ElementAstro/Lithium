/*
 * static_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: A static vector

**************************************************/

#ifndef ATOM_EXPERIMENT_STATIC_VECTOR_HPP
#define ATOM_EXPERIMENT_STATIC_VECTOR_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T, std::size_t Capacity>
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

    constexpr StaticVector() noexcept = default;

    constexpr StaticVector(std::initializer_list<T> init) noexcept {
        assert(init.size() <= Capacity);
        std::copy(init.begin(), init.end(), begin());
        m_size = init.size();
    }

    constexpr void push_back(const T& value) noexcept {
        assert(m_size < Capacity);
        m_data[m_size++] = value;
    }

    constexpr void push_back(T&& value) noexcept {
        assert(m_size < Capacity);
        m_data[m_size++] = std::move(value);
    }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args) noexcept {
        assert(m_size < Capacity);
        return m_data[m_size++] = T(std::forward<Args>(args)...);
    }

    constexpr void pop_back() noexcept {
        assert(m_size > 0);
        --m_size;
    }

    constexpr void clear() noexcept { m_size = 0; }

    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

    [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return Capacity;
    }

    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        assert(index < m_size);
        return m_data[index];
    }

    [[nodiscard]] constexpr const_reference operator[](
        size_type index) const noexcept {
        assert(index < m_size);
        return m_data[index];
    }

    [[nodiscard]] constexpr reference at(size_type index) {
        if (index >= m_size) {
            throw std::out_of_range("StaticVector::at");
        }
        return m_data[index];
    }

    [[nodiscard]] constexpr const_reference at(size_type index) const {
        if (index >= m_size) {
            throw std::out_of_range("StaticVector::at");
        }
        return m_data[index];
    }

    [[nodiscard]] constexpr reference front() noexcept {
        assert(m_size > 0);
        return m_data[0];
    }

    [[nodiscard]] constexpr const_reference front() const noexcept {
        assert(m_size > 0);
        return m_data[0];
    }

    [[nodiscard]] constexpr reference back() noexcept {
        assert(m_size > 0);
        return m_data[m_size - 1];
    }

    [[nodiscard]] constexpr const_reference back() const noexcept {
        assert(m_size > 0);
        return m_data[m_size - 1];
    }

    [[nodiscard]] constexpr pointer data() noexcept { return m_data.data(); }

    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return m_data.data();
    }

    [[nodiscard]] constexpr iterator begin() noexcept { return data(); }

    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return data();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] constexpr iterator end() noexcept { return data() + m_size; }

    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return data() + m_size;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return rend();
    }

    constexpr void swap(StaticVector& other) noexcept {
        using std::swap;
        swap(m_data, other.m_data);
        swap(m_size, other.m_size);
    }

private:
    std::array<T, Capacity> m_data{};
    size_type m_size{0};
};

template <typename T, std::size_t Capacity>
constexpr bool operator==(const StaticVector<T, Capacity>& lhs,
                          const StaticVector<T, Capacity>& rhs) noexcept {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, std::size_t Capacity>
constexpr auto operator<=>(const StaticVector<T, Capacity>& lhs,
                           const StaticVector<T, Capacity>& rhs) noexcept {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                  rhs.begin(), rhs.end());
}

template <typename T, std::size_t Capacity>
constexpr void swap(StaticVector<T, Capacity>& lhs,
                    StaticVector<T, Capacity>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif  // ATOM_EXPERIMENT_STATIC_VECTOR_HPP