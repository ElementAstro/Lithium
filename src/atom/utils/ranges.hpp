/*
 * ranges.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-12

Description: Some ranges functions for C++20

**************************************************/

#ifndef ATOM_EXPERIMENT_RANGES_HPP
#define ATOM_EXPERIMENT_RANGES_HPP

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <thread>
#include <vector>

/**
 * @brief Filters elements in a range satisfying a predicate and transforms them
 * using a function.
 *
 * @tparam Range The type of the range.
 * @tparam Pred The type of the predicate.
 * @tparam Func The type of the function.
 * @param range The input range.
 * @param pred The predicate function.
 * @param func The transformation function.
 * @return The transformed range.
 *
 * @usage
 * std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
 * auto result = filter_and_transform(
 *     numbers, [](int x) { return x % 2 == 0; }, [](int x) { return x * 2; });
 * for (auto x : result) {
 *     std::cout << x << " ";
 * }
 */
template <typename Range, typename Pred, typename Func>
auto filter_and_transform(Range&& range, Pred&& pred, Func&& func) {
    return std::forward<Range>(range) |
           std::views::filter(std::forward<Pred>(pred)) |
           std::views::transform(std::forward<Func>(func));
}

/**
 * @brief Finds an element in a range.
 *
 * @tparam Range The type of the range.
 * @tparam T The type of the value to find.
 * @param range The input range.
 * @param value The value to find.
 * @return An optional containing the found value, or nullopt if not found.
 *
 * @usage
 * std::vector<int> numbers = {1, 2, 3, 4, 5};
 * auto result = find_element(numbers, 3);
 * if (result) {
 *     std::cout << "Found: " << *result << std::endl;
 * } else {
 *     std::cout << "Element not found" << std::endl;
 * }
 */
template <typename Range, typename T>
auto find_element(Range&& range, const T& value) {
    auto it = std::ranges::find(std::forward<Range>(range), value);
    return it != std::ranges::end(range)
               ? std::optional<std::remove_cvref_t<decltype(*it)>>{*it}
               : std::nullopt;
}

/**
 * @brief Groups elements in a range by a key and aggregates values using an
 * aggregator function.
 *
 * @tparam Range The type of the range.
 * @tparam KeySelector The type of the key selector function.
 * @tparam Aggregator The type of the aggregator function.
 * @param range The input range.
 * @param key_selector The function to extract keys from elements.
 * @param aggregator The function to aggregate values.
 * @return A map containing grouped and aggregated results.
 *
 * @usage
 * std::vector<std::pair<std::string, int>> data = {{"apple", 2},
 *                                                  {"banana", 3},
 *                                                  {"apple", 1},
 *                                                  {"cherry", 4},
 *                                                  {"banana", 1}};
 * auto fruit_counts = group_and_aggregate(
 *     data, [](const auto& pair) { return pair.first; },
 *     [](const auto& pair) { return pair.second; });
 * for (const auto& [fruit, count] : fruit_counts) {
 *     std::cout << fruit << ": " << count << std::endl;
 * }
 */
template <typename Range, typename KeySelector, typename Aggregator>
auto group_and_aggregate(Range&& range, KeySelector&& key_selector,
                         Aggregator&& aggregator) {
    using Key = std::invoke_result_t<KeySelector,
                                     std::ranges::range_reference_t<Range>>;
    using Value =
        std::invoke_result_t<Aggregator, std::ranges::range_reference_t<Range>>;

    std::map<Key, Value> result;
    for (auto&& item : std::forward<Range>(range)) {
        Key key = std::invoke(std::forward<KeySelector>(key_selector), item);
        Value value = std::invoke(std::forward<Aggregator>(aggregator), item);
        result[key] += value;
    }
    return result;
}

