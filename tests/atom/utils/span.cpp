#include <gtest/gtest.h>

#include "atom/utils/span.hpp"

namespace atom::utils::test {

TEST(SpanUtilsTest, SumFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_EQ(sum(span), 15);
}

TEST(SpanUtilsTest, ContainsFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_TRUE(contains(span, 3));
    ASSERT_FALSE(contains(span, 6));
}

TEST(SpanUtilsTest, SortSpanFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    sortSpan(span);
    ASSERT_EQ(data, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(SpanUtilsTest, FilterSpanFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    auto result = filterSpan(span, [](int x) { return x % 2 == 0; });
    ASSERT_EQ(result, (std::vector<int>{2, 4}));
}

TEST(SpanUtilsTest, CountIfSpanFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_EQ(countIfSpan(span, [](int x) { return x % 2 == 0; }), 2);
}

TEST(SpanUtilsTest, MinElementSpanFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    ASSERT_EQ(minElementSpan(span), 1);
}

TEST(SpanUtilsTest, MaxElementSpanFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    ASSERT_EQ(maxElementSpan(span), 5);
}

TEST(SpanUtilsTest, MaxElementIndexFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    ASSERT_EQ(maxElementIndex(span), 0);
}

TEST(SpanUtilsTest, PrintSpanFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    std::ostringstream oss;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(oss.rdbuf());
    printSpan(span);
    std::cout.rdbuf(oldCoutBuf);
    ASSERT_EQ(oss.str(), "1 2 3 4 5 \n");
}

TEST(SpanUtilsTest, TransposeMatrixFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    std::span<int, 6> span(data);
    transposeMatrix(span, 2, 3);
    ASSERT_EQ(data, (std::vector<int>{1, 4, 2, 5, 3, 6}));
}

TEST(SpanUtilsTest, NormalizeFunction) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::span<double> span(data);
    normalize(span);
    ASSERT_DOUBLE_EQ(data[0], 0.0);
    ASSERT_DOUBLE_EQ(data[4], 1.0);
}

TEST(SpanUtilsTest, MeanFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_DOUBLE_EQ(mean(span), 3.0);
}

TEST(SpanUtilsTest, MedianFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    ASSERT_DOUBLE_EQ(median(span), 3.0);
}

TEST(SpanUtilsTest, ModeFunction) {
    std::vector<int> data = {1, 2, 2, 3, 4};
    std::span<int> span(data);
    ASSERT_EQ(mode(span), 2);
}

TEST(SpanUtilsTest, StandardDeviationFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_DOUBLE_EQ(standardDeviation(span), std::sqrt(2.0));
}

TEST(SpanUtilsTest, TopNElementsFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    auto result = topNElements(span, 3);
    ASSERT_EQ(result, (std::vector<int>{5, 4, 3}));
}

TEST(SpanUtilsTest, VarianceFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    ASSERT_DOUBLE_EQ(variance(span), 2.0);
}

TEST(SpanUtilsTest, BottomNElementsFunction) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    std::span<int> span(data);
    auto result = bottomNElements(span, 3);
    ASSERT_EQ(result, (std::vector<int>{1, 2, 3}));
}

TEST(SpanUtilsTest, CumulativeSumFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    auto result = cumulativeSum(span);
    ASSERT_EQ(result, (std::vector<int>{1, 3, 6, 10, 15}));
}

TEST(SpanUtilsTest, CumulativeProductFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    auto result = cumulativeProduct(span);
    ASSERT_EQ(result, (std::vector<int>{1, 2, 6, 24, 120}));
}

TEST(SpanUtilsTest, FindIndexFunction) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::span<int> span(data);
    auto result = findIndex(span, 3);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), 2);
    result = findIndex(span, 6);
    ASSERT_FALSE(result.has_value());
}

}  // namespace atom::utils::test
