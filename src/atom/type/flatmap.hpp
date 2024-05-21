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
#include <optional>
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
    iterator find(const Lookup &s) noexcept {
        return std::find_if(
            data.begin(), data.end(),
            [&s, this](const auto &d) { return comparator(d.first, s); });
    }

    template <typename Lookup>
    const_iterator find(const Lookup &s) const noexcept {
        return std::find_if(
            data.cbegin(), data.cend(),
            [&s, this](const auto &d) { return comparator(d.first, s); });
    }

    template <typename Lookup>
    const_iterator find(const Lookup &s, std::size_t t_hint) const noexcept {
        if constexpr (std::is_invocable_v<decltype(comparator), const Key &,
                                          const Lookup &>) {
            if (data.size() > t_hint && comparator(data[t_hint].first, s)) {
                return data.cbegin() + t_hint;
            } else {
                return find(s);
            }
        } else {
            if (data.size() > t_hint && comparator(s, data[t_hint].first)) {
                return data.cbegin() + t_hint;
            } else {
                return find(s);
            }
        }
    }

    std::size_t size() const noexcept { return data.size(); }

    bool empty() const noexcept { return data.empty(); }

    iterator begin() noexcept { return data.begin(); }
    const_iterator begin() const noexcept { return data.begin(); }
    iterator end() noexcept { return data.end(); }
    const_iterator end() const noexcept { return data.end(); }

    Value &operator[](const Key &s) {
        auto itr = find(s);
        if (itr != data.end()) {
            return itr->second;
        } else {
            grow();
            return data.emplace_back(s, Value()).second;
        }
    }

    Value &at_index(std::size_t idx) noexcept { return data[idx].second; }
    const Value &at_index(std::size_t idx) const noexcept {
        return data[idx].second;
    }

    Value &at(const Key &s) {
        auto itr = find(s);
        if (itr != data.end()) {
            return itr->second;
        } else {
            throw std::out_of_range("Unknown key: " + s);
        }
    }

    const Value &at(const Key &s) const {
        auto itr = find(s);
        if (itr != data.end()) {
            return itr->second;
        } else {
            throw std::out_of_range("Unknown key: " + s);
        }
    }

    template <typename M>
    std::pair<iterator, bool> insert_or_assign(const Key &key, M &&m) {
        if (auto itr = find(key); itr != data.end()) {
            itr->second = std::forward<M>(m);
            return {itr, false};
        } else {
            grow();
            return {data.emplace(data.end(), key, std::forward<M>(m)), true};
        }
    }

    std::pair<iterator, bool> insert(value_type value) {
        if (auto itr = find(value.first); itr != data.end()) {
            return {itr, false};
        } else {
            grow();
            return {data.insert(data.end(), std::move(value)), true};
        }
    }

    template <typename Itr>
    void assign(Itr first, Itr last) {
        data.assign(first, last);
    }

    void grow() {
        if (data.capacity() == data.size()) {
            data.reserve(data.size() + 2);
        }
    }

private:
    std::vector<value_type> data;
    Comparator comparator;
};

#endif  // ATOM_TYPE_FLATMAP_HPP
