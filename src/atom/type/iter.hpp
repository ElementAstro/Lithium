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
#include <optional>
#include <utility>
#include <vector>

#if ENABLE_DEBUG
#include <iostream>
#endif

template <typename IteratorT>
class PointerIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = decltype(&*std::declval<IteratorT>());
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    IteratorT it_;

public:
    PointerIterator() = default;

    explicit PointerIterator(IteratorT it) : it_(std::move(it)) {}

    value_type operator*() const { return &*it_; }

    PointerIterator& operator++() {
        ++it_;
        return *this;
    }

    auto operator++(int) -> PointerIterator {
        PointerIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    auto operator==(const PointerIterator& other) const -> bool = default;
    auto operator!=(const PointerIterator& other) const -> bool = default;
};

template <typename IteratorT>
auto makePointerRange(IteratorT begin, IteratorT end) {
    return std::make_pair(PointerIterator<IteratorT>(begin),
                          PointerIterator<IteratorT>(end));
}

template <typename ContainerT>
void processContainer(ContainerT& container) {
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
class EarlyIncIterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    EarlyIncIterator() = default;
    explicit EarlyIncIterator(I x) : current_(x) {}

    auto operator++() -> EarlyIncIterator& {
        ++current_;
        return *this;
    }

    auto operator++(int) -> EarlyIncIterator {
        EarlyIncIterator tmp = *this;
        ++current_;
        return tmp;
    }

    friend auto operator==(const EarlyIncIterator& x,
                           const EarlyIncIterator& y) -> bool {
        return x.current_ == y.current_;
    }

    friend auto operator!=(const EarlyIncIterator& x,
                           const EarlyIncIterator& y) -> bool {
        return !(x == y);
    }

    auto operator*() const { return *current_; }

private:
    I current_{};
};

template <std::input_or_output_iterator I>
auto makeEarlyIncIterator(I x) -> EarlyIncIterator<I> {
    return EarlyIncIterator<I>(x);
}

template <typename IteratorT, typename FuncT>
class TransformIterator {
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
    TransformIterator() : it_(), func_() {}
    TransformIterator(IteratorT it, FuncT func) : it_(it), func_(func) {}

    auto operator*() const -> reference { return func_(*it_); }
    auto operator->() const -> pointer { return &(operator*()); }

    auto operator++() -> TransformIterator& {
        ++it_;
        return *this;
    }
    auto operator++(int) -> TransformIterator {
        TransformIterator tmp = *this;
        ++it_;
        return tmp;
    }

    auto operator==(const TransformIterator& other) const -> bool {
        return it_ == other.it_;
    }
    auto operator!=(const TransformIterator& other) const -> bool {
        return !(*this == other);
    }
};

template <typename IteratorT, typename FuncT>
auto makeTransformIterator(IteratorT it,
                           FuncT func) -> TransformIterator<IteratorT, FuncT> {
    return TransformIterator<IteratorT, FuncT>(it, func);
}

template <typename IteratorT, typename PredicateT>
class FilterIterator {
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

    void satisfyPredicate() {
        while (it_ != end_ && !pred_(*it_)) {
            ++it_;
        }
    }

public:
    FilterIterator() : it_(), end_(), pred_() {}
    FilterIterator(IteratorT it, IteratorT end, PredicateT pred)
        : it_(it), end_(end), pred_(pred) {
        satisfyPredicate();
    }

    auto operator*() const -> reference { return *it_; }
    auto operator->() const -> pointer { return &(operator*()); }

    auto operator++() -> FilterIterator& {
        ++it_;
        satisfyPredicate();
        return *this;
    }

    auto operator++(int) -> FilterIterator {
        FilterIterator tmp = *this;
        ++*this;
        return tmp;
    }

    auto operator==(const FilterIterator& other) const -> bool {
        return it_ == other.it_;
    }
    auto operator!=(const FilterIterator& other) const -> bool {
        return !(*this == other);
    }
};

template <typename IteratorT, typename PredicateT>
auto makeFilterIterator(IteratorT it, IteratorT end, PredicateT pred)
    -> FilterIterator<IteratorT, PredicateT> {
    return FilterIterator<IteratorT, PredicateT>(it, end, pred);
}

template <typename IteratorT>
class ReverseIterator {
public:
    using iterator_category =
        typename std::iterator_traits<IteratorT>::iterator_category;
    using value_type = typename std::iterator_traits<IteratorT>::value_type;
    using difference_type =
        typename std::iterator_traits<IteratorT>::difference_type;
    using pointer = typename std::iterator_traits<IteratorT>::pointer;
    using reference = typename std::iterator_traits<IteratorT>::reference;

private:
    IteratorT current_;

public:
    ReverseIterator() : current_() {}
    explicit ReverseIterator(IteratorT x) : current_(x) {}

    auto base() const -> IteratorT { return current_; }
    auto operator*() const -> reference {
        IteratorT tmp = current_;
        return *--tmp;
    }
    auto operator->() const -> pointer { return &(operator*()); }
    auto operator++() -> ReverseIterator& {
        --current_;
        return *this;
    }
    auto operator++(int) -> ReverseIterator {
        ReverseIterator tmp = *this;
        --current_;
        return tmp;
    }
    auto operator--() -> ReverseIterator& {
        ++current_;
        return *this;
    }
    auto operator--(int) -> ReverseIterator {
        ReverseIterator tmp = *this;
        ++current_;
        return tmp;
    }

    auto operator==(const ReverseIterator& x) const -> bool {
        return current_ == x.current_;
    }
    auto operator!=(const ReverseIterator& x) const -> bool {
        return !(*this == x);
    }
};

#endif
