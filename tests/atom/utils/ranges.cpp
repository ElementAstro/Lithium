#include "atom/utils/ranges.hpp"
#include <gtest/gtest.h>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;
using namespace atom::utils;

TEST(RangesTest, FilterAndTransform) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = filter_and_transform(
        numbers, [](int x) { return x % 2 == 0; }, [](int x) { return x * 2; });

    std::vector<int> expected = {4, 8, 12, 16, 20};
    EXPECT_EQ(std::vector<int>(result.begin(), result.end()), expected);
}

TEST(RangesTest, FindElement) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = find_element(numbers, 3);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3);

    auto notFound = find_element(numbers, 6);
    EXPECT_FALSE(notFound.has_value());
}

TEST(RangesTest, GroupAndAggregate) {
    std::vector<std::pair<std::string, int>> data = {{"apple"s, 2},
                                                     {"banana"s, 3},
                                                     {"apple"s, 1},
                                                     {"cherry"s, 4},
                                                     {"banana"s, 1}};
    auto fruit_counts = group_and_aggregate(
        data, [](const auto& pair) { return pair.first; },
        [](const auto& pair) { return pair.second; });

    std::map<std::string, int> expected = {
        {"apple"s, 3}, {"banana"s, 4}, {"cherry"s, 4}};
    EXPECT_EQ(fruit_counts, expected);
}

TEST(RangesTest, Drop) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = drop(numbers, 2);

    std::vector<int> expected = {3, 4, 5};
    EXPECT_EQ(std::vector<int>(result.begin(), result.end()), expected);
}

TEST(RangesTest, Take) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = take(numbers, 3);

    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(std::vector<int>(result.begin(), result.end()), expected);
}

TEST(RangesTest, TakeWhile) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result_view = take_while(numbers, [](int x) { return x < 4; });

    std::vector<int> result;
    std::ranges::copy(result_view, std::back_inserter(result));

    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(result, expected);
}

TEST(RangesTest, DropWhile) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = drop_while(numbers, [](int x) { return x <= 2; });

    std::vector<int> expected = {3, 4, 5};
    EXPECT_EQ(std::vector<int>(result.begin(), result.end()), expected);
}

TEST(RangesTest, Reverse) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = reverse(numbers);

    std::vector<int> expected = {5, 4, 3, 2, 1};
    EXPECT_EQ(std::vector<int>(result.begin(), result.end()), expected);
}

TEST(RangesTest, Accumulate) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto sum = accumulate(numbers, 0, std::plus<>{});
    EXPECT_EQ(sum, 15);
}

TEST(RangesTest, Slice) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = Slice(numbers.begin(), numbers.end(), 2, 4);

    std::vector<int> expected = {3, 4, 5, 6};
    EXPECT_EQ(result, expected);
}

TEST(RangesTest, SliceContainer) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = slice(numbers, 2, 6);

    std::vector<int> expected = {3, 4, 5, 6};
    EXPECT_EQ(result, expected);
}

class ViewsTest : public ::testing::Test {
protected:
    std::vector<int> v1 = {1, 2, 3, 4, 5};
    std::vector<int> v2 = {10, 20, 30, 40, 50};
    std::array<int, 3> a = {1, 3, 7};
    std::array<int, 4> b = {2, 4, 5, 9};
};

TEST_F(ViewsTest, MergeViewTest) {
    std::vector<int> expected = {1, 2, 3, 4, 5, 7, 9};
    std::vector<int> result;

    for (auto i : merge_view(a, b)) {
        result.push_back(i);
    }

    EXPECT_EQ(result, expected);
}

TEST_F(ViewsTest, ZipViewTest) {
    std::vector<std::pair<int, int>> expected = {
        {1, 10}, {2, 20}, {3, 30}, {4, 40}, {5, 50}};
    std::vector<std::pair<int, int>> result;

    for (auto [a, b] : zip_view(v1, v2)) {
        result.emplace_back(a, b);
    }

    EXPECT_EQ(result, expected);
}

TEST_F(ViewsTest, ChunkViewTest) {
    std::vector<std::vector<int>> expected = {{1, 2}, {3, 4}, {5}};
    std::vector<std::vector<int>> result;

    for (auto chunk : chunk_view(v1, 2)) {
        result.push_back(chunk);
    }

    EXPECT_EQ(result, expected);
}

TEST_F(ViewsTest, FilterViewTest) {
    std::vector<int> expected = {2, 4};
    std::vector<int> result;

    for (auto i : filter_view(v1, [](int x) { return x % 2 == 0; })) {
        result.push_back(i);
    }

    EXPECT_EQ(result, expected);
}

TEST_F(ViewsTest, TransformViewTest) {
    std::vector<int> expected = {1, 4, 9, 16, 25};
    std::vector<int> result;

    for (auto i : transform_view(v1, [](int x) { return x * x; })) {
        result.push_back(i);
    }

    EXPECT_EQ(result, expected);
}

TEST_F(ViewsTest, AdjacentViewTest) {
    std::vector<std::pair<int, int>> expected = {
        {1, 2}, {2, 3}, {3, 4}, {4, 5}};
    std::vector<std::pair<int, int>> result;

    for (auto [a, b] : adjacent_view(v1)) {
        result.emplace_back(a, b);
    }

    EXPECT_EQ(result, expected);
}

// 测试不同类型的组合
TEST_F(ViewsTest, MixedTypeTest) {
    std::vector<double> doubles = {1.1, 2.2, 3.3};
    std::list<int> ints = {1, 2, 3};

    auto result = zip_view(doubles, ints);
    int count = 0;
    for (auto [d, i] : result) {
        EXPECT_DOUBLE_EQ(d, static_cast<double>(i) + 0.1);
        count++;
    }
    EXPECT_EQ(count, 3);
}