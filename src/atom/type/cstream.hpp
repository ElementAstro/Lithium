#ifndef ATOM_TYPE_CONTAINERS_STREAMS_HPP
#define ATOM_TYPE_CONTAINERS_STREAMS_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <numeric>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace atom::type {

/**
 * @brief A functor for accumulating containers.
 *
 * @tparam C The type of the container.
 */
template <typename C>
struct ContainerAccumulate {
    using value_type = typename C::value_type;

    /**
     * @brief Accumulates the source container into the destination container.
     *
     * @param dest The destination container.
     * @param source The source container.
     * @return C& The accumulated container.
     */
    auto operator()(C& dest, const C& source) const -> C& {
        std::ranges::copy(source, std::back_inserter(dest));
        return dest;
    }
};

/**
 * @brief A functor that returns the input value.
 *
 * @tparam V The type of the value.
 */
template <typename V>
struct identity {
    /**
     * @brief Returns the input value.
     *
     * @param v The input value.
     * @return V The same value.
     */
    auto operator()(const V& v) const -> V { return v; }
};

/**
 * @brief A stream-like class for performing operations on containers.
 *
 * @tparam C The type of the container.
 */
template <typename C>
class cstream {
public:
    using value_type = typename C::value_type;
    using it_type = decltype(std::begin(C{}));

    /**
     * @brief Constructs a cstream from a container reference.
     *
     * @param c The container reference.
     */
    explicit cstream(C& c) : container_ref_{c} {}

    /**
     * @brief Constructs a cstream from an rvalue container.
     *
     * @param c The rvalue container.
     */
    explicit cstream(C&& c) : moved_{std::move(c)}, container_ref_{moved_} {}

    /**
     * @brief Gets the reference to the container.
     *
     * @return C& The container reference.
     */
    auto getRef() -> C& { return container_ref_; }

    /**
     * @brief Moves the container out of the stream.
     *
     * @return C&& The moved container.
     */
    auto getMove() -> C&& { return std::move(container_ref_); }

    /**
     * @brief Gets a copy of the container.
     *
     * @return C The copied container.
     */
    auto get() const -> C { return container_ref_; }

    /**
     * @brief Converts the stream to an rvalue container.
     *
     * @return C&& The moved container.
     */
    explicit operator C&&() { return getMove(); }

    // Operations

    /**
     * @brief Sorts the container.
     *
     * @tparam BinaryFunction The type of the comparison function.
     * @param op The comparison function.
     * @return cstream<C>& The sorted stream.
     */
    template <typename BinaryFunction = std::less<value_type>>
    auto sorted(const BinaryFunction& op = {}) -> cstream<C>& {
        std::ranges::sort(container_ref_, op);
        return *this;
    }

    /**
     * @brief Transforms the container using a function.
     *
     * @tparam T The type of the destination container.
     * @tparam UnaryFunction The type of the transformation function.
     * @param transform_f The transformation function.
     * @return cstream<T> The transformed stream.
     */
    template <typename T, typename UnaryFunction>
    auto transform(UnaryFunction transform_f) const -> cstream<T> {
        T dest;
        std::ranges::transform(container_ref_, std::back_inserter(dest),
                               transform_f);
        return cstream<T>(std::move(dest));
    }

    /**
     * @brief Removes elements from the container based on a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param remove_f The predicate function.
     * @return cstream<C>& The stream with elements removed.
     */
    template <typename UnaryFunction>
    auto remove(UnaryFunction remove_f) -> cstream<C>& {
        std::erase_if(container_ref_, remove_f);
        return *this;
    }

    /**
     * @brief Erases a specific value from the container.
     *
     * @tparam ValueType The type of the value.
     * @param v The value to erase.
     * @return cstream<C>& The stream with the value erased.
     */
    template <typename ValueType>
    auto erase(const ValueType& v) -> cstream<C>& {
        container_ref_.erase(v);
        return *this;
    }

    /**
     * @brief Filters the container based on a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param filter The predicate function.
     * @return cstream<C>& The filtered stream.
     */
    template <typename UnaryFunction>
    auto filter(UnaryFunction filter) -> cstream<C>& {
        return remove([&](const value_type& v) { return !filter(v); });
    }

    /**
     * @brief Creates a copy of the container and filters it based on a
     * predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param filter The predicate function.
     * @return cstream<C> The filtered stream.
     */
    template <typename UnaryFunction>
    auto cpFilter(UnaryFunction filter) const -> cstream<C> {
        C c;
        std::ranges::copy_if(container_ref_, std::back_inserter(c), filter);
        return cstream<C>(std::move(c));
    }