/**
 * @brief Drops the first n elements from a range.
 *
 * @tparam Range The type of the range.
 * @param range The input range.
 * @param n The number of elements to drop.
 * @return The range with the first n elements dropped.
 *
 * @usage
 * auto remaining_numbers = drop(numbers, 3);
 * for (auto x : remaining_numbers) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Range>
auto drop(Range&& range, std::ranges::range_difference_t<Range> n) {
    return std::forward<Range>(range) | std::views::drop(n);
}

/**
 * @brief Takes the first n elements from a range.
 *
 * @tparam Range The type of the range.
 * @param range The input range.
 * @param n The number of elements to take.
 * @return The range with the first n elements taken.
 *
 * @usage
 * auto first_three = take(numbers, 3);
 * for (auto x : first_three) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Range>
auto take(Range&& range, std::ranges::range_difference_t<Range> n) {
    return std::forward<Range>(range) | std::views::take(n);
}

/**
 * @brief Takes elements from a range while a predicate is true.
 *
 * @tparam Range The type of the range.
 * @tparam Pred The type of the predicate function.
 * @param range The input range.
 * @param pred The predicate function.
 * @return The range with elements taken while the predicate is true.
 *
 * @usage
 * auto less_than_six = take_while(numbers, [](int x) { return x < 6; });
 * for (auto x : less_than_six) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Range, typename Pred>
auto take_while(Range&& range, Pred&& pred) {
    return std::forward<Range>(range) |
           std::views::take_while(std::forward<Pred>(pred));
}

/**
 * @brief Drops elements from a range while a predicate is true.
 *
 * @tparam Range The type of the range.
 * @tparam Pred The type of the predicate function.
 * @param range The input range.
 * @param pred The predicate function.
 * @return The range with elements dropped while the predicate is true.
 *
 * @usage
 * auto more_than_two = drop_while(numbers, [](int x) { return x <= 2; });
 * for (auto x : more_than_two) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Range, typename Pred>
auto drop_while(Range&& range, Pred&& pred) {
    return std::forward<Range>(range) |
           std::views::drop_while(std::forward<Pred>(pred));
}

/**
 * @brief Reverses the elements in a range.
 *
 * @tparam Range The type of the range.
 * @param range The input range.
 * @return The reversed range.
 *
 * @usage
 * auto reversed_numbers = reverse(numbers);
 * for (auto x : reversed_numbers) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Range>
auto reverse(Range&& range) {
    return std::forward<Range>(range) | std::views::reverse;
}

/**
 * @brief Accumulates the elements in a range using a binary operation.
 *
 * @tparam Range The type of the range.
 * @tparam T The type of the initial value.
 * @tparam BinaryOp The type of the binary operation function.
 * @param range The input range.
 * @param init The initial value for accumulation.
 * @param op The binary operation function.
 * @return The result of the accumulation.
 *
 * @usage
 * auto sum = accumulate(numbers, 0, std::plus<>{});
 * std::cout << "Sum: " << sum << std::endl;
 */
template <typename Range, typename T, typename BinaryOp>
auto accumulate(Range&& range, T init, BinaryOp&& op) {
    return std::accumulate(std::begin(range), std::end(range), std::move(init),
                           std::forward<BinaryOp>(op));
}

/**
 * @brief Slices a range into a new range.
 *
 * @tparam Iterator The type of the iterator.
 * @param begin The beginning of the range.
 * @param end The end of the range.
 * @param start The starting index of the slice.
 * @param length The length of the slice.
 * @return A new range containing the sliced elements.
 *
 * @usage
 * auto sliced_numbers = Slice(numbers.begin(), numbers.end(), 2, 4);
 * for (auto x : sliced_numbers) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Iterator>
std::vector<typename Iterator::value_type> Slice(Iterator begin, Iterator end,
                                                 size_t start, size_t length) {
    std::vector<typename Iterator::value_type> result;
    if (start >= std::distance(begin, end)) {
        return result;
    }
    std::advance(begin, start);
    auto it = begin;
    for (size_t i = 0; i < length && it != end; ++i, ++it) {
        result.push_back(*it);
    }
    return result;
}

/**
 * @brief Slices a container into a new container.
 *
 * @tparam Container The type of the container.
 * @tparam Index The type of the index.
 * @param c The input container.
 * @param start The starting index of the slice.
 * @param end The ending index of the slice.
 * @return A new container containing the sliced elements.
 *
 * @usage
 * auto sliced_numbers = slice(numbers, 2, 4);
 * for (auto x : sliced_numbers) {
 *     std::cout << x << " ";
 * }
 * std::cout << std::endl;
 */
template <typename Container, typename Index>
auto slice(Container& c, Index start, Index end) {
    auto first = std::begin(c) + start;
    auto last = (end == std::numeric_limits<Index>::max())
                    ? std::end(c)
                    : std::begin(c) + end;

    return std::vector<typename Container::value_type>(first, last);
}

#endif