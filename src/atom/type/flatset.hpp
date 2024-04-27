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
#include <stdexcept>
#include <utility>
#include <vector>

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
    std::vector<T> m_data;
    Compare m_comp;

public:
    FlatSet() = default;

    explicit FlatSet(const Compare& comp) : m_comp(comp) {}

    template <typename InputIt>
    FlatSet(InputIt first, InputIt last, const Compare& comp = Compare())
        : m_data(first, last), m_comp(comp) {
        std::sort(m_data.begin(), m_data.end(), m_comp);
        m_data.erase(std::unique(m_data.begin(), m_data.end()), m_data.end());
    }

    FlatSet(std::initializer_list<T> init, const Compare& comp = Compare())
        : FlatSet(init.begin(), init.end(), comp) {}

    FlatSet(const FlatSet& other) = default;
    FlatSet(FlatSet&& other) noexcept = default;

    FlatSet& operator=(const FlatSet& other) = default;
    FlatSet& operator=(FlatSet&& other) noexcept = default;

    iterator begin() noexcept { return m_data.begin(); }
    const_iterator begin() const noexcept { return m_data.begin(); }
    const_iterator cbegin() const noexcept { return m_data.cbegin(); }

    iterator end() noexcept { return m_data.end(); }
    const_iterator end() const noexcept { return m_data.end(); }
    const_iterator cend() const noexcept { return m_data.cend(); }

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

    bool empty() const noexcept { return m_data.empty(); }
    size_type size() const noexcept { return m_data.size(); }
    size_type max_size() const noexcept { return m_data.max_size(); }

    void clear() noexcept { m_data.clear(); }

    std::pair<iterator, bool> insert(const T& value) {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
        if (it != m_data.end() && !m_comp(value, *it)) {
            return {it, false};
        }
        return {m_data.insert(it, value), true};
    }

    std::pair<iterator, bool> insert(T&& value) {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
        if (it != m_data.end() && !m_comp(value, *it)) {
            return {it, false};
        }
        return {m_data.insert(it, std::move(value)), true};
    }

    iterator insert(const_iterator hint, const T& value) {
        if (hint == m_data.end() || m_comp(value, *hint)) {
            if (hint == m_data.begin() || m_comp(*(hint - 1), value)) {
                return m_data.insert(hint, value);
            }
            return insert(value).first;
        }
        return hint;
    }

    iterator insert(const_iterator hint, T&& value) {
        if (hint == m_data.end() || m_comp(value, *hint)) {
            if (hint == m_data.begin() || m_comp(*(hint - 1), value)) {
                return m_data.insert(hint, std::move(value));
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
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        return insert(hint, T(std::forward<Args>(args)...));
    }

    iterator erase(const_iterator pos) { return m_data.erase(pos); }

    iterator erase(const_iterator first, const_iterator last) {
        return m_data.erase(first, last);
    }

    size_type erase(const T& value) {
        auto it = find(value);
        if (it != m_data.end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    void swap(FlatSet& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_comp, other.m_comp);
    }

    size_type count(const T& value) const {
        return find(value) != m_data.end() ? 1 : 0;
    }

    iterator find(const T& value) {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
        if (it != m_data.end() && !m_comp(value, *it)) {
            return it;
        }
        return m_data.end();
    }

    const_iterator find(const T& value) const {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
        if (it != m_data.end() && !m_comp(value, *it)) {
            return it;
        }
        return m_data.end();
    }

    std::pair<iterator, iterator> equal_range(const T& value) {
        return std::equal_range(m_data.begin(), m_data.end(), value, m_comp);
    }

    std::pair<const_iterator, const_iterator> equal_range(
        const T& value) const {
        return std::equal_range(m_data.begin(), m_data.end(), value, m_comp);
    }

    iterator lower_bound(const T& value) {
        return std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
    }

    const_iterator lower_bound(const T& value) const {
        return std::lower_bound(m_data.begin(), m_data.end(), value, m_comp);
    }

    iterator upper_bound(const T& value) {
        return std::upper_bound(m_data.begin(), m_data.end(), value, m_comp);
    }

    const_iterator upper_bound(const T& value) const {
        return std::upper_bound(m_data.begin(), m_data.end(), value, m_comp);
    }

    key_compare key_comp() const { return m_comp; }

    value_compare value_comp() const { return m_comp; }

    bool contains(const T& value) const { return find(value) != m_data.end(); }
};

template <typename T, typename Compare>
bool operator==(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Compare>
bool operator!=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) {
    return !(lhs == rhs);
}

template <typename T, typename Compare>
bool operator<(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename T, typename Compare>
bool operator<=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) {
    return !(rhs < lhs);
}

template <typename T, typename Compare>
bool operator>(const FlatSet<T, Compare>& lhs, const FlatSet<T, Compare>& rhs) {
    return rhs < lhs;
}

template <typename T, typename Compare>
bool operator>=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) {
    return !(lhs < rhs);
}

template <typename T, typename Compare>
void swap(FlatSet<T, Compare>& lhs,
          FlatSet<T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

#endif  // ATOM_TYPE_FLAT_SET_HPP