    /**
     * @brief Accumulates the elements of the container using a binary function.
     *
     * @tparam UnaryFunction The type of the binary function.
     * @param initial The initial value.
     * @param op The binary function.
     * @return value_type The accumulated value.
     */
    template <typename UnaryFunction = std::plus<value_type>>
    auto accumulate(value_type initial = {},
                    UnaryFunction op = {}) const -> value_type {
        return std::accumulate(std::begin(container_ref_),
                               std::end(container_ref_), initial, op);
    }

    /**
     * @brief Applies a function to each element of the container.
     *
     * @tparam UnaryFunction The type of the function.
     * @param f The function to apply.
     * @return cstream<C>& The stream.
     */
    template <typename UnaryFunction>
    auto forEach(UnaryFunction f) -> cstream<C>& {
        std::ranges::for_each(container_ref_, f);
        return *this;
    }

    /**
     * @brief Checks if all elements satisfy a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param f The predicate function.
     * @return bool True if all elements satisfy the predicate, false otherwise.
     */
    template <typename UnaryFunction>
    auto all(UnaryFunction f) const -> bool {
        return std::ranges::all_of(container_ref_, f);
    }

    /**
     * @brief Checks if any element satisfies a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param f The predicate function.
     * @return bool True if any element satisfies the predicate, false
     * otherwise.
     */
    template <typename UnaryFunction>
    auto any(UnaryFunction f) const -> bool {
        return std::ranges::any_of(container_ref_, f);
    }

    /**
     * @brief Checks if no elements satisfy a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param f The predicate function.
     * @return bool True if no elements satisfy the predicate, false otherwise.
     */
    template <typename UnaryFunction>
    auto none(UnaryFunction f) const -> bool {
        return std::ranges::none_of(container_ref_, f);
    }

    /**
     * @brief Creates a copy of the container.
     *
     * @return cstream<C> The copied stream.
     */
    auto copy() const -> cstream<C> {
        C c(container_ref_);
        return cstream<C>{std::move(c)};
    }

    /**
     * @brief Gets the size of the container.
     *
     * @return std::size_t The size of the container.
     */
    [[nodiscard]] auto size() const -> std::size_t {
        return container_ref_.size();
    }

    /**
     * @brief Counts the number of elements that satisfy a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param f The predicate function.
     * @return std::size_t The count of elements that satisfy the predicate.
     */
    template <typename UnaryFunction>
    auto count(UnaryFunction f) const -> std::size_t {
        return std::ranges::count_if(container_ref_, f);
    }

    /**
     * @brief Counts the number of occurrences of a value.
     *
     * @param v The value to count.
     * @return std::size_t The count of the value.
     */
    auto count(const value_type& v) const -> std::size_t {
        return std::ranges::count(container_ref_, v);
    }

    /**
     * @brief Checks if the container contains a value.
     *
     * @param value The value to check.
     * @return bool True if the container contains the value, false otherwise.
     */
    auto contains(const value_type& value) const -> bool {
        return any([&](const value_type& v) { return value == v; });
    }

    /**
     * @brief Gets the minimum element in the container.
     *
     * @return value_type The minimum element.
     */
    auto min() const -> value_type {
        return *std::ranges::min_element(container_ref_);
    }

    /**
     * @brief Gets the maximum element in the container.
     *
     * @return value_type The maximum element.
     */
    auto max() const -> value_type {
        return *std::ranges::max_element(container_ref_);
    }

    /**
     * @brief Calculates the mean of the elements in the container.
     *
     * @return double The mean value.
     */
    [[nodiscard]] auto mean() const -> double {
        return static_cast<double>(accumulate()) / static_cast<double>(size());
    }

    /**
     * @brief Gets the first element in the container.
     *
     * @return std::optional<value_type> The first element, or std::nullopt if
     * the container is empty.
     */
    auto first() const -> std::optional<value_type> {
        if (container_ref_.empty())
            return std::nullopt;
        return {*std::begin(container_ref_)};
    }

    /**
     * @brief Gets the first element that satisfies a predicate.
     *
     * @tparam UnaryFunction The type of the predicate function.
     * @param f The predicate function.
     * @return std::optional<value_type> The first element that satisfies the
     * predicate, or std::nullopt if no such element exists.
     */
    template <typename UnaryFunction>
    auto first(UnaryFunction f) const -> std::optional<value_type> {
        auto it = std::ranges::find_if(container_ref_, f);
        if (it == std::end(container_ref_))
            return std::nullopt;
        return {*it};
    }

