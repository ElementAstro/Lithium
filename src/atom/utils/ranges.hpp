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
#include <array>
#include <chrono>
#include <concepts>
#include <coroutine>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

namespace atom::utils {
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

template <typename T>
struct generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value_;
        std::suspend_always yield_value(T value) noexcept {
            value_ = value;
            return {};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        generator get_return_object() noexcept {
            return generator{handle_type::from_promise(*this)};
        }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    handle_type handle_;
    explicit generator(handle_type handle) : handle_(handle) {}
    ~generator() {
        if (handle_)
            handle_.destroy();
    }
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;
    generator(generator&& other) noexcept
        : handle_(std::exchange(other.handle_, {})) {}
    generator& operator=(generator&& other) noexcept {
        if (std::addressof(other) != this) {
            if (handle_)
                handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    struct iterator {
        handle_type handle_;
        bool done_ = false;

        iterator() = default;
        iterator(handle_type handle) : handle_(handle) { ++(*this); }

        iterator& operator++() {
            handle_.resume();
            done_ = handle_.done();
            return *this;
        }

        T operator*() const { return handle_.promise().value_; }
        bool operator==(std::default_sentinel_t) const { return done_; }
    };

    iterator begin() { return iterator{handle_}; }
    std::default_sentinel_t end() { return {}; }
};

// 特化版本以支持引用类型
template <typename T>
struct generator<T&> {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T* value_;
        std::suspend_always yield_value(T& value) noexcept {
            value_ = std::addressof(value);
            return {};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        generator get_return_object() noexcept {
            return generator{handle_type::from_promise(*this)};
        }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    handle_type handle_;
    explicit generator(handle_type handle) : handle_(handle) {}
    ~generator() {
        if (handle_)
            handle_.destroy();
    }
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;
    generator(generator&& other) noexcept
        : handle_(std::exchange(other.handle_, {})) {}
    generator& operator=(generator&& other) noexcept {
        if (std::addressof(other) != this) {
            if (handle_)
                handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    struct iterator {
        handle_type handle_;
        bool done_ = false;

        iterator() = default;
        iterator(handle_type handle) : handle_(handle) { ++(*this); }

        iterator& operator++() {
            handle_.resume();
            done_ = handle_.done();
            return *this;
        }

        T& operator*() const { return *handle_.promise().value_; }
        bool operator==(std::default_sentinel_t) const { return done_; }
    };

    iterator begin() { return iterator{handle_}; }
    std::default_sentinel_t end() { return {}; }
};

struct merge_view_impl {
    template <std::ranges::input_range R1, std::ranges::input_range R2>
        requires std::common_reference_with<std::ranges::range_reference_t<R1>,
                                            std::ranges::range_reference_t<R2>>
    generator<std::common_reference_t<std::ranges::range_reference_t<R1>,
                                      std::ranges::range_reference_t<R2>>>
    operator()(R1&& r1, R2&& r2) {
        auto it1 = std::ranges::begin(r1);
        auto it2 = std::ranges::begin(r2);
        auto end1 = std::ranges::end(r1);
        auto end2 = std::ranges::end(r2);

        while (it1 != end1 && it2 != end2) {
            if (*it1 <= *it2) {
                co_yield *it1;
                ++it1;
            } else {
                co_yield *it2;
                ++it2;
            }
        }

        while (it1 != end1) {
            co_yield *it1;
            ++it1;
        }

        while (it2 != end2) {
            co_yield *it2;
            ++it2;
        }
    }
} merge_view;

struct zip_view_impl {
    template <std::ranges::input_range... Rs>
    generator<std::tuple<std::ranges::range_value_t<Rs>...>> operator()(
        Rs&&... ranges) {
        auto its = std::tuple{std::ranges::begin(ranges)...};
        auto ends = std::tuple{std::ranges::end(ranges)...};

        while (check_iterators(its, ends, std::index_sequence_for<Rs...>{})) {
            co_yield get_values(its, std::index_sequence_for<Rs...>{});
            increment_iterators(its, std::index_sequence_for<Rs...>{});
        }
    }

private:
    template <typename Tuple, size_t... Is>
    static bool check_iterators(const Tuple& its, const Tuple& ends,
                                std::index_sequence<Is...>) {
        return ((std::get<Is>(its) != std::get<Is>(ends)) && ...);
    }

    template <typename Tuple, size_t... Is>
    static auto get_values(const Tuple& its, std::index_sequence<Is...>) {
        return std::tuple{*std::get<Is>(its)...};
    }

    template <typename Tuple, size_t... Is>
    static void increment_iterators(Tuple& its, std::index_sequence<Is...>) {
        (++std::get<Is>(its), ...);
    }
} zip_view;

struct chunk_view_impl {
    template <std::ranges::input_range R>
    generator<std::vector<std::ranges::range_value_t<R>>> operator()(
        R&& range, size_t chunk_size) {
        std::vector<std::ranges::range_value_t<R>> chunk;
        chunk.reserve(chunk_size);

        for (auto&& elem : range) {
            chunk.push_back(std::forward<decltype(elem)>(elem));
            if (chunk.size() == chunk_size) {
                co_yield std::move(chunk);
                chunk.clear();
            }
        }

        if (!chunk.empty()) {
            co_yield std::move(chunk);
        }
    }
} chunk_view;

struct filter_view_impl {
    template <std::ranges::input_range R,
              std::invocable<std::ranges::range_reference_t<R>> Pred>
    generator<std::ranges::range_reference_t<R>> operator()(R&& range,
                                                            Pred pred) {
        for (auto&& elem : range) {
            if (pred(elem)) {
                co_yield elem;
            }
        }
    }
} filter_view;

struct transform_view_impl {
    template <std::ranges::input_range R,
              std::invocable<std::ranges::range_reference_t<R>> F>
    generator<std::invoke_result_t<F, std::ranges::range_reference_t<R>>>
    operator()(R&& range, F f) {
        for (auto&& elem : range) {
            co_yield f(elem);
        }
    }
} transform_view;

struct adjacent_view_impl {
    template <std::ranges::forward_range R>
    generator<
        std::pair<std::ranges::range_value_t<R>, std::ranges::range_value_t<R>>>
    operator()(R&& range) {
        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);
        if (it == end)
            co_return;

        auto prev = *it++;
        while (it != end) {
            co_yield {prev, *it};
            prev = *it++;
        }
    }
} adjacent_view;
}  // namespace atom::utils

#endif
