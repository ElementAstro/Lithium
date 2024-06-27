/*
 * flatset.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-16

Description: A FlatSet

**************************************************/

#ifndef ATOM_TYPE_FLAT_SET_HPP
#define ATOM_TYPE_FLAT_SET_HPP

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <vector>

namespace atom::type {
template <typename T, typename Compare = std::less<T>>
class FlatSet {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = typename std::vector<T>::const_iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using key_compare = Compare;
    using value_compare = Compare;

private:
    std::vector<T> m_data_;
    Compare m_comp_;

public:
    FlatSet() = default;
    ~FlatSet() = default;

    explicit FlatSet(const Compare& comp) : m_comp_(comp) {}

    template <typename InputIt>
    FlatSet(InputIt first, InputIt last, const Compare& comp = Compare())
        : m_data_(first, last), m_comp_(comp) {
        std::sort(m_data_.begin(), m_data_.end(), m_comp_);
        m_data_.erase(std::unique(m_data_.begin(), m_data_.end()), m_data_.end());
    }

    FlatSet(std::initializer_list<T> init, const Compare& comp = Compare())
        : FlatSet(init.begin(), init.end(), comp) {}

    FlatSet(const FlatSet& other) = default;
    FlatSet(FlatSet&& other) noexcept = default;

    FlatSet& operator=(const FlatSet& other) = default;
    FlatSet& operator=(FlatSet&& other) noexcept = default;

    iterator begin() noexcept { return m_data_.begin(); }
    const_iterator begin() const noexcept { return m_data_.begin(); }
    const_iterator cbegin() const noexcept { return m_data_.cbegin(); }

    iterator end() noexcept { return m_data_.end(); }
    const_iterator end() const noexcept { return m_data_.end(); }
    const_iterator cend() const noexcept { return m_data_.cend(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] auto empty() const noexcept -> bool { return m_data_.empty(); }
    [[nodiscard]] auto size() const noexcept -> size_type { return m_data_.size(); }
    [[nodiscard]] auto maxSize() const noexcept -> size_type { return m_data_.max_size(); }

    void clear() noexcept { m_data_.clear(); }

    auto insert(const T& value) -> std::pair<iterator, bool> {
        auto data = std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return {data, false};
        }
        return {m_data_.insert(data, value), true};
    }

    auto insert(T&& value) -> std::pair<iterator, bool> {
        auto data = std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return {data, false};
        }
        return {m_data_.insert(data, std::move(value)), true};
    }

    auto insert(const_iterator hint, const T& value) -> iterator {
        if (hint == m_data_.end() || m_comp_(value, *hint)) {
            if (hint == m_data_.begin() || m_comp_(*(hint - 1), value)) {
                return m_data_.insert(hint, value);
            }
            return insert(value).first;
        }
        return hint;
    }

    auto insert(const_iterator hint, T&& value) -> iterator {
        if (hint == m_data_.end() || m_comp_(value, *hint)) {
            if (hint == m_data_.begin() || m_comp_(*(hint - 1), value)) {
                return m_data_.insert(hint, std::move(value));
            }
            return insert(std::move(value)).first;
        }
        return hint;
    }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first++);
        }
    }

    void insert(std::initializer_list<T> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return insert(T(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto emplaceHint(const_iterator hint, Args&&... args) -> iterator {
        return insert(hint, T(std::forward<Args>(args)...));
    }

    auto erase(const_iterator pos) -> iterator { return m_data_.erase(pos); }

    auto erase(const_iterator first, const_iterator last) -> iterator {
        return m_data_.erase(first, last);
    }

    auto erase(const T& value) -> size_type {
        auto data = find(value);
        if (data != m_data_.end()) {
            erase(data);
            return 1;
        }
        return 0;
    }

    void swap(FlatSet& other) noexcept {
        std::swap(m_data_, other.m_data_);
        std::swap(m_comp_, other.m_comp_);
    }

    auto count(const T& value) const -> size_type {
        return find(value) != m_data_.end() ? 1 : 0;
    }

    auto find(const T& value) -> iterator {
        auto data = std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return data;
        }
        return m_data_.end();
    }

    auto find(const T& value) const -> const_iterator {
        auto data = std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return data;
        }
        return m_data_.end();
    }

    auto equalRange(const T& value) -> std::pair<iterator, iterator> {
        return std::equal_range(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto equalRange(
        const T& value) const -> std::pair<const_iterator, const_iterator> {
        return std::equal_range(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto lowerBound(const T& value) -> iterator {
        return std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto lowerBound(const T& value) const -> const_iterator {
        return std::lower_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto upperBound(const T& value) -> iterator {
        return std::upper_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto upperBound(const T& value) const -> const_iterator {
        return std::upper_bound(m_data_.begin(), m_data_.end(), value, m_comp_);
    }

    auto keyComp() const -> key_compare { return m_comp_; }

    auto valueComp() const -> value_compare { return m_comp_; }

    auto contains(const T& value) const -> bool { return find(value) != m_data_.end(); }
};

template <typename T, typename Compare>
auto operator==(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Compare>
auto operator!=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(lhs == rhs);
}

template <typename T, typename Compare>
auto operator<(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs) -> bool {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename T, typename Compare>
auto operator<=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(rhs < lhs);
}

template <typename T, typename Compare>
auto operator>(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs) -> bool {
    return rhs < lhs;
}

template <typename T, typename Compare>
auto operator>=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(lhs < rhs);
}

template <typename T, typename Compare>
void swap(FlatSet<T, Compare>& lhs,
          FlatSet<T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}
}  // namespace atom::type

#endif  // ATOM_TYPE_FLAT_SET_HPP