    // New functionalities

    /**
     * @brief Maps the elements of the container using a function.
     *
     * @tparam UnaryFunction The type of the mapping function.
     * @param f The mapping function.
     * @return cstream<C> The mapped stream.
     */
    template <typename UnaryFunction>
    auto map(UnaryFunction f) const -> cstream<C> {
        C c;
        std::ranges::transform(container_ref_, std::back_inserter(c), f);
        return cstream<C>(std::move(c));
    }

    /**
     * @brief Flat maps the elements of the container using a function.
     *
     * @tparam UnaryFunction The type of the flat mapping function.
     * @param f The flat mapping function.
     * @return cstream<C> The flat mapped stream.
     */
    template <typename UnaryFunction>
    auto flatMap(UnaryFunction f) const -> cstream<C> {
        C c;
        for (const auto& item : container_ref_) {
            auto subContainer = f(item);
            std::ranges::copy(subContainer, std::back_inserter(c));
        }
        return cstream<C>(std::move(c));
    }

    /**
     * @brief Removes duplicate elements from the container.
     *
     * @return cstream<C>& The stream with duplicates removed.
     */
    auto distinct() -> cstream<C>& {
        std::ranges::sort(container_ref_);
        auto last = std::unique(container_ref_.begin(), container_ref_.end());
        container_ref_.erase(last, container_ref_.end());
        return *this;
    }

    /**
     * @brief Reverses the elements of the container.
     *
     * @return cstream<C>& The stream with elements reversed.
     */
    auto reverse() -> cstream<C>& {
        std::ranges::reverse(container_ref_);
        return *this;
    }

private:
    C moved_;
    C& container_ref_;
};

/**
 * @brief A functor for joining containers with a separator.
 *
 * @tparam C The type of the container.
 * @tparam Add The type of the addition function.
 */
template <typename C, typename Add = std::plus<C>>
struct JoinAccumulate {
    C separator;  ///< The separator to use.
    Add adder;    ///< The addition function.

    /**
     * @brief Joins the source container into the destination container with a
     * separator.
     *
     * @param dest The destination container.
     * @param source The source container.
     * @return C The joined container.
     */
    auto operator()(C& dest, const C& source) const -> C {
        return dest.empty() ? source : adder(adder(dest, separator), source);
    }
};

/**
 * @brief A utility struct for working with pairs.
 *
 * @tparam A The type of the first element.
 * @tparam B The type of the second element.
 */
template <typename A, typename B>
struct Pair {
    /**
     * @brief Gets the first element of the pair.
     *
     * @param p The pair.
     * @return A The first element.
     */
    static auto first(const std::pair<A, B>& p) -> A { return p.first; }

    /**
     * @brief Gets the second element of the pair.
     *
     * @param p The pair.
     * @return B The second element.
     */
    static auto second(const std::pair<A, B>& p) -> B { return p.second; }
};

/**
 * @brief Creates a cstream from a container reference.
 *
 * @tparam T The type of the container.
 * @param t The container reference.
 * @return cstream<T> The created stream.
 */
template <typename T>
auto makeStream(T& t) -> cstream<T> {
    return cstream<T>{t};
}

/**
 * @brief Creates a cstream from a container rvalue.
 *
 * @tparam T The type of the container.
 * @param t The container rvalue.
 * @return cstream<T> The created stream.
 */
template <typename T>
auto makeStream(T&& t) -> cstream<T> {
    return cstream<T>{std::forward<T>(t)};
}

/**
 * @brief Creates a cstream from a container copy.
 *
 * @tparam T The type of the container.
 * @param t The container copy.
 * @return cstream<T> The created stream.
 */
template <typename T>
auto makeStreamCopy(const T& t) -> cstream<T> {
    return cstream<T>{T{t}};
}

/**
 * @brief Creates a cstream from a container.
 *
 * @tparam T The type of the container.
 * @param t The container.
 * @return cstream<T> The created stream.
 */
template <typename N, typename T = N>
auto cpstream(const T* t, std::size_t size) -> cstream<std::vector<N>> {
    std::vector<N> data(size);
    std::ranges::copy_n(t, size, data.begin());
    return makeStream(std::move(data));
}

}  // namespace atom::type

#endif  // ATOM_TYPE_CONTAINERS_STREAMS_HPP