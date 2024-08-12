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
#include <utility>
#include <vector>

template <typename Key, typename T, typename Compare = std::less<Key>>
class FlatMultimap {
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

    FlatMultimap() = default;
    ~FlatMultimap() = default;

    explicit FlatMultimap(const Compare& comp) : compare_(comp) {}

    template <typename InputIt>
    FlatMultimap(InputIt first, InputIt last, const Compare& comp = Compare())
        : compare_(comp), data_(first, last) {
        std::sort(data_.begin(), data_.end(),
                  [this](const auto& lhs, const auto& rhs) {
                      return compare_(lhs.first, rhs.first);
                  });
    }

    FlatMultimap(std::initializer_list<value_type> init,
                 const Compare& comp = Compare())
        : FlatMultimap(init.begin(), init.end(), comp) {}

    FlatMultimap(const FlatMultimap&) = default;
    FlatMultimap(FlatMultimap&&) noexcept = default;

    auto operator=(const FlatMultimap&) -> FlatMultimap& = default;
    auto operator=(FlatMultimap&&) noexcept -> FlatMultimap& = default;

    [[nodiscard]] auto begin() noexcept -> iterator { return data_.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator {
        return data_.begin();
    }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator {
        return data_.cbegin();
    }

    [[nodiscard]] auto end() noexcept -> iterator { return data_.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator {
        return data_.end();
    }
    [[nodiscard]] auto cend() const noexcept -> const_iterator {
        return data_.cend();
    }

    [[nodiscard]] auto rbegin() noexcept -> reverse_iterator {
        return data_.rbegin();
    }
    [[nodiscard]] auto rbegin() const noexcept -> const_reverse_iterator {
        return data_.rbegin();
    }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator {
        return data_.crbegin();
    }

    [[nodiscard]] auto rend() noexcept -> reverse_iterator {
        return data_.rend();
    }
    [[nodiscard]] auto rend() const noexcept -> const_reverse_iterator {
        return data_.rend();
    }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator {
        return data_.crend();
    }

    [[nodiscard]] auto empty() const noexcept -> bool { return data_.empty(); }
    [[nodiscard]] auto size() const noexcept -> size_type {
        return data_.size();
    }
    [[nodiscard]] auto maxSize() const noexcept -> size_type {
        return data_.max_size();
    }

    template <typename... Args>
    auto emplace(Args&&... args) -> iterator {
        auto pos = std::lower_bound(data_.begin(), data_.end(),
                                    std::forward<Args>(args)...,
                                    [this](const auto& lhs, const auto& rhs) {
                                        return compare_(lhs.first, rhs.first);
                                    });
        return data_.emplace(pos, std::forward<Args>(args)...);
    }

    auto insert(const value_type& value) -> iterator { return emplace(value); }

    auto insert(value_type&& value) -> iterator {
        return emplace(std::move(value));
    }

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
    auto tryEmplace(const key_type& key, Args&&... args) -> iterator {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return emplace(std::piecewise_construct, std::forward_as_tuple(key),
                       std::forward_as_tuple(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto tryEmplace(key_type&& key, Args&&... args) -> iterator {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return emplace(std::piecewise_construct,
                       std::forward_as_tuple(std::move(key)),
                       std::forward_as_tuple(std::forward<Args>(args)...));
    }

    auto erase(iterator pos) -> iterator { return data_.erase(pos); }

    auto erase(const_iterator pos) -> iterator { return data_.erase(pos); }

    auto erase(const_iterator first, const_iterator last) -> iterator {
        return data_.erase(first, last);
    }

    auto erase(const key_type& key) -> size_type {
        auto [first, last] = equal_range(key);
        auto cnt = std::distance(first, last);
        erase(first, last);
        return cnt;
    }

    void swap(FlatMultimap& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(compare_, other.compare_);
    }

    void clear() noexcept { data_.clear(); }

    // TODO: Fix this
    template <typename K>
    [[nodiscard]] auto find(const K& key) -> iterator {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return end();
    }

    template <typename K>
    [[nodiscard]] auto find(const K& key) const -> const_iterator {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos;
        }
        return end();
    }

    template <typename K>
    [[nodiscard]] auto count(const K& key) const -> size_type {
        auto [first, last] = equal_range(key);
        return std::distance(first, last);
    }

    template <typename K>
    [[nodiscard]] auto lowerBound(const K& key) -> iterator {
        return std::lower_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] auto lowerBound(const K& key) const -> const_iterator {
        return std::lower_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] auto upperBound(const K& key) -> iterator {
        return std::upper_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs, rhs.first);
                                });
    }

    template <typename K>
    [[nodiscard]] auto upperBound(const K& key) const -> const_iterator {
        return std::upper_bound(begin(), end(), key,
                                [this](const auto& lhs, const auto& rhs) {
                                    return compare_(lhs, rhs.first);
                                });
    }

    template <typename K>
    [[nodiscard]] auto equalRange(const K& key)
        -> std::pair<iterator, iterator> {
        return std::equal_range(begin(), end(), key,
                                [this](const value_type& lhs, const K& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    template <typename K>
    [[nodiscard]] auto equalRange(const K& key) const
        -> std::pair<const_iterator, const_iterator> {
        return std::equal_range(begin(), end(), key,
                                [this](const value_type& lhs, const K& rhs) {
                                    return compare_(lhs.first, rhs);
                                });
    }

    [[nodiscard]] auto keyComp() const -> key_compare { return compare_; }

    [[nodiscard]] auto data() noexcept -> value_type* { return data_.data(); }

    [[nodiscard]] auto data() const noexcept -> const value_type* {
        return data_.data();
    }

    auto operator[](const key_type& key) -> mapped_type& {
        auto pos = lower_bound(key);
        if (pos != end() && !compare_(key, pos->first)) {
            return pos->second;
        }
        return emplace(key, T()).first->second;
    }

private:
    std::vector<value_type> data_;
    Compare compare_;
};

template <typename Key, typename T, typename Compare>
auto operator==(const FlatMultimap<Key, T, Compare>& lhs,
                const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Key, typename T, typename Compare>
auto operator!=(const FlatMultimap<Key, T, Compare>& lhs,
                const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return !(lhs == rhs);
}

template <typename Key, typename T, typename Compare>
auto operator<(const FlatMultimap<Key, T, Compare>& lhs,
               const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename Key, typename T, typename Compare>
auto operator<=(const FlatMultimap<Key, T, Compare>& lhs,
                const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return !(rhs < lhs);
}

template <typename Key, typename T, typename Compare>
auto operator>(const FlatMultimap<Key, T, Compare>& lhs,
               const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return rhs < lhs;
}

template <typename Key, typename T, typename Compare>
auto operator>=(const FlatMultimap<Key, T, Compare>& lhs,
                const FlatMultimap<Key, T, Compare>& rhs) -> bool {
    return !(lhs < rhs);
}

template <typename Key, typename T, typename Compare>
void swap(
    FlatMultimap<Key, T, Compare>& lhs,
    FlatMultimap<Key, T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

#endif  // ATOM_FLAT_MULTIMAP_HPP
