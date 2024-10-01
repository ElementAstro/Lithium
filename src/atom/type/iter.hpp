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
#include <tuple>
#include <utility>
#include <vector>

#if ENABLE_DEBUG
#include <iostream>
#endif

/**
 * @brief An iterator that returns pointers to the elements of another iterator.
 *
 * @tparam IteratorT The type of the underlying iterator.
 */
template <typename IteratorT>
class PointerIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = decltype(&*std::declval<IteratorT>());
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

private:
    IteratorT iter_;  ///< The underlying iterator.

public:
    PointerIterator() = default;

    /**
     * @brief Constructs a PointerIterator from an underlying iterator.
     *
     * @param iterator The underlying iterator.
     */
    explicit PointerIterator(IteratorT iterator) : iter_(std::move(iterator)) {}

    /**
     * @brief Dereferences the iterator to return a pointer to the element.
     *
     * @return A pointer to the element.
     */
    auto operator*() const -> value_type { return &*iter_; }

    /**
     * @brief Pre-increment operator.
     *
     * @return Reference to the incremented iterator.
     */
    auto operator++() -> PointerIterator& {
        ++iter_;
        return *this;
    }

    /**
     * @brief Post-increment operator.
     *
     * @return A copy of the iterator before incrementing.
     */
    auto operator++(int) -> PointerIterator {
        PointerIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    /**
     * @brief Equality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are equal, false otherwise.
     */
    auto operator==(const PointerIterator& other) const -> bool = default;

    /**
     * @brief Inequality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are not equal, false otherwise.
     */
    auto operator!=(const PointerIterator& other) const -> bool = default;
};

/**
 * @brief Creates a range of PointerIterator from two iterators.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @param begin The beginning of the range.
 * @param end The end of the range.
 * @return A pair of PointerIterator representing the range.
 */
template <typename IteratorT>
auto makePointerRange(IteratorT begin, IteratorT end) {
    return std::make_pair(PointerIterator<IteratorT>(begin),
                          PointerIterator<IteratorT>(end));
}

/**
 * @brief Processes a container by erasing elements pointed to by the iterators.
 *
 * @tparam ContainerT The type of the container.
 * @param container The container to process.
 */
template <typename ContainerT>
void processContainer(ContainerT& container) {
    auto beginIter = std::next(container.begin());
    auto endIter = std::prev(container.end());

    std::vector<std::optional<typename ContainerT::value_type*>> ptrs;
    auto ptrPair = makePointerRange(beginIter, endIter);
#pragma unroll
    for (auto iter = ptrPair.first; iter != ptrPair.second; ++iter) {
        ptrs.push_back(*iter);
    }

#pragma unroll
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

/**
 * @brief An iterator that increments the underlying iterator early.
 *
 * @tparam I The type of the underlying iterator.
 */
template <std::input_or_output_iterator I>
class EarlyIncIterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    EarlyIncIterator() = default;

    /**
     * @brief Constructs an EarlyIncIterator from an underlying iterator.
     *
     * @param iterator The underlying iterator.
     */
    explicit EarlyIncIterator(I iterator) : current_(iterator) {}

    /**
     * @brief Pre-increment operator.
     *
     * @return Reference to the incremented iterator.
     */
    auto operator++() -> EarlyIncIterator& {
        ++current_;
        return *this;
    }

    /**
     * @brief Post-increment operator.
     *
     * @return A copy of the iterator before incrementing.
     */
    auto operator++(int) -> EarlyIncIterator {
        EarlyIncIterator tmp = *this;
        ++current_;
        return tmp;
    }

    /**
     * @brief Equality operator.
     *
     * @param iter1 The first iterator to compare.
     * @param iter2 The second iterator to compare.
     * @return True if the iterators are equal, false otherwise.
     */
    friend auto operator==(const EarlyIncIterator& iter1,
                           const EarlyIncIterator& iter2) -> bool {
        return iter1.current_ == iter2.current_;
    }

    /**
     * @brief Inequality operator.
     *
     * @param iter1 The first iterator to compare.
     * @param iter2 The second iterator to compare.
     * @return True if the iterators are not equal, false otherwise.
     */
    friend auto operator!=(const EarlyIncIterator& iter1,
                           const EarlyIncIterator& iter2) -> bool {
        return !(iter1 == iter2);
    }

    /**
     * @brief Dereferences the iterator.
     *
     * @return The value pointed to by the underlying iterator.
     */
    auto operator*() const { return *current_; }

private:
    I current_{};  ///< The underlying iterator.
};

/**
 * @brief Creates an EarlyIncIterator from an underlying iterator.
 *
 * @tparam I The type of the underlying iterator.
 * @param iterator The underlying iterator.
 * @return An EarlyIncIterator.
 */
template <std::input_or_output_iterator I>
auto makeEarlyIncIterator(I iterator) -> EarlyIncIterator<I> {
    return EarlyIncIterator<I>(iterator);
}

/**
 * @brief An iterator that applies a transformation function to the elements.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @tparam FuncT The type of the transformation function.
 */
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
    IteratorT iter_;  ///< The underlying iterator.
    FuncT func_;      ///< The transformation function.

