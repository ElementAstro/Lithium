#ifndef ATOM_UTILS_CONTAINER_HPP
#define ATOM_UTILS_CONTAINER_HPP

#include <algorithm>
#include <functional>
#include <ranges>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace atom::utils {
/**
 * @brief Checks if one container is a subset of another container.
 *
 * @tparam Container1 Type of the subset container.
 * @tparam Container2 Type of the superset container.
 * @param subset The container to check if it is a subset.
 * @param superset The container to check against.
 * @return true if subset is a subset of superset, false otherwise.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
                 std::ranges::input_range<Container2> &&
                 std::equality_comparable_with<
                     typename Container1::value_type,
                     typename Container2::value_type> &&
                 std::regular<typename Container2::value_type> &&
                 std::regular<typename Container1::value_type>
auto isSubset(const Container1& subset, const Container2& superset) -> bool {
    std::unordered_set<typename Container2::value_type> set(superset.begin(),
                                                            superset.end());
    return std::ranges::all_of(
        subset, [&set](const auto& elem) { return set.contains(elem); });
}

/**
 * @brief Checks if a container contains a specific element.
 *
 * @tparam Container Type of the container.
 * @tparam T Type of the element.
 * @param container The container to check.
 * @param value The element to check for.
 * @return true if the container contains the element, false otherwise.
 */
template <typename Container, typename T>
    requires std::ranges::input_range<Container> &&
                 std::equality_comparable_with<typename Container::value_type,
                                               T>
auto contains(const Container& container, const T& value) -> bool {
    return std::ranges::find(container, value) != container.end();
}

/**
 * @brief Converts a container to an unordered_set for fast lookup.
 *
 * @tparam Container Type of the container.
 * @param container The container to convert.
 * @return std::unordered_set<typename Container::value_type> The converted
 * unordered_set.
 */
template <typename Container>
    requires std::ranges::input_range<Container> &&
             std::regular<typename Container::value_type>
auto toUnorderedSet(const Container& container) {
    return std::unordered_set<typename Container::value_type>(container.begin(),
                                                              container.end());
}

