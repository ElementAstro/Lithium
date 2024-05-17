/*
 * iter.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-26

Description: Some iterators

**************************************************/

#ifndef ATOM_EXPERIMENTAL_ITERATOR_HPP
#define ATOM_EXPERIMENTAL_ITERATOR_HPP

#include <algorithm>
#include <iterator>
#include <list>
#include <optional>
#include <utility>
#include <vector>

#if ENABLE_DEBUG
#include <iostream>
#endif

template <typename IteratorT>
class pointer_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = decltype(&*std::declval<IteratorT>());
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    IteratorT it_;

public:
    pointer_iterator() = default;

    explicit pointer_iterator(IteratorT it) : it_(std::move(it)) {}

    value_type operator*() const { return &*it_; }

    pointer_iterator& operator++() {
        ++it_;
        return *this;
    }

    pointer_iterator operator++(int) {
        pointer_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const pointer_iterator& other) const = default;
    bool operator!=(const pointer_iterator& other) const = default;
};

template <typename IteratorT>
auto make_pointer_range(IteratorT begin, IteratorT end) {
    return std::make_pair(pointer_iterator<IteratorT>(begin),
                          pointer_iterator<IteratorT>(end));
}

template <typename ContainerT>
void process_container(ContainerT& container) {
    auto beginIter = std::next(container.begin());
    auto endIter = std::prev(container.end());

    std::vector<std::optional<typename ContainerT::value_type*>> ptrs;
    auto ptrPair = make_pointer_range(beginIter, endIter);
    for (auto it = ptrPair.first; it != ptrPair.second; ++it) {
        ptrs.push_back(*it);
    }

    for (auto& ptrOpt : ptrs) {
        if (ptrOpt) {
            auto ptr = *ptrOpt;
#if ENABLE_DEBUG
            std::cout << "pointer addr: " << static_cast<const void*>(&ptr)
                      << '\n';
            std::cout << "point to: " << static_cast<const void*>(ptr) << '\n';
            std::cout << "value: " << *ptr << '\n';
#endif
            container.erase(
                std::find(container.begin(), container.end(), *ptr));
        }
    }
}

template <std::input_or_output_iterator I>
class early_inc_iterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    early_inc_iterator() = default;
    explicit early_inc_iterator(I x) : current(x) {}

    early_inc_iterator& operator++() {
        ++current;
        return *this;
    }

    early_inc_iterator operator++(int) {
        early_inc_iterator tmp = *this;
        ++current;
        return tmp;
    }

    friend bool operator==(const early_inc_iterator& x,
                           const early_inc_iterator& y) {
        return x.current == y.current;
    }

    friend bool operator!=(const early_inc_iterator& x,
                           const early_inc_iterator& y) {
        return !(x == y);
    }

    auto operator*() const { return *current; }

private:
    I current{};
};

template <std::input_or_output_iterator I>
early_inc_iterator<I> make_early_inc_iterator(I x) {
    return early_inc_iterator<I>(x);
}

template <typename IteratorT, typename FuncT>
class transform_iterator {
public:
    using iterator_category =
        typename std::iterator_traits<IteratorT>::iterator_category;
    using value_type = std::invoke_result_t<
        FuncT, typename std::iterator_traits<IteratorT>::reference>;
    using difference_type =
        typename std::iterator_traits<IteratorT>::difference_type;
    using pointer = value_type*;
    using reference = value_type;

private:
    IteratorT it_;
    FuncT func_;

public:
    transform_iterator() : it_(), func_() {}
    transform_iterator(IteratorT it, FuncT func) : it_(it), func_(func) {}

    reference operator*() const { return func_(*it_); }
    pointer operator->() const { return &(operator*()); }

    transform_iterator& operator++() {
        ++it_;
        return *this;
    }
    transform_iterator operator++(int) {
        transform_iterator tmp = *this;
        ++it_;
        return tmp;
    }

    bool operator==(const transform_iterator& other) const {
        return it_ == other.it_;
    }
    bool operator!=(const transform_iterator& other) const {
        return !(*this == other);
    }
};

template <typename IteratorT, typename FuncT>
transform_iterator<IteratorT, FuncT> make_transform_iterator(IteratorT it,
                                                             FuncT func) {
    return transform_iterator<IteratorT, FuncT>(it, func);
}

template <typename IteratorT, typename PredicateT>
class filter_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<IteratorT>::value_type;
    using difference_type =
        typename std::iterator_traits<IteratorT>::difference_type;
    using pointer = typename std::iterator_traits<IteratorT>::pointer;
    using reference = typename std::iterator_traits<IteratorT>::reference;

private:
    IteratorT it_;
    IteratorT end_;
    PredicateT pred_;

    void satisfy_predicate() {
        while (it_ != end_ && !pred_(*it_)) {
            ++it_;
        }
    }

public:
    filter_iterator() : it_(), end_(), pred_() {}
    filter_iterator(IteratorT it, IteratorT end, PredicateT pred)
        : it_(it), end_(end), pred_(pred) {
        satisfy_predicate();
    }

    reference operator*() const { return *it_; }
    pointer operator->() const { return &(operator*()); }

    filter_iterator& operator++() {
        ++it_;
        satisfy_predicate();
        return *this;
    }

    filter_iterator operator++(int) {
        filter_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const filter_iterator& other) const {
        return it_ == other.it_;
    }
    bool operator!=(const filter_iterator& other) const {
        return !(*this == other);
    }
};

template <typename IteratorT>
class reverse_iterator {
public:
    using iterator_category =
        typename std::iterator_traits<IteratorT>::iterator_category;
    using value_type = typename std::iterator_traits<IteratorT>::value_type;
    using difference_type =
        typename std::iterator_traits<IteratorT>::difference_type;
    using pointer = typename std::iterator_traits<IteratorT>::pointer;
    using reference = typename std::iterator_traits<IteratorT>::reference;

private:
    IteratorT current;

public:
    reverse_iterator() : current() {}
    explicit reverse_iterator(IteratorT x) : current(x) {}

    IteratorT base() const { return current; }
    reference operator*() const {
        IteratorT tmp = current;
        return *--tmp;
    }
    pointer operator->() const { return &(operator*()); }
    reverse_iterator& operator++() {
        --current;
        return *this;
    }
    reverse_iterator operator++(int) {
        reverse_iterator tmp = *this;
        --current;
        return tmp;
    }
    reverse_iterator& operator--() {
        ++current;
        return *this;
    }
    reverse_iterator operator--(int) {
        reverse_iterator tmp = *this;
        ++current;
        return tmp;
    }

    bool operator==(const reverse_iterator& x) const {
        return current == x.current;
    }
    bool operator!=(const reverse_iterator& x) const { return !(*this == x); }
};

#endif
