#ifndef ATOM_UTILS_LINQ_HPP
#define ATOM_UTILS_LINQ_HPP

#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atom::utils {
// Helper to deduce return type for selectMany
template <typename T>
auto flatten(const std::vector<std::vector<T>>& nested) {
    std::vector<T> flat;
    for (const auto& sublist : nested) {
        flat.insert(flat.end(), sublist.begin(), sublist.end());
    }
    return flat;
}

template <typename T>
class Enumerable {
public:
    explicit Enumerable(const std::vector<T>& elements) : elements_(elements) {}

    // ======== Filters and Reorders ========
    // Filter
    [[nodiscard]] auto where(auto predicate) const -> Enumerable<T> {
        std::vector<T> result;
        for (const T& element : elements_ | std::views::filter(predicate)) {
            result.push_back(element);
        }
        return Enumerable(result);
    }

    template <typename U, typename BinaryOperation>
    [[nodiscard]] auto reduce(U init, BinaryOperation binary_op) const -> U {
        return std::accumulate(elements_.begin(), elements_.end(), init,
                               binary_op);
    }

    [[nodiscard]] auto whereI(auto predicate) const -> Enumerable<T> {
        std::vector<T> result;
        for (size_t index = 0; index < elements_.size(); ++index) {
            if (predicate(elements_[index], index)) {
                result.push_back(elements_[index]);
            }
        }
        return Enumerable(result);
    }

    [[nodiscard]] auto take(size_t count) const -> Enumerable<T> {
        return Enumerable(std::vector<T>(
            elements_.begin(),
            elements_.begin() + std::min(count, elements_.size())));
    }

    [[nodiscard]] auto takeWhile(auto predicate) const -> Enumerable<T> {
        std::vector<T> result;
        for (const auto& element : elements_) {
            if (!predicate(element)) {
                break;
            }
            result.push_back(element);
        }
        return Enumerable(result);
    }

    [[nodiscard]] auto takeWhileI(auto predicate) const -> Enumerable<T> {
        std::vector<T> result;
        for (size_t index = 0; index < elements_.size(); ++index) {
            if (!predicate(elements_[index], index)) {
                break;
            }
            result.push_back(elements_[index]);
        }
        return Enumerable(result);
    }

    [[nodiscard]] auto skip(size_t count) const -> Enumerable<T> {
        return Enumerable(std::vector<T>(
            elements_.begin() + std::min(count, elements_.size()),
            elements_.end()));
    }

    [[nodiscard]] auto skipWhile(auto predicate) const -> Enumerable<T> {
        auto iterator =
            std::find_if_not(elements_.begin(), elements_.end(), predicate);
        return Enumerable(std::vector<T>(iterator, elements_.end()));
    }

    [[nodiscard]] auto skipWhileI(auto predicate) const -> Enumerable<T> {
        auto iterator =
            std::find_if_not(elements_.begin(), elements_.end(),
                             [index = 0, &predicate](const T& element) mutable {
                                 return predicate(element, index++);
                             });
        return Enumerable(std::vector<T>(iterator, elements_.end()));
    }

    [[nodiscard]] auto orderBy() const -> Enumerable<T> {
        std::vector<T> result = elements_;
        std::sort(result.begin(), result.end());
        return Enumerable(result);
    }

    [[nodiscard]] auto orderBy(auto transformer) const -> Enumerable<T> {
        std::vector<T> result = elements_;
        std::sort(result.begin(), result.end(),
                  [&transformer](const T& elementA, const T& elementB) {
                      return transformer(elementA) < transformer(elementB);
                  });
        return Enumerable(result);
    }

    [[nodiscard]] auto distinct() const -> Enumerable<T> {
        std::unordered_set<T> set(elements_.begin(), elements_.end());
        return Enumerable(std::vector<T>(set.begin(), set.end()));
    }

    [[nodiscard]] auto distinct(auto transformer) const -> Enumerable<T> {
        std::unordered_set<std::invoke_result_t<decltype(transformer), T>> set;
        std::vector<T> result;
        for (const auto& element : elements_) {
            auto transformed = transformer(element);
            if (set.insert(transformed).second) {
                result.push_back(element);
            }
        }
        return Enumerable(result);
    }

    [[nodiscard]] auto append(const std::vector<T>& items) const
        -> Enumerable<T> {
        std::vector<T> result = elements_;
        result.insert(result.end(), items.begin(), items.end());
        return Enumerable(result);
    }

    [[nodiscard]] auto prepend(const std::vector<T>& items) const
        -> Enumerable<T> {
        std::vector<T> result = items;
        result.insert(result.end(), elements_.begin(), elements_.end());
        return Enumerable(result);
    }

    [[nodiscard]] auto concat(const Enumerable<T>& other) const
        -> Enumerable<T> {
        return append(other.elements_);
    }

    [[nodiscard]] auto reverse() const -> Enumerable<T> {
        std::vector<T> result = elements_;
        std::reverse(result.begin(), result.end());
        return Enumerable(result);
    }

    template <typename U>
    [[nodiscard]] auto cast() const -> Enumerable<U> {
        std::vector<U> result;
        for (const T& element : elements_) {
            result.push_back(static_cast<U>(element));
        }
        return Enumerable<U>(result);
    }

    // ======== Transformers ========
    template <typename U>
    [[nodiscard]] auto select(auto transformer) const -> Enumerable<U> {
        std::vector<U> result;
        for (const auto& element :
             elements_ | std::views::transform(transformer)) {
            result.push_back(transformer(element));
        }
        return Enumerable<U>(result);
    }

