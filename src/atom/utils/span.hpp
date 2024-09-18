#ifndef ATOM_UTILS_SPAN_HPP
#define ATOM_UTILS_SPAN_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <span>
#include <unordered_map>
#include <vector>

namespace atom::utils {
/**
 * @brief Computes the sum of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return T The sum of the elements.
 */
template <typename T>
auto sum(std::span<T> data) -> T {
    return std::accumulate(data.begin(), data.end(), T{0});
}

/**
 * @brief Checks if a span contains a specific value.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to search.
 * @param value The value to find.
 * @return bool True if the value is found, false otherwise.
 */
template <typename T>
auto contains(std::span<T> data, T value) -> bool {
    return std::find(data.begin(), data.end(), value) != data.end();
}

/**
 * @brief Sorts the elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to sort.
 */
template <typename T>
void sortSpan(std::span<T> data) {
    std::sort(data.begin(), data.end());
}

/**
 * @brief Filters elements in a span based on a predicate.
 *
 * @tparam T The type of elements in the span.
 * @tparam Predicate The type of the predicate function.
 * @param data The span to filter.
 * @param pred The predicate function to apply.
 * @return std::vector<T> A vector containing the filtered elements.
 */
template <typename T, typename Predicate>
auto filterSpan(std::span<T> data, Predicate pred) -> std::vector<T> {
    std::vector<T> result;
    std::copy_if(data.begin(), data.end(), std::back_inserter(result), pred);
    return result;
}

/**
 * @brief Counts the number of elements in a span that satisfy a predicate.
 *
 * @tparam T The type of elements in the span.
 * @tparam Predicate The type of the predicate function.
 * @param data The span to search.
 * @param pred The predicate function to apply.
 * @return size_t The number of elements that satisfy the predicate.
 */
template <typename T, typename Predicate>
auto countIfSpan(std::span<T> data, Predicate pred) -> size_t {
    return std::count_if(data.begin(), data.end(), pred);
}

/**
 * @brief Finds the minimum element in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to search.
 * @return T The minimum element.
 */
template <typename T>
auto minElementSpan(std::span<T> data) -> T {
    return *std::min_element(data.begin(), data.end());
}

/**
 * @brief Finds the maximum element in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to search.
 * @return T The maximum element.
 */
template <typename T>
auto maxElementSpan(std::span<T> data) -> T {
    return *std::max_element(data.begin(), data.end());
}

/**
 * @brief Finds the index of the maximum element in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to search.
 * @return size_t The index of the maximum element.
 */
template <typename T>
auto maxElementIndex(std::span<T> data) -> size_t {
    return std::distance(data.begin(),
                         std::max_element(data.begin(), data.end()));
}

/**
 * @brief Prints the elements of a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to print.
 */
template <typename T>
void printSpan(std::span<T> data) {
    for (const auto& element : data) {
        std::cout << element << " ";
    }
    std::cout << '\n';
}

/**
 * @brief Transposes a matrix represented as a span.
 *
 * @tparam T The type of elements in the matrix.
 * @tparam N The size of the span.
 * @param matrix The span representing the matrix.
 * @param rows The number of rows in the matrix.
 * @param cols The number of columns in the matrix.
 */
template <typename T, size_t N>
void transposeMatrix(std::span<T, N> matrix, size_t rows, size_t cols) {
    std::vector<T> transposedMatrix(cols * rows);
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            transposedMatrix[j * rows + i] = matrix[i * cols + j];
        }
    }
    std::copy(transposedMatrix.begin(), transposedMatrix.end(), matrix.begin());
}

/**
 * @brief Normalizes the elements in a span to the range [0, 1].
 *
 * @tparam T The type of elements in the span.
 * @param data The span to normalize.
 */
template <typename T>
void normalize(std::span<T> data) {
    T minVal = *std::min_element(data.begin(), data.end());
    T maxVal = *std::max_element(data.begin(), data.end());
    T range = maxVal - minVal;
    if (range == 0) {
        return;  // Avoid division by zero
    }
    for (auto& element : data) {
        element = (element - minVal) / range;
    }
}

/**
 * @brief Computes the mean of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return double The mean of the elements.
 */
