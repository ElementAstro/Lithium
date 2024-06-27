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
        m_size_ = init.size();
    }

    constexpr void pushBack(const T& value) noexcept {
        assert(m_size_ < Capacity);
        m_data_[m_size_++] = value;
    }

    constexpr void pushBack(T&& value) noexcept {
        assert(m_size_ < Capacity);
        m_data_[m_size_++] = std::move(value);
    }

    template <typename... Args>
    constexpr auto emplaceBack(Args&&... args) noexcept -> reference {
        assert(m_size_ < Capacity);
        return m_data_[m_size_++] = T(std::forward<Args>(args)...);
    }

    constexpr void popBack() noexcept {
        assert(m_size_ > 0);
        --m_size_;
    }

    constexpr void clear() noexcept { m_size_ = 0; }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return m_size_ == 0;
    }

    [[nodiscard]] constexpr auto size() const noexcept -> size_type {
        return m_size_;
    }

    [[nodiscard]] constexpr auto capacity() const noexcept -> size_type {
        return Capacity;
    }

    [[nodiscard]] constexpr auto operator[](size_type index) noexcept
        -> reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    [[nodiscard]] constexpr auto operator[](size_type index) const noexcept
        -> const_reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    [[nodiscard]] constexpr auto at(size_type index) -> reference {
        if (index >= m_size_) {
            throw std::out_of_range("StaticVector::at");
        }
        return m_data_[index];
    }

    [[nodiscard]] constexpr auto at(size_type index) const -> const_reference {
        if (index >= m_size_) {
            throw std::out_of_range("StaticVector::at");
        }
        return m_data_[index];
    }

    [[nodiscard]] constexpr auto front() noexcept -> reference {
        assert(m_size_ > 0);
        return m_data_[0];
    }

    [[nodiscard]] constexpr auto front() const noexcept -> const_reference {
        assert(m_size_ > 0);
        return m_data_[0];
    }

    [[nodiscard]] constexpr auto back() noexcept -> reference {
        assert(m_size_ > 0);
        return m_data_[m_size_ - 1];
    }

    [[nodiscard]] constexpr auto back() const noexcept -> const_reference {
        assert(m_size_ > 0);
        return m_data_[m_size_ - 1];
    }

    [[nodiscard]] constexpr auto data() noexcept -> pointer {
        return m_data_.data();
    }

    [[nodiscard]] constexpr auto data() const noexcept -> const_pointer {
        return m_data_.data();
    }

    [[nodiscard]] constexpr auto begin() noexcept -> iterator { return data(); }

    [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
        return data();
    }

    [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator {
        return begin();
    }

    [[nodiscard]] constexpr auto end() noexcept -> iterator {
        return data() + m_size_;
    }

    [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
        return data() + m_size_;
    }

    [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
        return end();
    }

    [[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr auto rbegin() const noexcept
        -> const_reverse_iterator {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] constexpr auto crbegin() const noexcept
        -> const_reverse_iterator {
        return rbegin();
    }

    [[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr auto rend() const noexcept
        -> const_reverse_iterator {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr auto crend() const noexcept
        -> const_reverse_iterator {
        return rend();
    }

    constexpr void swap(StaticVector& other) noexcept {
        using std::swap;
        swap(m_data_, other.m_data_);
        swap(m_size_, other.m_size_);
    }

private:
    std::array<T, Capacity> m_data_{};
    size_type m_size_{0};
};

template <typename T, std::size_t Capacity>
constexpr auto operator==(const StaticVector<T, Capacity>& lhs,
                          const StaticVector<T, Capacity>& rhs) noexcept
    -> bool {
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
