/*
 * flatmap.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-2

Description: QuickFlatMap for C++20

**************************************************/

#ifndef ATOM_TYPE_FLATMAP_HPP
#define ATOM_TYPE_FLATMAP_HPP

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Comparator = std::equal_to<>>
class QuickFlatMap {
public:
    using value_type = std::pair<Key, Value>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    QuickFlatMap() = default;

    template <typename Lookup>
    auto find(const Lookup &s) noexcept -> iterator {
        return std::find_if(
            data_.begin(), data_.end(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
    }

    template <typename Lookup>
    auto find(const Lookup &s) const noexcept -> const_iterator {
        return std::find_if(
            data_.cbegin(), data_.cend(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
    }

    template <typename Lookup>
    auto find(const Lookup &s,
              std::size_t t_hint) const noexcept -> const_iterator {
        if constexpr (std::is_invocable_v<decltype(comparator_), const Key &,
                                          const Lookup &>) {
            if (data_.size() > t_hint && comparator_(data_[t_hint].first, s)) {
                return data_.cbegin() + t_hint;
            }
            return find(s);
        } else {
            if (data_.size() > t_hint && comparator_(s, data_[t_hint].first)) {
                return data_.cbegin() + t_hint;
            }
            return find(s);
        }
    }

    auto size() const noexcept -> std::size_t { return data_.size(); }

    auto empty() const noexcept -> bool { return data_.empty(); }

    auto begin() noexcept -> iterator { return data_.begin(); }
    auto begin() const noexcept -> const_iterator { return data_.begin(); }
    auto end() noexcept -> iterator { return data_.end(); }
    auto end() const noexcept -> const_iterator { return data_.end(); }

    auto operator[](const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        grow();
        return data_.emplace_back(s, Value()).second;
    }

    auto atIndex(std::size_t idx) noexcept -> Value & {
        return data_[idx].second;
    }
    auto atIndex(std::size_t idx) const noexcept -> const Value & {
        return data_[idx].second;
    }

    auto at(const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    const Value &at(const Key &s) const {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    template <typename M>
    auto insertOrAssign(const Key &key, M &&m) -> std::pair<iterator, bool> {
        if (auto itr = find(key); itr != data_.end()) {
            itr->second = std::forward<M>(m);
            return {itr, false};
        }
        grow();
        return {data_.emplace(data_.end(), key, std::forward<M>(m)), true};
    }

    std::pair<iterator, bool> insert(value_type value) {
        if (auto itr = find(value.first); itr != data_.end()) {
            return {itr, false};
        }
        grow();
        return {data_.insert(data_.end(), std::move(value)), true};
    }

    template <typename Itr>
    void assign(Itr first, Itr last) {
        data_.assign(first, last);
    }

    void grow() {
        if (data_.capacity() == data_.size()) {
            data_.reserve(data_.size() + 2);
        }
    }

private:
    std::vector<value_type> data_;
    Comparator comparator_;
};

template <typename Key, typename Value, typename Comparator = std::equal_to<>>
class QuickFlatMultiMap {
public:
    using value_type = std::pair<Key, Value>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    QuickFlatMultiMap() = default;

    template <typename Lookup>
    auto find(const Lookup &s) noexcept -> iterator {
        return std::find_if(
            data_.begin(), data_.end(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
    }

    template <typename Lookup>
    auto find(const Lookup &s) const noexcept -> const_iterator {
        return std::find_if(
            data_.cbegin(), data_.cend(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
    }

    template <typename Lookup>
    auto equalRange(const Lookup &s) noexcept -> std::pair<iterator, iterator> {
        auto lower = std::find_if(
            data_.begin(), data_.end(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        if (lower == data_.end()) {
            return {lower, lower};
        }

        auto upper = std::find_if_not(
            lower, data_.end(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        return {lower, upper};
    }

    template <typename Lookup>
    auto equalRange(const Lookup &s) const noexcept
        -> std::pair<const_iterator, const_iterator> {
        auto lower = std::find_if(
            data_.cbegin(), data_.cend(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        if (lower == data_.cend()) {
            return {lower, lower};
        }

        auto upper = std::find_if_not(
            lower, data_.cend(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        return {lower, upper};
    }

    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return data_.size();
    }

    [[nodiscard]] auto empty() const noexcept -> bool { return data_.empty(); }

    auto begin() noexcept -> iterator { return data_.begin(); }
    auto begin() const noexcept -> const_iterator { return data_.begin(); }
    auto end() noexcept -> iterator { return data_.end(); }
    auto end() const noexcept -> const_iterator { return data_.end(); }

    auto operator[](const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        grow();
        return data_.emplace_back(s, Value()).second;
    }

    auto atIndex(std::size_t idx) noexcept -> Value & {
        return data_[idx].second;
    }
    auto atIndex(std::size_t idx) const noexcept -> const Value & {
        return data_[idx].second;
    }

    auto at(const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    const Value &at(const Key &s) const {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    auto insert(value_type value) -> std::pair<iterator, bool> {
        grow();
        return {data_.insert(data_.end(), std::move(value)), true};
    }

    template <typename Itr>
    void assign(Itr first, Itr last) {
        data_.assign(first, last);
    }

    void grow() {
        if (data_.capacity() == data_.size()) {
            data_.reserve(data_.size() + 2);
        }
    }

    auto count(const Key &s) const -> size_t {
        auto [lower, upper] = equal_range(s);
        return std::distance(lower, upper);
    }

private:
    std::vector<value_type> data_;
    Comparator comparator_;
};

#endif  // ATOM_TYPE_FLATMAP_HPP