public:
    TransformIterator() : iter_(), func_() {}

    /**
     * @brief Constructs a TransformIterator from an underlying iterator and a
     * transformation function.
     *
     * @param iterator The underlying iterator.
     * @param function The transformation function.
     */
    TransformIterator(IteratorT iterator, FuncT function)
        : iter_(iterator), func_(function) {}

    /**
     * @brief Dereferences the iterator and applies the transformation function.
     *
     * @return The transformed value.
     */
    auto operator*() const -> reference { return func_(*iter_); }

    /**
     * @brief Returns a pointer to the transformed value.
     *
     * @return A pointer to the transformed value.
     */
    auto operator->() const -> pointer { return &(operator*()); }

    /**
     * @brief Pre-increment operator.
     *
     * @return Reference to the incremented iterator.
     */
    auto operator++() -> TransformIterator& {
        ++iter_;
        return *this;
    }

    /**
     * @brief Post-increment operator.
     *
     * @return A copy of the iterator before incrementing.
     */
    auto operator++(int) -> TransformIterator {
        TransformIterator tmp = *this;
        ++iter_;
        return tmp;
    }

    /**
     * @brief Equality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are equal, false otherwise.
     */
    auto operator==(const TransformIterator& other) const -> bool {
        return iter_ == other.iter_;
    }

    /**
     * @brief Inequality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are not equal, false otherwise.
     */
    auto operator!=(const TransformIterator& other) const -> bool {
        return !(*this == other);
    }
};

/**
 * @brief Creates a TransformIterator from an underlying iterator and a
 * transformation function.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @tparam FuncT The type of the transformation function.
 * @param iterator The underlying iterator.
 * @param function The transformation function.
 * @return A TransformIterator.
 */
template <typename IteratorT, typename FuncT>
auto makeTransformIterator(IteratorT iterator, FuncT function)
    -> TransformIterator<IteratorT, FuncT> {
    return TransformIterator<IteratorT, FuncT>(iterator, function);
}

/**
 * @brief An iterator that filters elements based on a predicate.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @tparam PredicateT The type of the predicate.
 */
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
    IteratorT iter_;
    IteratorT end_;
    PredicateT pred_;

    /**
     * @brief Advances the iterator to the next element that satisfies the
     * predicate.
     */
    void satisfyPredicate() {
#pragma unroll
        while (iter_ != end_ && !pred_(*iter_)) {
            ++iter_;
        }
    }

public:
    FilterIterator() : iter_(), end_(), pred_() {}
    FilterIterator(IteratorT iter, IteratorT end, PredicateT pred)
        : iter_(iter), end_(end), pred_(pred) {
        satisfyPredicate();
    }

    auto operator*() const -> reference { return *iter_; }
    auto operator->() const -> pointer { return &(operator*()); }

    /**
     * @brief Pre-increment operator.
     *
     * @return Reference to the incremented iterator.
     */
    auto operator++() -> FilterIterator& {
        ++iter_;
        satisfyPredicate();
        return *this;
    }

    /**
     * @brief Post-increment operator.
     *
     * @return A copy of the iterator before incrementing.
     */
    auto operator++(int) -> FilterIterator {
        FilterIterator tmp = *this;
        ++*this;
        return tmp;
    }

    /**
     * @brief Equality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are equal, false otherwise.
     */
    auto operator==(const FilterIterator& other) const -> bool {
        return iter_ == other.iter_;
    }

    /**
     * @brief Inequality operator.
     *
     * @param other The other iterator to compare with.
     * @return True if the iterators are not equal, false otherwise.
     */
    auto operator!=(const FilterIterator& other) const -> bool {
        return !(*this == other);
    }
};

/**
 * @brief Creates a FilterIterator from an underlying iterator and a predicate.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @tparam PredicateT The type of the predicate.
 * @param iter The underlying iterator.
 * @param end The end of the range.
 * @param pred The predicate.
 * @return A FilterIterator.
 */
template <typename IteratorT, typename PredicateT>
auto makeFilterIterator(IteratorT iter, IteratorT end, PredicateT pred)
    -> FilterIterator<IteratorT, PredicateT> {
    return FilterIterator<IteratorT, PredicateT>(iter, end, pred);
}

/**
 * @brief An iterator that reverses the direction of another iterator.
 *
 * @tparam IteratorT The type of the underlying iterator.
 */
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
    explicit ReverseIterator(IteratorT iterator) : current_(iterator) {}

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

    auto operator==(const ReverseIterator& iter) const -> bool {
        return current_ == iter.current_;
    }
    auto operator!=(const ReverseIterator& iter) const -> bool {
        return !(*this == iter);
    }
};

/**
 * @brief Creates a ReverseIterator from an underlying iterator.
 *
 * @tparam IteratorT The type of the underlying iterator.
 * @param iterator The underlying iterator.
 * @return A ReverseIterator.
 */
template <typename... Iterators>
class ZipIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type =
        std::tuple<typename std::iterator_traits<Iterators>::value_type...>;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

private:
    std::tuple<Iterators...> iterators_;

    template <std::size_t... Indices>
    auto dereference(std::index_sequence<Indices...>) const -> value_type {
        return std::make_tuple(*std::get<Indices>(iterators_)...);
    }

    template <std::size_t... Indices>
    void increment(std::index_sequence<Indices...>) {
        (++std::get<Indices>(iterators_), ...);
    }

public:
    ZipIterator() = default;
    explicit ZipIterator(Iterators... its) : iterators_(its...) {}

    auto operator*() const -> value_type {
        return dereference(std::index_sequence_for<Iterators...>{});
    }

    auto operator++() -> ZipIterator& {
        increment(std::index_sequence_for<Iterators...>{});
        return *this;
    }

    auto operator++(int) -> ZipIterator {
        ZipIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    auto operator==(const ZipIterator& other) const -> bool {
        return iterators_ == other.iterators_;
    }

    auto operator!=(const ZipIterator& other) const -> bool {
        return !(*this == other);
    }
};

template <typename... Iterators>
auto makeZipIterator(Iterators... its) -> ZipIterator<Iterators...> {
    return ZipIterator<Iterators...>(its...);
}

#endif  // ATOM_EXPERIMENTAL_ITERATOR_HPP