/**
 * @brief Checks if one container is a subset of another container using linear
 * search.
 *
 * @tparam Container1 Type of the subset container.
 * @tparam Container2 Type of the superset container.
 * @param subset The container to check if it is a subset.
 * @param superset The container to check against.
 * @return true if subset is a subset of superset, false otherwise.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
                 std::ranges::input_range<Container2> &&
                 std::equality_comparable_with<typename Container1::value_type,
                                               typename Container2::value_type>
auto isSubsetLinearSearch(const Container1& subset,
                          const Container2& superset) -> bool {
    return std::ranges::all_of(subset, [&superset](const auto& elem) {
        return contains(superset, elem);
    });
}

/**
 * @brief Checks if one container is a subset of another container using an
 * unordered_set for fast lookup.
 *
 * @tparam Container1 Type of the subset container.
 * @tparam Container2 Type of the superset container.
 * @param subset The container to check if it is a subset.
 * @param superset The container to check against.
 * @return true if subset is a subset of superset, false otherwise.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
                 std::ranges::input_range<Container2> &&
                 std::equality_comparable_with<
                     typename Container1::value_type,
                     typename Container2::value_type> &&
                 std::regular<typename Container2::value_type>
auto isSubsetWithHashSet(const Container1& subset,
                         const Container2& superset) -> bool {
    auto supersetSet =
        toUnorderedSet(superset);  // Convert superset to unordered_set
    return std::ranges::all_of(subset, [&supersetSet](const auto& elem) {
        return supersetSet.contains(elem);
    });
}

/**
 * @brief Returns the intersection of two containers.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<typename Container1::value_type> The intersection of the
 * two containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2> &&
             std::equality_comparable_with<typename Container1::value_type,
                                           typename Container2::value_type>
auto intersection(const Container1& container1, const Container2& container2) {
    std::vector<typename Container1::value_type> result;
    for (const auto& elem : container1) {
        if (contains(container2, elem)) {
            result.push_back(elem);
        }
    }
    return result;
}

/**
 * @brief Returns the union of two containers.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<typename Container1::value_type> The union of the two
 * containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2> &&
             std::equality_comparable_with<typename Container1::value_type,
                                           typename Container2::value_type>
auto unionSet(const Container1& container1, const Container2& container2) {
    std::unordered_set<typename Container1::value_type> result(
        container1.begin(), container1.end());
    result.insert(container2.begin(), container2.end());
    return std::vector<typename Container1::value_type>(result.begin(),
                                                        result.end());
}

/**
 * @brief Returns the difference of two containers (container1 - container2).
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<typename Container1::value_type> The difference of the
 * two containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2> &&
             std::equality_comparable_with<typename Container1::value_type,
                                           typename Container2::value_type>
auto difference(const Container1& container1, const Container2& container2) {
    std::vector<typename Container1::value_type> result;
    for (const auto& elem : container1) {
        if (!contains(container2, elem)) {
            result.push_back(elem);
        }
    }
    return result;
}

/**
 * @brief Returns the symmetric difference of two containers.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<typename Container1::value_type> The symmetric difference
 * of the two containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2> &&
             std::equality_comparable_with<typename Container1::value_type,
                                           typename Container2::value_type>
auto symmetricDifference(const Container1& container1,
                         const Container2& container2) {
    auto diff1 = difference(container1, container2);
    auto diff2 = difference(container2, container1);
    auto result = unionSet(diff1, diff2);
    return result;
}

/**
 * @brief Checks if two containers are equal.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return true if the containers are equal, false otherwise.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
                 std::ranges::input_range<Container2> &&
                 std::equality_comparable_with<typename Container1::value_type,
                                               typename Container2::value_type>
auto isEqual(const Container1& container1,
             const Container2& container2) -> bool {
    return isSubsetLinearSearch(container1, container2) &&
           isSubsetLinearSearch(container2, container1);
}

template <typename Container, typename MemberFunc>
auto applyAndStore(const Container& source, MemberFunc memberFunc) {
    using ReturnType =
        decltype((std::invoke(memberFunc, *source.begin())));  // 推导返回值类型
    std::vector<ReturnType> result;  // 创建存储结果的新容器

    for (const auto& elem : source) {
        result.push_back(
            std::invoke(memberFunc, elem));  // 调用成员函数并存储结果
    }

    return result;  // 返回结果容器
}

template <typename T, typename U>
concept HasMemberFunc =
    std::invocable<U, T>;  // U 是可调用对象，且可以被 T 调用

// 泛型转换函数，带有Concept约束，确保类型符合要求
template <typename Container, typename MemberFunc>
    requires std::ranges::input_range<Container> &&
             HasMemberFunc<typename Container::value_type, MemberFunc>
auto transformToVector(const Container& source, MemberFunc memberFunc) {
    using ReturnType = decltype(std::invoke(
        memberFunc, *std::begin(source)));  // 推导返回值类型
    std::vector<ReturnType> result;         // 创建目标类型的std::vector

    for (const auto& elem : source) {
        result.push_back(
            std::invoke(memberFunc, elem));  // 调用成员函数并存储结果
    }

    return result;  // 返回新容器
}

/**
 * @brief Removes duplicate elements from a container.
 *
 * @tparam Container Type of the container.
 * @param container The container from which to remove duplicates.
 * @return std::vector<typename Container::value_type> A new container without
 * duplicates.
 */
template <typename Container>
    requires std::ranges::input_range<Container> &&
             std::regular<typename Container::value_type>
auto unique(const Container& container) {
    std::unordered_set<typename Container::value_type> set(container.begin(),
                                                           container.end());
    return std::vector<typename Container::value_type>(set.begin(), set.end());
}

/**
 * @brief Flattens a container of containers into a single container.
 *
 * @tparam Container Type of the outer container.
 * @tparam InnerContainer Type of the inner containers.
 * @param container The container of containers to flatten.
 * @return std::vector<typename InnerContainer::value_type> The flattened
 * container.
 */
template <typename Container>
    requires std::ranges::input_range<Container> &&
             std::ranges::input_range<typename Container::value_type>
auto flatten(const Container& container) {
    using InnerContainer = typename Container::value_type;
    std::vector<typename InnerContainer::value_type> result;

    for (const auto& inner : container) {
        result.insert(result.end(), inner.begin(), inner.end());
    }

    return result;
}

