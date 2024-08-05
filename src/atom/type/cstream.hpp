/*
 * Copyright (C) 2016 Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

template <typename C>
struct ContainerAccumulate {
    using value_type = typename C::value_type;
    auto operator()(C& dest, const C& source) const -> C& {
        std::ranges::copy(source, std::back_inserter(dest));
        return dest;
    }
};

template <typename V>
struct identity {
    auto operator()(const V& v) const -> V { return v; }
};

template <typename C>
class cstream {
public:
    using value_type = typename C::value_type;
    using it_type = decltype(std::begin(C{}));

    explicit cstream(C& c) : container_ref_{c} {}
    explicit cstream(C&& c) : moved_{std::move(c)}, container_ref_{moved_} {}

    auto getRef() -> C& { return container_ref_; }
    auto getMove() -> C&& { return std::move(container_ref_); }
    auto get() const -> C { return container_ref_; }

    explicit operator C&&() { return getMove(); }

    // Operations
    template <typename BinaryFunction = std::less<value_type>>
    auto sorted(const BinaryFunction& op = {}) -> cstream<C>& {
        std::ranges::sort(container_ref_, op);
        return *this;
    }

    template <typename T, typename UnaryFunction>
    auto transform(UnaryFunction transform_f) const -> cstream<T> {
        T dest;
        std::ranges::transform(container_ref_, std::back_inserter(dest),
                               transform_f);
        return cstream<T>(std::move(dest));
    }

    template <typename UnaryFunction>
    auto remove(UnaryFunction remove_f) -> cstream<C>& {
        std::erase_if(container_ref_, remove_f);
        return *this;
    }

    template <typename ValueType>
    auto erase(const ValueType& v) -> cstream<C>& {
        container_ref_.erase(v);
        return *this;
    }

    template <typename UnaryFunction>
    auto filter(UnaryFunction filter) -> cstream<C>& {
        return remove([&](const value_type& v) { return !filter(v); });
    }

    template <typename UnaryFunction>
    auto cpFilter(UnaryFunction filter) const -> cstream<C> {
        C c;
        std::ranges::copy_if(container_ref_, std::back_inserter(c), filter);
        return cstream<C>(std::move(c));
    }

    template <typename UnaryFunction = std::plus<value_type>>
    auto accumulate(value_type initial = {},
                    UnaryFunction op = {}) const -> value_type {
        return std::accumulate(std::begin(container_ref_),
                               std::end(container_ref_), initial, op);
    }

    template <typename UnaryFunction>
    auto forEach(UnaryFunction f) -> cstream<C>& {
        std::ranges::for_each(container_ref_, f);
        return *this;
    }

    template <typename UnaryFunction>
    auto all(UnaryFunction f) const -> bool {
        return std::ranges::all_of(container_ref_, f);
    }

    template <typename UnaryFunction>
    auto any(UnaryFunction f) const -> bool {
        return std::ranges::any_of(container_ref_, f);
    }

    template <typename UnaryFunction>
    auto none(UnaryFunction f) const -> bool {
        return std::ranges::none_of(container_ref_, f);
    }

    auto copy() const -> cstream<C> {
        C c(container_ref_);
        return cstream<C>{std::move(c)};
    }

    [[nodiscard]] auto size() const -> std::size_t {
        return container_ref_.size();
    }

    template <typename UnaryFunction>
    auto count(UnaryFunction f) const -> std::size_t {
        return std::ranges::count_if(container_ref_, f);
    }

    auto count(const value_type& v) const -> std::size_t {
        return std::ranges::count(container_ref_, v);
    }

    auto contains(const value_type& value) const -> bool {
        return any([&](const value_type& v) { return value == v; });
    }

    auto min() const -> value_type {
        return *std::ranges::min_element(container_ref_);
    }

    auto max() const -> value_type {
        return *std::ranges::max_element(container_ref_);
    }

    [[nodiscard]] auto mean() const -> double {
        return static_cast<double>(accumulate()) / static_cast<double>(size());
    }

    auto first() const -> std::optional<value_type> {
        if (container_ref_.empty())
            return std::nullopt;
        return {*std::begin(container_ref_)};
    }

    template <typename UnaryFunction>
    auto first(UnaryFunction f) const -> std::optional<value_type> {
        auto it = std::ranges::find_if(container_ref_, f);
        if (it == std::end(container_ref_))
            return std::nullopt;
        return {*it};
    }

private:
    C moved_;
    C& container_ref_;
};

template <typename C, typename Add = std::plus<C>>
struct JoinAccumulate {
    C separator;
    Add adder;
    auto operator()(C& dest, const C& source) const -> C {
        return dest.empty() ? source : adder(adder(dest, separator), source);
    }
};

template <typename A, typename B>
struct Pair {
    static auto first(const std::pair<A, B>& p) -> A { return p.first; }
    static auto second(const std::pair<A, B>& p) -> B { return p.second; }
};

template <typename T>
auto makeStream(T& t) -> cstream<T> {
    return cstream<T>{t};
}

template <typename T>
auto makeStream(T&& t) -> cstream<T> {
    return cstream<T>{std::forward<T>(t)};
}

template <typename T>
auto makeStreamCopy(const T& t) -> cstream<T> {
    return cstream<T>{T{t}};
}

template <typename N, typename T = N>
auto cpstream(const T* t, std::size_t size) -> cstream<std::vector<N>> {
    std::vector<N> data(size);
    std::ranges::copy_n(t, size, data.begin());
    return make_stream(std::move(data));
}

}  // namespace atom::type

#endif  // ATOM_TYPE_CONTAINERS_STREAMS_HPP
