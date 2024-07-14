/*
 * static_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: A static vector

**************************************************/

#ifndef ATOM_TYPE_STATIC_VECTOR_HPP
#define ATOM_TYPE_STATIC_VECTOR_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

#include "error/exception.hpp"
#include "macro.hpp"

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

    ATOM_CONSTEXPR StaticVector() ATOM_NOEXCEPT = default;

    ATOM_CONSTEXPR StaticVector(std::initializer_list<T> init) ATOM_NOEXCEPT {
        assert(init.size() <= Capacity);
        std::copy(init.begin(), init.end(), begin());
        m_size_ = init.size();
    }

    ATOM_CONSTEXPR void pushBack(const T& value) ATOM_NOEXCEPT {
        assert(m_size_ < Capacity);
        m_data_[m_size_++] = value;
    }

    ATOM_CONSTEXPR void pushBack(T&& value) ATOM_NOEXCEPT {
        assert(m_size_ < Capacity);
        m_data_[m_size_++] = std::move(value);
    }

    template <typename... Args>
    ATOM_CONSTEXPR auto emplaceBack(Args&&... args) ATOM_NOEXCEPT -> reference {
        assert(m_size_ < Capacity);
        return m_data_[m_size_++] = T(std::forward<Args>(args)...);
    }

    ATOM_CONSTEXPR void popBack() ATOM_NOEXCEPT {
        assert(m_size_ > 0);
        --m_size_;
    }

    ATOM_CONSTEXPR void clear() ATOM_NOEXCEPT { m_size_ = 0; }

    ATOM_NODISCARD ATOM_CONSTEXPR auto empty() const ATOM_NOEXCEPT -> bool {
        return m_size_ == 0;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto size() const ATOM_NOEXCEPT -> size_type {
        return m_size_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto capacity() const ATOM_NOEXCEPT
        -> size_type {
        return Capacity;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto operator[](size_type index)
        ATOM_NOEXCEPT->reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto operator[](size_type index) const
        ATOM_NOEXCEPT->const_reference {
        assert(index < m_size_);
        return m_data_[index];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto at(size_type index) -> reference {
        if (index >= m_size_) {
            THROW_OUT_OF_RANGE("StaticVector::at");
        }
        return m_data_[index];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto at(size_type index) const
        -> const_reference {
        if (index >= m_size_) {
            THROW_OUT_OF_RANGE("StaticVector::at");
        }
        return m_data_[index];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto front() ATOM_NOEXCEPT -> reference {
        assert(m_size_ > 0);
        return m_data_[0];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto front() const ATOM_NOEXCEPT
        -> const_reference {
        assert(m_size_ > 0);
        return m_data_[0];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto back() ATOM_NOEXCEPT -> reference {
        assert(m_size_ > 0);
        return m_data_[m_size_ - 1];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto back() const ATOM_NOEXCEPT
        -> const_reference {
        assert(m_size_ > 0);
        return m_data_[m_size_ - 1];
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto data() ATOM_NOEXCEPT -> pointer {
        return m_data_.data();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto data() const ATOM_NOEXCEPT
        -> const_pointer {
        return m_data_.data();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto begin() ATOM_NOEXCEPT -> iterator {
        return data();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto begin() const ATOM_NOEXCEPT
        -> const_iterator {
        return data();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto cbegin() const ATOM_NOEXCEPT
        -> const_iterator {
        return begin();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto end() ATOM_NOEXCEPT -> iterator {
        return data() + m_size_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto end() const ATOM_NOEXCEPT
        -> const_iterator {
        return data() + m_size_;
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto cend() const ATOM_NOEXCEPT
        -> const_iterator {
        return end();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto rbegin()
        ATOM_NOEXCEPT -> reverse_iterator {
        return reverse_iterator(end());
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto rbegin() const ATOM_NOEXCEPT
        -> const_reverse_iterator {
        return const_reverse_iterator(end());
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto crbegin() const ATOM_NOEXCEPT
        -> const_reverse_iterator {
        return rbegin();
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto rend()
        ATOM_NOEXCEPT -> reverse_iterator {
        return reverse_iterator(begin());
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto rend() const ATOM_NOEXCEPT
        -> const_reverse_iterator {
        return const_reverse_iterator(begin());
    }

    ATOM_NODISCARD ATOM_CONSTEXPR auto crend() const ATOM_NOEXCEPT
        -> const_reverse_iterator {
        return rend();
    }

    ATOM_CONSTEXPR void swap(StaticVector& other) ATOM_NOEXCEPT {
        using std::swap;
        swap(m_data_, other.m_data_);
        swap(m_size_, other.m_size_);
    }

private:
    std::array<T, Capacity> m_data_{};
    size_type m_size_{0};
};

template <typename T, std::size_t Capacity>
ATOM_CONSTEXPR auto operator==(const StaticVector<T, Capacity>& lhs,
                               const StaticVector<T, Capacity>& rhs)
    ATOM_NOEXCEPT->bool {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, std::size_t Capacity>
ATOM_CONSTEXPR auto operator<=>(const StaticVector<T, Capacity>& lhs,
                                const StaticVector<T, Capacity>& rhs)
    ATOM_NOEXCEPT {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                  rhs.begin(), rhs.end());
}

template <typename T, std::size_t Capacity>
ATOM_CONSTEXPR void swap(StaticVector<T, Capacity>& lhs,
                         StaticVector<T, Capacity>& rhs) ATOM_NOEXCEPT {
    lhs.swap(rhs);
}

#endif  // ATOM_TYPE_STATIC_VECTOR_HPP
