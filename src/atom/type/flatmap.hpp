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
#include <concepts>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace atom::type {

/**
 * @brief A flat map implementation using a sorted vector.
 *
 * @tparam Key The type of the keys.
 * @tparam Value The type of the values.
 * @tparam Comparator The type of the comparator used to compare keys.
 */
template <typename Key, typename Value, typename Comparator = std::equal_to<>>
    requires std::predicate<Comparator, Key, Key>
class QuickFlatMap {
public:
    using value_type = std::pair<Key, Value>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    /**
     * @brief Default constructor.
     */
    QuickFlatMap() = default;

    /**
     * @brief Finds an element with the specified key.
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return An iterator to the element, or end() if not found.
     */
    template <typename Lookup>
    auto find(const Lookup &s) noexcept -> iterator {
        return std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
    }

    /**
     * @brief Finds an element with the specified key (const version).
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return A const_iterator to the element, or end() if not found.
     */
    template <typename Lookup>
    auto find(const Lookup &s) const noexcept -> const_iterator {
        return std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
    }

    /**
     * @brief Finds an element with the specified key, using a hint.
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @param t_hint A hint for the position of the element.
     * @return A const_iterator to the element, or end() if not found.
     */
    template <typename Lookup>
    auto find(const Lookup &s,
              std::size_t t_hint) const noexcept -> const_iterator {
        if (data_.size() > t_hint && comparator_(data_[t_hint].first, s)) {
            return data_.cbegin() + t_hint;
        }
        return find(s);
    }

    /**
     * @brief Returns the number of elements in the map.
     *
     * @return The number of elements.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return data_.size();
    }

    /**
     * @brief Checks if the map is empty.
     *
     * @return True if the map is empty, false otherwise.
     */
    [[nodiscard]] auto empty() const noexcept -> bool { return data_.empty(); }

    /**
     * @brief Returns an iterator to the beginning of the map.
     *
     * @return An iterator to the beginning.
     */
    auto begin() noexcept -> iterator { return data_.begin(); }

    /**
     * @brief Returns a const_iterator to the beginning of the map.
     *
     * @return A const_iterator to the beginning.
     */
    auto begin() const noexcept -> const_iterator { return data_.begin(); }

    /**
     * @brief Returns an iterator to the end of the map.
     *
     * @return An iterator to the end.
     */
    auto end() noexcept -> iterator { return data_.end(); }

    /**
     * @brief Returns a const_iterator to the end of the map.
     *
     * @return A const_iterator to the end.
     */
    auto end() const noexcept -> const_iterator { return data_.end(); }

    /**
     * @brief Accesses the value associated with the specified key.
     *
     * @param s The key to look up.
     * @return A reference to the value associated with the key.
     */
    auto operator[](const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        grow();
        return data_.emplace_back(s, Value()).second;
    }

    /**
     * @brief Accesses the value at the specified index.
     *
     * @param idx The index.
     * @return A reference to the value at the index.
     */
    auto atIndex(std::size_t idx) noexcept -> Value & {
        return data_[idx].second;
    }

    /**
     * @brief Accesses the value at the specified index (const version).
     *
     * @param idx The index.
     * @return A const reference to the value at the index.
     */
    auto atIndex(std::size_t idx) const noexcept -> const Value & {
        return data_[idx].second;
    }