/**
 * @brief Zips two containers into a container of pairs.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<std::pair<typename Container1::value_type, typename
 * Container2::value_type>> A container of pairs from the two containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2>
auto zip(const Container1& container1, const Container2& container2) {
    using ValueType1 = typename Container1::value_type;
    using ValueType2 = typename Container2::value_type;
    std::vector<std::pair<ValueType1, ValueType2>> result;

    auto it1 = container1.begin();
    auto it2 = container2.begin();

    while (it1 != container1.end() && it2 != container2.end()) {
        result.emplace_back(*it1, *it2);
        ++it1;
        ++it2;
    }

    return result;
}

/**
 * @brief Computes the Cartesian product of two containers.
 *
 * @tparam Container1 Type of the first container.
 * @tparam Container2 Type of the second container.
 * @param container1 The first container.
 * @param container2 The second container.
 * @return std::vector<std::pair<typename Container1::value_type, typename
 * Container2::value_type>> The Cartesian product of the two containers.
 */
template <typename Container1, typename Container2>
    requires std::ranges::input_range<Container1> &&
             std::ranges::input_range<Container2>
auto cartesianProduct(const Container1& container1,
                      const Container2& container2) {
    using ValueType1 = typename Container1::value_type;
    using ValueType2 = typename Container2::value_type;
    std::vector<std::pair<ValueType1, ValueType2>> result;

    for (const auto& elem1 : container1) {
        for (const auto& elem2 : container2) {
            result.emplace_back(elem1, elem2);
        }
    }

    return result;
}

/**
 * @brief Filters elements in a container based on a predicate.
 *
 * @tparam Container Type of the container.
 * @tparam Predicate Type of the predicate function.
 * @param container The container to filter.
 * @param predicate The predicate function to apply.
 * @return std::vector<typename Container::value_type> A new container with
 * elements that satisfy the predicate.
 */
template <typename Container, typename Predicate>
    requires std::ranges::input_range<Container> &&
             std::predicate<Predicate, typename Container::value_type>
auto filter(const Container& container, Predicate predicate) {
    std::vector<typename Container::value_type> result;

    for (const auto& elem : container) {
        if (predicate(elem)) {
            result.push_back(elem);
        }
    }

    return result;
}

/**
 * @brief Partitions a container into two containers based on a predicate.
 *
 * @tparam Container Type of the container.
 * @tparam Predicate Type of the predicate function.
 * @param container The container to partition.
 * @param predicate The predicate function to apply.
 * @return std::pair<std::vector<typename Container::value_type>,
 * std::vector<typename Container::value_type>> Two containers: one where the
 * predicate is true, and one where it is false.
 */
template <typename Container, typename Predicate>
    requires std::ranges::input_range<Container> &&
             std::predicate<Predicate, typename Container::value_type>
auto partition(const Container& container, Predicate predicate) {
    std::vector<typename Container::value_type> truePart;
    std::vector<typename Container::value_type> falsePart;

    for (const auto& elem : container) {
        if (predicate(elem)) {
            truePart.push_back(elem);
        } else {
            falsePart.push_back(elem);
        }
    }

    return std::make_pair(truePart, falsePart);
}

/**
 * @brief Finds the first element in a container that satisfies a predicate.
 *
 * @tparam Container Type of the container.
 * @tparam Predicate Type of the predicate function.
 * @param container The container to search.
 * @param predicate The predicate function.
 * @return std::optional<typename Container::value_type> The first element that
 * satisfies the predicate, or std::nullopt if none found.
 */
template <typename Container, typename Predicate>
    requires std::ranges::input_range<Container> &&
                 std::predicate<Predicate, typename Container::value_type>
auto findIf(const Container& container, Predicate predicate)
    -> std::optional<typename Container::value_type> {
    for (const auto& elem : container) {
        if (predicate(elem)) {
            return elem;
        }
    }
    return std::nullopt;
}

}  // namespace atom::utils

inline auto operator"" _vec(const char* str,
                            size_t) -> std::vector<std::string> {
    std::vector<std::string> vec;
    std::string token;
    std::istringstream tokenStream(str);

    // 使用逗号作为分隔符将字符串分割成多个部分
    while (std::getline(tokenStream, token, ',')) {
        // 去除多余的空格
        size_t start = token.find_first_not_of(" ");
        size_t end = token.find_last_not_of(" ");
        if (start != std::string::npos && end != std::string::npos) {
            vec.push_back(token.substr(start, end - start + 1));
        }
    }

    return vec;
}

#endif  // ATOM_UTILS_CONTAINER_HPP