template <typename T>
auto mean(std::span<T> data) -> double {
    if (data.empty()) {
        return 0.0;
    }
    return static_cast<double>(sum(data)) / data.size();
}

/**
 * @brief Finds the median of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return double The median of the elements.
 */
template <typename T>
auto median(std::span<T> data) -> double {
    if (data.empty()) {
        return 0.0;
    }
    std::vector<T> sortedData(data.begin(), data.end());
    std::sort(sortedData.begin(), sortedData.end());
    size_t mid = sortedData.size() / 2;
    if (sortedData.size() % 2 == 0) {
        return (sortedData[mid - 1] + sortedData[mid]) / 2.0;
    }
    return sortedData[mid];
}

/**
 * @brief Finds the mode of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return T The mode of the elements.
 */
template <typename T>
auto mode(std::span<T> data) -> T {
    std::unordered_map<T, size_t> frequency;
    for (const auto& element : data) {
        ++frequency[element];
    }
    return std::max_element(
               frequency.begin(), frequency.end(),
               [](const auto& a, const auto& b) { return a.second < b.second; })
        ->first;
}

/**
 * @brief Computes the standard deviation of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return double The standard deviation of the elements.
 */
template <typename T>
auto standardDeviation(std::span<T> data) -> double {
    if (data.empty()) {
        return 0.0;
    }
    double meanValue = mean(data);
    double sumOfSquares = std::accumulate(
        data.begin(), data.end(), 0.0, [meanValue](double acc, T value) {
            return acc + (value - meanValue) * (value - meanValue);
        });
    return std::sqrt(sumOfSquares / data.size());
}

/**
 * @brief Finds the top N maximum elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @param n The number of top elements to find.
 * @return std::vector<T> A vector containing the top N elements.
 */
template <typename T>
auto topNElements(std::span<T> data, size_t n) -> std::vector<T> {
    std::vector<T> result(data.begin(), data.end());
    std::partial_sort(result.begin(), result.begin() + n, result.end(),
                      std::greater<T>());
    result.resize(n);
    return result;
}

/**
 * @brief Computes the variance of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return double The variance of the elements.
 */
template <typename T>
auto variance(std::span<T> data) -> double {
    if (data.empty()) {
        return 0.0;
    }
    double meanValue = mean(data);
    double sumOfSquares = std::accumulate(
        data.begin(), data.end(), 0.0, [meanValue](double acc, T value) {
            return acc + (value - meanValue) * (value - meanValue);
        });
    return sumOfSquares / data.size();
}

/**
 * @brief Finds the top N minimum elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @param n The number of top elements to find.
 * @return std::vector<T> A vector containing the top N elements.
 */
template <typename T>
auto bottomNElements(std::span<T> data, size_t n) -> std::vector<T> {
    std::vector<T> result(data.begin(), data.end());
    std::partial_sort(result.begin(), result.begin() + n, result.end());
    result.resize(n);
    return result;
}

/**
 * @brief Computes the cumulative sum of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return std::vector<T> A vector containing the cumulative sums.
 */
template <typename T>
auto cumulativeSum(std::span<T> data) -> std::vector<T> {
    std::vector<T> result(data.size());
    std::partial_sum(data.begin(), data.end(), result.begin());
    return result;
}

/**
 * @brief Computes the cumulative product of elements in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span containing the elements.
 * @return std::vector<T> A vector containing the cumulative products.
 */
template <typename T>
auto cumulativeProduct(std::span<T> data) -> std::vector<T> {
    std::vector<T> result(data.size());
    std::partial_sum(data.begin(), data.end(), result.begin(),
                     std::multiplies<T>());
    return result;
}

/**
 * @brief Finds the index of a specific value in a span.
 *
 * @tparam T The type of elements in the span.
 * @param data The span to search.
 * @param value The value to find.
 * @return std::optional<size_t> The index of the value if found, std::nullopt
 * otherwise.
 */
template <typename T>
auto findIndex(std::span<T> data, T value) -> std::optional<size_t> {
    auto it = std::find(data.begin(), data.end(), value);
    if (it != data.end()) {
        return std::distance(data.begin(), it);
    }
    return std::nullopt;
}
}  // namespace atom::utils

#endif  // ATOM_UTILS_SPAN_HPP