    /**
     * @brief Accesses the value associated with the specified key.
     *
     * @param s The key to look up.
     * @return A reference to the value associated with the key.
     * @throws std::out_of_range if the key is not found.
     */
    auto at(const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    /**
     * @brief Accesses the value associated with the specified key (const
     * version).
     *
     * @param s The key to look up.
     * @return A const reference to the value associated with the key.
     * @throws std::out_of_range if the key is not found.
     */
    const Value &at(const Key &s) const {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    /**
     * @brief Inserts or assigns a value to the specified key.
     *
     * @tparam M The type of the value to insert or assign.
     * @param key The key to insert or assign.
     * @param m The value to insert or assign.
     * @return A pair consisting of an iterator to the inserted or assigned
     * element, and a bool denoting whether the insertion took place.
     */
    template <typename M>
    auto insertOrAssign(const Key &key, M &&m) -> std::pair<iterator, bool> {
        if (auto itr = find(key); itr != data_.end()) {
            itr->second = std::forward<M>(m);
            return {itr, false};
        }
        grow();
        return {data_.emplace(data_.end(), key, std::forward<M>(m)), true};
    }

    /**
     * @brief Inserts a value into the map.
     *
     * @param value The value to insert.
     * @return A pair consisting of an iterator to the inserted element, and a
     * bool denoting whether the insertion took place.
     */
    auto insert(value_type value) -> std::pair<iterator, bool> {
        if (auto itr = find(value.first); itr != data_.end()) {
            return {itr, false};
        }
        grow();
        return {data_.insert(data_.end(), std::move(value)), true};
    }

    /**
     * @brief Assigns a range of values to the map.
     *
     * @tparam Itr The type of the iterator.
     * @param first The beginning of the range.
     * @param last The end of the range.
     */
    template <typename Itr>
    void assign(Itr first, Itr last) {
        data_.assign(first, last);
    }

    /**
     * @brief Grows the capacity of the underlying vector.
     */
    void grow() {
        if (data_.capacity() == data_.size()) {
            data_.reserve(data_.size() + 2);
        }
    }

    /**
     * @brief Checks if the map contains the specified key.
     *
     * @param s The key to check.
     * @return True if the key is found, false otherwise.
     */
    auto contains(const Key &s) const noexcept -> bool {
        return find(s) != data_.end();
    }

    /**
     * @brief Erases the element with the specified key.
     *
     * @param s The key to erase.
     * @return True if the element was erased, false otherwise.
     */
    auto erase(const Key &s) -> bool {
        if (auto itr = find(s); itr != data_.end()) {
            data_.erase(itr);
            return true;
        }
        return false;
    }

private:
    std::vector<value_type> data_;  ///< The underlying data storage.
    Comparator comparator_;         ///< The comparator used to compare keys.
};

/**
 * @brief A flat multi-map implementation using a sorted vector.
 *
 * @tparam Key The type of the keys.
 * @tparam Value The type of the values.
 * @tparam Comparator The type of the comparator used to compare keys.
 */
template <typename Key, typename Value, typename Comparator = std::equal_to<>>
    requires std::predicate<Comparator, Key, Key>
class QuickFlatMultiMap {
public:
    using value_type = std::pair<Key, Value>;
    using iterator = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    /**
     * @brief Default constructor.
     */
    QuickFlatMultiMap() = default;

    /**
     * @brief Finds an element with the specified key.
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return An iterator to the element, or end() if not found.
     */
    template <typename Lookup>
    auto find(const Lookup &s) noexcept -> iterator {
        return std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
    }

    /**
     * @brief Finds an element with the specified key (const version).
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return A const_iterator to the element, or end() if not found.
     */
    template <typename Lookup>
    auto find(const Lookup &s) const noexcept -> const_iterator {
        return std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
    }

    /**
     * @brief Finds the range of elements with the specified key.
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return A pair of iterators to the range of elements.
     */
    template <typename Lookup>
    auto equalRange(const Lookup &s) noexcept -> std::pair<iterator, iterator> {
        auto lower = std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
        if (lower == data_.end()) {
            return {lower, lower};
        }

        auto upper = std::ranges::find_if_not(
            lower, data_.end(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        return {lower, upper};
    }

    /**
     * @brief Finds the range of elements with the specified key (const
     * version).
     *
     * @tparam Lookup The type of the key to look up.
     * @param s The key to look up.
     * @return A pair of const_iterators to the range of elements.
     */
    template <typename Lookup>
    auto equalRange(const Lookup &s) const noexcept
        -> std::pair<const_iterator, const_iterator> {
        auto lower = std::ranges::find_if(data_, [&s, this](const auto &d) {
            return comparator_(d.first, s);
        });
        if (lower == data_.cend()) {
            return {lower, lower};
        }

        auto upper = std::ranges::find_if_not(
            lower, data_.cend(),
            [&s, this](const auto &d) { return comparator_(d.first, s); });
        return {lower, upper};
    }

    /**
     * @brief Returns the number of elements in the map.
     *
     * @return The number of elements.
     */
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return data_.size();
    }

    /**
     * @brief Checks if the map is empty.
     *
     * @return True if the map is empty, false otherwise.
     */
    [[nodiscard]] auto empty() const noexcept -> bool { return data_.empty(); }

    /**
     * @brief Returns an iterator to the beginning of the map.
     *
     * @return An iterator to the beginning.
     */
    auto begin() noexcept -> iterator { return data_.begin(); }

    /**
     * @brief Returns a const_iterator to the beginning of the map.
     *
     * @return A const_iterator to the beginning.
     */
    auto begin() const noexcept -> const_iterator { return data_.begin(); }

    /**
     * @brief Returns an iterator to the end of the map.
     *
     * @return An iterator to the end.
     */
    auto end() noexcept -> iterator { return data_.end(); }

    /**
     * @brief Returns a const_iterator to the end of the map.
     *
     * @return A const_iterator to the end.
     */
    auto end() const noexcept -> const_iterator { return data_.end(); }

    /**
     * @brief Accesses the value associated with the specified key.
     *
     * @param s The key to look up.
     * @return A reference to the value associated with the key.
     */
    auto operator[](const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        grow();
        return data_.emplace_back(s, Value()).second;
    }

    /**
     * @brief Accesses the value at the specified index.
     *
     * @param idx The index.
     * @return A reference to the value at the index.
     */
    auto atIndex(std::size_t idx) noexcept -> Value & {
        return data_[idx].second;
    }

    /**
     * @brief Accesses the value at the specified index (const version).
     *
     * @param idx The index.
     * @return A const reference to the value at the index.
     */
    auto atIndex(std::size_t idx) const noexcept -> const Value & {
        return data_[idx].second;
    }

    /**
     * @brief Accesses the value associated with the specified key.
     *
     * @param s The key to look up.
     * @return A reference to the value associated with the key.
     * @throws std::out_of_range if the key is not found.
     */
    auto at(const Key &s) -> Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    /**
     * @brief Accesses the value associated with the specified key (const
     * version).
     *
     * @param s The key to look up.
     * @return A const reference to the value associated with the key.
     * @throws std::out_of_range if the key is not found.
     */
    auto at(const Key &s) const -> const Value & {
        auto itr = find(s);
        if (itr != data_.end()) {
            return itr->second;
        }
        throw std::out_of_range("Unknown key: " + s);
    }

    /**
     * @brief Inserts a value into the map.
     *
     * @param value The value to insert.
     * @return A pair consisting of an iterator to the inserted element, and a
     * bool denoting whether the insertion took place.
     */
    auto insert(value_type value) -> std::pair<iterator, bool> {
        grow();
        return {data_.insert(data_.end(), std::move(value)), true};
    }

    /**
     * @brief Assigns a range of values to the map.
     *
     * @tparam Itr The type of the iterator.
     * @param first The beginning of the range.
     * @param last The end of the range.
     */
    template <typename Itr>
    void assign(Itr first, Itr last) {
        data_.assign(first, last);
    }

    /**
     * @brief Grows the capacity of the underlying vector.
     */
    void grow() {
        if (data_.capacity() == data_.size()) {
            data_.reserve(data_.size() + 2);
        }
    }

    /**
     * @brief Counts the number of elements with the specified key.
     *
     * @param s The key to count.
     * @return The number of elements with the key.
     */
    auto count(const Key &s) const -> size_t {
        auto [lower, upper] = equalRange(s);
        return std::distance(lower, upper);
    }

    /**
     * @brief Checks if the map contains the specified key.
     *
     * @param s The key to check.
     * @return True if the key is found, false otherwise.
     */
    auto contains(const Key &s) const noexcept -> bool {
        return find(s) != data_.end();
    }

    /**
     * @brief Erases the element with the specified key.
     *
     * @param s The key to erase.
     * @return True if the element was erased, false otherwise.
     */
    auto erase(const Key &s) -> bool {
        auto [lower, upper] = equalRange(s);
        if (lower != upper) {
            data_.erase(lower, upper);
            return true;
        }
        return false;
    }

private:
    std::vector<value_type> data_;
    Comparator comparator_;
};
}  // namespace atom::type

#endif  // ATOM_TYPE_FLATMAP_HPP