    template <typename U>
    [[nodiscard]] auto selectI(auto transformer) const -> Enumerable<U> {
        std::vector<U> result;
        for (size_t index = 0; index < elements_.size(); ++index) {
            result.push_back(transformer(elements_[index], index));
        }
        return Enumerable<U>(result);
    }

    template <typename U>
    [[nodiscard]] auto groupBy(auto transformer) const -> Enumerable<U> {
        std::unordered_map<U, std::vector<T>> groups;
        for (const T& element : elements_) {
            groups[transformer(element)].push_back(element);
        }
        std::vector<U> keys;
        keys.reserve(groups.size());
        for (const auto& group : groups) {
            keys.push_back(group.first);
        }
        return Enumerable<U>(keys);
    }

    template <typename U>
    [[nodiscard]] auto selectMany(auto transformer) const -> Enumerable<U> {
        std::vector<std::vector<U>> nested;
        for (const T& element : elements_) {
            nested.push_back(transformer(element));
        }
        return Enumerable<U>(flatten(nested));
    }

    // ======== Aggregators ========
    [[nodiscard]] auto all(auto predicate = [](const T&) {
        return true;
    }) const -> bool {
        return std::all_of(elements_.begin(), elements_.end(), predicate);
    }

    [[nodiscard]] auto any(auto predicate = [](const T&) {
        return true;
    }) const -> bool {
        return std::any_of(elements_.begin(), elements_.end(), predicate);
    }

    [[nodiscard]] auto sum() const -> T {
        return std::accumulate(elements_.begin(), elements_.end(), T{});
    }

    template <typename U>
    [[nodiscard]] auto sum(auto transformer) const -> U {
        U result{};
        for (const auto& element : elements_) {
            result += transformer(element);
        }
        return result;
    }

    [[nodiscard]] auto avg() const -> double {
        return static_cast<double>(sum()) / elements_.size();
    }

    template <typename U>
    [[nodiscard]] auto avg(auto transformer) const -> U {
        return sum<U>(transformer) / static_cast<U>(elements_.size());
    }

    [[nodiscard]] auto min() const -> T {
        return *std::min_element(elements_.begin(), elements_.end());
    }

    [[nodiscard]] auto min(auto transformer) const -> T {
        return *std::min_element(
            elements_.begin(), elements_.end(),
            [&transformer](const T& elementA, const T& elementB) {
                return transformer(elementA) < transformer(elementB);
            });
    }

    [[nodiscard]] auto max() const -> T {
        return *std::max_element(elements_.begin(), elements_.end());
    }

    [[nodiscard]] auto max(auto transformer) const -> T {
        return *std::max_element(
            elements_.begin(), elements_.end(),
            [&transformer](const T& elementA, const T& elementB) {
                return transformer(elementA) < transformer(elementB);
            });
    }

    [[nodiscard]] auto count() const -> size_t { return elements_.size(); }

    [[nodiscard]] auto count(auto predicate) const -> size_t {
        return std::count_if(elements_.begin(), elements_.end(), predicate);
    }

    [[nodiscard]] auto contains(const T& value) const -> bool {
        return std::find(elements_.begin(), elements_.end(), value) !=
               elements_.end();
    }

    [[nodiscard]] auto elementAt(size_t index) const -> T {
        return elements_.at(index);
    }

    [[nodiscard]] auto first() const -> T { return elements_.front(); }

    [[nodiscard]] auto first(auto predicate) const -> T {
        return *std::find_if(elements_.begin(), elements_.end(), predicate);
    }

    [[nodiscard]] auto firstOrDefault() const -> std::optional<T> {
        return elements_.empty() ? std::nullopt
                                 : std::optional<T>(elements_.front());
    }

    [[nodiscard]] auto firstOrDefault(auto predicate) const
        -> std::optional<T> {
        auto iterator =
            std::find_if(elements_.begin(), elements_.end(), predicate);
        return iterator == elements_.end() ? std::nullopt
                                           : std::optional<T>(*iterator);
    }

    [[nodiscard]] auto last() const -> T { return elements_.back(); }

    [[nodiscard]] auto last(auto predicate) const -> T {
        return *std::find_if(elements_.rbegin(), elements_.rend(), predicate);
    }

    [[nodiscard]] auto lastOrDefault() const -> std::optional<T> {
        return elements_.empty() ? std::nullopt
                                 : std::optional<T>(elements_.back());
    }

    [[nodiscard]] auto lastOrDefault(auto predicate) const -> std::optional<T> {
        auto iterator =
            std::find_if(elements_.rbegin(), elements_.rend(), predicate);
        return iterator == elements_.rend() ? std::nullopt
                                            : std::optional<T>(*iterator);
    }

    // ======== Conversion Methods ========
    [[nodiscard]] auto toStdSet() const -> std::set<T> {
        return std::set<T>(elements_.begin(), elements_.end());
    }

    [[nodiscard]] auto toStdList() const -> std::list<T> {
        return std::list<T>(elements_.begin(), elements_.end());
    }

    [[nodiscard]] auto toStdDeque() const -> std::deque<T> {
        return std::deque<T>(elements_.begin(), elements_.end());
    }

    [[nodiscard]] auto toStdVector() const -> std::vector<T> {
        return elements_;
    }

    // ======== Printing ========
    void print() const {
        for (const T& element : elements_) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<T> elements_;
};
}  // namespace atom::utils

#endif