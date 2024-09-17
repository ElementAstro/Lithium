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
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace atom::type {

/**
 * @brief A flat set implementation using a sorted vector.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type (default is
 * std::less<T>).
 */
template <typename T, typename Compare = std::less<T>>
    requires std::predicate<Compare, T, T>
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
    /**
     * @brief Default constructor.
     */
    FlatSet() = default;

    /**
     * @brief Destructor.
     */
    ~FlatSet() = default;

    /**
     * @brief Constructs a FlatSet with a custom comparator.
     *
     * @param comp The comparator to use.
     */
    explicit FlatSet(const Compare& comp) : m_comp_(comp) {}

    /**
     * @brief Constructs a FlatSet from a range of elements.
     *
     * @tparam InputIt The type of the input iterator.
     * @param first The beginning of the range.
     * @param last The end of the range.
     * @param comp The comparator to use (default is Compare()).
     */
    template <std::input_iterator InputIt>
    FlatSet(InputIt first, InputIt last, const Compare& comp = Compare())
        : m_data_(first, last), m_comp_(comp) {
        std::ranges::sort(m_data_, m_comp_);
        m_data_.erase(std::unique(m_data_.begin(), m_data_.end()),
                      m_data_.end());
    }

    /**
     * @brief Constructs a FlatSet from an initializer list.
     *
     * @param init The initializer list.
     * @param comp The comparator to use (default is Compare()).
     */
    FlatSet(std::initializer_list<T> init, const Compare& comp = Compare())
        : FlatSet(init.begin(), init.end(), comp) {}

    /**
     * @brief Copy constructor.
     *
     * @param other The other FlatSet to copy from.
     */
    FlatSet(const FlatSet& other) = default;

    /**
     * @brief Move constructor.
     *
     * @param other The other FlatSet to move from.
     */
    FlatSet(FlatSet&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     *
     * @param other The other FlatSet to copy from.
     * @return A reference to this FlatSet.
     */
    FlatSet& operator=(const FlatSet& other) = default;

    /**
     * @brief Move assignment operator.
     *
     * @param other The other FlatSet to move from.
     * @return A reference to this FlatSet.
     */
    FlatSet& operator=(FlatSet&& other) noexcept = default;

    /**
     * @brief Returns an iterator to the beginning.
     *
     * @return An iterator to the beginning.
     */
    iterator begin() noexcept { return m_data_.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     *
     * @return A const iterator to the beginning.
     */
    const_iterator begin() const noexcept { return m_data_.begin(); }

    /**
     * @brief Returns a const iterator to the beginning.
     *
     * @return A const iterator to the beginning.
     */
    const_iterator cbegin() const noexcept { return m_data_.cbegin(); }

    /**
     * @brief Returns an iterator to the end.
     *
     * @return An iterator to the end.
     */
    iterator end() noexcept { return m_data_.end(); }

    /**
     * @brief Returns a const iterator to the end.
     *
     * @return A const iterator to the end.
     */
    const_iterator end() const noexcept { return m_data_.end(); }

    /**
     * @brief Returns a const iterator to the end.
     *
     * @return A const iterator to the end.
     */
    const_iterator cend() const noexcept { return m_data_.cend(); }

    /**
     * @brief Returns a reverse iterator to the beginning.
     *
     * @return A reverse iterator to the beginning.
     */
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

    /**
     * @brief Returns a const reverse iterator to the beginning.
     *
     * @return A const reverse iterator to the beginning.
     */
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    /**
     * @brief Returns a const reverse iterator to the beginning.
     *
     * @return A const reverse iterator to the beginning.
     */
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    /**
     * @brief Returns a reverse iterator to the end.
     *
     * @return A reverse iterator to the end.
     */
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    /**
     * @brief Returns a const reverse iterator to the end.
     *
     * @return A const reverse iterator to the end.
     */
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    /**
     * @brief Returns a const reverse iterator to the end.
     *
     * @return A const reverse iterator to the end.
     */
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    /**
     * @brief Checks if the set is empty.
     *
     * @return True if the set is empty, false otherwise.
     */
    [[nodiscard]] auto empty() const noexcept -> bool {
        return m_data_.empty();
    }

    /**
     * @brief Returns the number of elements in the set.
     *
     * @return The number of elements in the set.
     */
    [[nodiscard]] auto size() const noexcept -> size_type {
        return m_data_.size();
    }

    /**
     * @brief Returns the maximum possible number of elements in the set.
     *
     * @return The maximum possible number of elements in the set.
     */
    [[nodiscard]] auto max_size() const noexcept -> size_type {
        return m_data_.max_size();
    }

    /**
     * @brief Clears the set.
     */
    void clear() noexcept { m_data_.clear(); }

    /**
     * @brief Inserts a value into the set.
     *
     * @param value The value to insert.
     * @return A pair consisting of an iterator to the inserted element (or to
     * the element that prevented the insertion) and a bool denoting whether the
     * insertion took place.
     */
    auto insert(const T& value) -> std::pair<iterator, bool> {
        auto data = std::ranges::lower_bound(m_data_, value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return {data, false};
        }
        return {m_data_.insert(data, value), true};
    }

    /**
     * @brief Inserts a value into the set.
     *
     * @param value The value to insert.
     * @return A pair consisting of an iterator to the inserted element (or to
     * the element that prevented the insertion) and a bool denoting whether the
     * insertion took place.
     */
    auto insert(T&& value) -> std::pair<iterator, bool> {
        auto data = std::ranges::lower_bound(m_data_, value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return {data, false};
        }
        return {m_data_.insert(data, std::move(value)), true};
    }

    /**
     * @brief Inserts a value into the set with a hint.
     *
     * @param hint An iterator to the position before which the value will be
     * inserted.
     * @param value The value to insert.
     * @return An iterator to the inserted element.
     */
    auto insert(const_iterator hint, const T& value) -> iterator {
        if (hint == m_data_.end() || m_comp_(value, *hint)) {
            if (hint == m_data_.begin() || m_comp_(*(hint - 1), value)) {
                return m_data_.insert(hint, value);
            }
            return insert(value).first;
        }
        return hint;
    }

    /**
     * @brief Inserts a value into the set with a hint.
     *
     * @param hint An iterator to the position before which the value will be
     * inserted.
     * @param value The value to insert.
     * @return An iterator to the inserted element.
     */
    auto insert(const_iterator hint, T&& value) -> iterator {
        if (hint == m_data_.end() || m_comp_(value, *hint)) {
            if (hint == m_data_.begin() || m_comp_(*(hint - 1), value)) {
                return m_data_.insert(hint, std::move(value));
            }
            return insert(std::move(value)).first;
        }
        return hint;
    }

    /**
     * @brief Inserts a range of values into the set.
     *
     * @tparam InputIt The type of the input iterator.
     * @param first The beginning of the range.
     * @param last The end of the range.
     */
    template <std::input_iterator InputIt>
    void insert(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first++);
        }
    }

    /**
     * @brief Inserts values from an initializer list into the set.
     *
     * @param ilist The initializer list.
     */
    void insert(std::initializer_list<T> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    /**
     * @brief Constructs and inserts a value into the set.
     *
     * @tparam Args The types of the arguments.
     * @param args The arguments to construct the value.
     * @return A pair consisting of an iterator to the inserted element (or to
     * the element that prevented the insertion) and a bool denoting whether the
     * insertion took place.
     */
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return insert(T(std::forward<Args>(args)...));
    }

    /**
     * @brief Constructs and inserts a value into the set with a hint.
     *
     * @tparam Args The types of the arguments.
     * @param hint An iterator to the position before which the value will be
     * inserted.
     * @param args The arguments to construct the value.
     * @return An iterator to the inserted element.
     */
    template <typename... Args>
    auto emplace_hint(const_iterator hint, Args&&... args) -> iterator {
        return insert(hint, T(std::forward<Args>(args)...));
    }

    /**
     * @brief Erases an element from the set.
     *
     * @param pos An iterator to the element to erase.
     * @return An iterator to the element following the erased element.
     */
    auto erase(const_iterator pos) -> iterator { return m_data_.erase(pos); }

    /**
     * @brief Erases a range of elements from the set.
     *
     * @param first An iterator to the first element to erase.
     * @param last An iterator to the element following the last element to
     * erase.
     * @return An iterator to the element following the erased elements.
     */
    auto erase(const_iterator first, const_iterator last) -> iterator {
        return m_data_.erase(first, last);
    }

    /**
     * @brief Erases an element from the set by value.
     *
     * @param value The value to erase.
     * @return The number of elements erased.
     */
    auto erase(const T& value) -> size_type {
        auto data = find(value);
        if (data != m_data_.end()) {
            erase(data);
            return 1;
        }
        return 0;
    }

    /**
     * @brief Swaps the contents of this set with another set.
     *
     * @param other The other set to swap with.
     */
    void swap(FlatSet& other) noexcept {
        std::swap(m_data_, other.m_data_);
        std::swap(m_comp_, other.m_comp_);
    }

    /**
     * @brief Returns the number of elements matching a value.
     *
     * @param value The value to match.
     * @return The number of elements matching the value.
     */
    auto count(const T& value) const -> size_type {
        return find(value) != m_data_.end() ? 1 : 0;
    }

    /**
     * @brief Finds an element in the set.
     *
     * @param value The value to find.
     * @return An iterator to the element, or end() if not found.
     */
    auto find(const T& value) -> iterator {
        auto data = std::ranges::lower_bound(m_data_, value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return data;
        }
        return m_data_.end();
    }

    /**
     * @brief Finds an element in the set.
     *
     * @param value The value to find.
     * @return A const iterator to the element, or end() if not found.
     */
    auto find(const T& value) const -> const_iterator {
        auto data = std::ranges::lower_bound(m_data_, value, m_comp_);
        if (data != m_data_.end() && !m_comp_(value, *data)) {
            return data;
        }
        return m_data_.end();
    }

    /**
     * @brief Returns a range of elements matching a value.
     *
     * @param value The value to match.
     * @return A pair of iterators to the range of elements.
     */
    auto equal_range(const T& value) -> std::pair<iterator, iterator> {
        return std::ranges::equal_range(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns a range of elements matching a value.
     *
     * @param value The value to match.
     * @return A pair of const iterators to the range of elements.
     */
    auto equal_range(const T& value) const
        -> std::pair<const_iterator, const_iterator> {
        return std::ranges::equal_range(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns an iterator to the first element not less than the given
     * value.
     *
     * @param value The value to compare.
     * @return An iterator to the first element not less than the given value.
     */
    auto lower_bound(const T& value) -> iterator {
        return std::ranges::lower_bound(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns a const iterator to the first element not less than the
     * given value.
     *
     * @param value The value to compare.
     * @return A const iterator to the first element not less than the given
     * value.
     */
    auto lower_bound(const T& value) const -> const_iterator {
        return std::ranges::lower_bound(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns an iterator to the first element greater than the given
     * value.
     *
     * @param value The value to compare.
     * @return An iterator to the first element greater than the given value.
     */
    auto upper_bound(const T& value) -> iterator {
        return std::ranges::upper_bound(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns a const iterator to the first element greater than the
     * given value.
     *
     * @param value The value to compare.
     * @return A const iterator to the first element greater than the given
     * value.
     */
    auto upper_bound(const T& value) const -> const_iterator {
        return std::ranges::upper_bound(m_data_, value, m_comp_);
    }

    /**
     * @brief Returns the comparison function object.
     *
     * @return The comparison function object.
     */
    auto key_comp() const -> key_compare { return m_comp_; }

    /**
     * @brief Returns the comparison function object.
     *
     * @return The comparison function object.
     */
    auto value_comp() const -> value_compare { return m_comp_; }

    /**
     * @brief Checks if the set contains a value.
     *
     * @param value The value to check.
     * @return True if the value is found, false otherwise.
     */
    auto contains(const T& value) const -> bool {
        return find(value) != m_data_.end();
    }
};

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator==(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return lhs.size() == rhs.size() &&
           std::ranges::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator!=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(lhs == rhs);
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator<(const FlatSet<T, Compare>& lhs,
               const FlatSet<T, Compare>& rhs) -> bool {
    return std::ranges::lexicographical_compare(lhs.begin(), lhs.end(),
                                                rhs.begin(), rhs.end());
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator<=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(rhs < lhs);
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator>(const FlatSet<T, Compare>& lhs,
               const FlatSet<T, Compare>& rhs) -> bool {
    return rhs < lhs;
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
auto operator>=(const FlatSet<T, Compare>& lhs,
                const FlatSet<T, Compare>& rhs) -> bool {
    return !(lhs < rhs);
}

/**
 * @brief Swaps the contents of two FlatSets.
 *
 * @tparam T The type of elements.
 * @tparam Compare The comparison function object type.
 * @param lhs The first FlatSet.
 * @param rhs The second FlatSet.
 */
template <typename T, typename Compare>
void swap(FlatSet<T, Compare>& lhs,
          FlatSet<T, Compare>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

}  // namespace atom::type

#endif  // ATOM_TYPE_FLAT_SET_HPP