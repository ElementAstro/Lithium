/*
 * flat_multimap.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: Flat MultiMap

**************************************************/

#ifndef ATOM_FLAT_MULTIMAP_HPP
#define ATOM_FLAT_MULTIMAP_HPP

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename Key, typename T, typename Compare = std::less<Key>>
class flat_multimap {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;
    using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
    using const_reverse_iterator =
        typename std::vector<value_type>::const_reverse_iterator;

    flat_multimap() = default;

    explicit flat_multimap(const Compare& comp) : compare_(comp) {}

    template <typename InputIt>
    flat_multimap(InputIt first, InputIt last, const Compare& comp = Compare())
        : compare_(comp), data_(first, last) {
        std::sort(data_.begin(), data_.end(),
                  [this](const auto& lhs, const auto& rhs) {
                      return compare_(lhs.first, rhs.first);
                  });
    }

    flat_multimap(std::initializer_list<value_type> init,
                  const Compare& comp = Compare())
        : flat_multimap(init.begin(), init.end(), comp) {}

    flat_multimap(const flat_multimap&) = default;
    flat_multimap(flat_multimap&&) noexcept = default;

    flat_multimap& operator=(const flat_multimap&) = default;
    flat_multimap& operator=(flat_multimap&&) noexcept = default;

    [[nodiscard]] iterator begin() noexcept { return data_.begin(); }
    [[nodiscard]] const_iterator begin() const noexcept {
        return data_.begin();
    }
    [[nodiscard]] const_iterator cbegin() const noexcept {
        return data_.cbegin();
    }

    [[nodiscard]] iterator end() noexcept { return data_.end(); }
    [[nodiscard]] const_iterator end() const noexcept { return data_.end(); }
    [[nodiscard]] const_iterator cend() const noexcept { return data_.cend(); }

    [[nodiscard]] reverse_iterator rbegin() noexcept { return data_.rbegin(); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return data_.rbegin();
    }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return data_.crbegin();
    }

    [[nodiscard]] reverse_iterator rend() noexcept { return data_.rend(); }
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return data_.rend();
    }
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return data_.crend();
    }

    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] size_type size() const noexcept { return data_.size(); }
    [[nodiscard]] size_type max_size() const noexcept {
        return data_.max_size();
    }

    template <typename... Args>
    iterator emplace(Args&&... args) {
        auto pos = std::lower_bound(data_.begin(), data_.end(),
                                    std::forward<Args>(args)...,
                                    [this](const auto& lhs, const auto& rhs) {
                                        return compare_(lhs.first, rhs.first);
                                    });
        return data_.emplace(pos, std::forward<Args>(args)...);
    }

    iterator insert(const value_type& value) { return emplace(value); }

    iterator insert(value_type&& value) { return emplace(std::move(value)); }

    template <typename InputIt>
    void insert(InputIt first, InputIt last) {
        data_.insert(data_.end(), first, last);
        std::sort(data_.begin(), data_.end(),
                  [this](const auto& lhs, const auto& rhs) {
                      return compare_(lhs.first, rhs.first);
                  });
    }

    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    template <typename... Args>
    iterator try_emplace(const key_type& k, Args&&... args) {
        auto pos = lower_bound(k);
        if (pos != end() && !compare_(k, pos->first)) {
            return pos;
        }
        return emplace(std::piecewise_construct, std::forward_as_tuple(k),
                       std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template <typename... Args>
    iterator try_emplace(key_type&& k, Args&&... args) {
        auto pos = lower_bound(k);
        if (pos != end() && !compare_(k, pos->first)) {
            return pos;
        }
        return emplace(std::piecewise_construct,
                       std::forward_as_tuple(std::move(k)),
                       std::forward_as_tuple(std::forward<Args>(args)...));
    }

    iterator erase(iterator pos) { return data_.erase(pos); }

    iterator erase(const_iterator pos) { return data_.erase(pos); }

    iterator erase(const_iterator first, const_iterator last) {
        return data_.erase(first, last);
    }

    size_type erase(const key_type& key) {
        auto [first, last] = equal_range(key);
        auto cnt = std::distance(first, last);
        erase(first, last);
        return cnt;
    }

    void swap(flat_multimap& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(compare_, other.compare_);
    }

    void clear() noexcept { data_.clear(); }

    // TODO: Fix this
    template <typename K>
    [[nodiscard]] iterator find(const K& key) {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return end();
    }

    template <typename K>
    [[nodiscard]] const_iterator find(const K& key) const {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return end();
    }

    template <typename K>
    [[nodiscard]] size_type count(const K& key) const {
        auto [first, last] = equal_range(key);
        return std::distance(first, last);
    }

    template <typename K>
    [[nodiscard]] iterator lower_bound(const K& key) {
        return std::lower_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] const_iterator lower_bound(const K& key) const {
        return std::lower_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] iterator upper_bound(const K& key) {
        return std::upper_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs, rhs.first);
                                });
    }

    template <typename K>
    [[nodiscard]] const_iterator upper_bound(const K& key) const {
        return std::upper_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs, rhs.first);
                                });
    }

    template <typename K>
    [[nodiscard]] std::pair<iterator, iterator> equal_range(const K& key) {
        return std::equal_range(begin(), end(), key,
                                [this](const value_type& lhs, const K& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(
        const K& key) const {
        return std::equal_range(begin(), end(), key,
                                [this](const value_type& lhs, const K& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    [[nodiscard]] key_compare key_comp() const { return compare_; }

    [[nodiscard]] value_type* data() noexcept { return data_.data(); }

    [[nodiscard]] const value_type* data() const noexcept {
        return data_.data();
    }

private:
    std::vector<value_type> data_;
    Compare compare_;
};

template <typename Key, typename T, typename Compare>
bool operator==(const flat_multimap<Key, T, Compare>& lhs,
                const flat_multimap<Key, T, Compare>& rhs) {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Key, typename T, typename Compare>
bool operator!=(const flat_multimap<Key, T, Compare>& lhs,
                const flat_multimap<Key, T, Compare>& rhs) {
    return !(lhs == rhs);
}

template <typename Key, typename T, typename Compare>
bool operator<(const flat_multimap<Key, T, Compare>& lhs,
               const flat_multimap<Key, T, Compare>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename Key, typename T, typename Compare>
bool operator<=(const flat_multimap<Key, T, Compare>& lhs,
                const flat_multimap<Key, T, Compare>& rhs) {
    return !(rhs < lhs);
}

template <typename Key, typename T, typename Compare>
bool operator>(const flat_multimap<Key, T, Compare>& lhs,
               const flat_multimap<Key, T, Compare>& rhs) {
    return rhs < lhs;
}

template <typename Key, typename T, typename Compare>
bool operator>=(const flat_multimap<Key, T, Compare>& lhs,
                const flat_multimap<Key, T, Compare>& rhs) {
    return !(lhs < rhs);
}

template <typename Key, typename T, typename Compare>
void swap(
    flat_multimap<Key, T, Compare>& lhs,
    flat_multimap<Key, T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

#endif  // ATOM_FLAT_MULTIMAP_HPP
