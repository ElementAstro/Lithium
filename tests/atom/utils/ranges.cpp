#include "atom/utils/ranges.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace atom::utils;
using namespace std::string_literals;

TEST(FilterAndTransformTest, FiltersAndTransformsElements) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = filterAndTransform(
        numbers, [](int x) { return x % 2 == 0; }, [](int x) { return x * 2; });
    std::vector<int> expected = {4, 8, 12, 16, 20};
    std::vector<int> actual(result.begin(), result.end());
    EXPECT_EQ(actual, expected);
}

TEST(FindElementTest, FindsExistingElement) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = findElement(numbers, 3);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 3);
}

TEST(FindElementTest, DoesNotFindNonExistingElement) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = findElement(numbers, 6);
    EXPECT_FALSE(result.has_value());
}

TEST(GroupAndAggregateTest, GroupsAndAggregatesElements) {
    std::vector<std::pair<std::string, int>> data = {{"apple", 2},
                                                     {"banana", 3},
                                                     {"apple", 1},
                                                     {"cherry", 4},
                                                     {"banana", 1}};
    auto result = groupAndAggregate(
        data, [](const auto& pair) { return pair.first; },
        [](const auto& pair) { return pair.second; });
    std::map<std::string, int> expected = {
        {"apple", 3}, {"banana", 4}, {"cherry", 4}};
    EXPECT_EQ(result, expected);
}

TEST(DropTest, DropsFirstNElements) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = drop(numbers, 3);
    std::vector<int> expected = {4, 5};
    std::vector<int> actual(result.begin(), result.end());
    EXPECT_EQ(actual, expected);
}

TEST(TakeTest, TakesFirstNElements) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = take(numbers, 3);
    std::vector<int> expected = {1, 2, 3};
    std::vector<int> actual(result.begin(), result.end());
    EXPECT_EQ(actual, expected);
}

TEST(TakeWhileTest, TakesElementsWhilePredicateIsTrue) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    std::vector<int> expected = {1, 2, 3};
    std::vector<int> actual =
        toVector(takeWhile(numbers, [](int x) { return x < 4; }));
    EXPECT_EQ(actual, expected);
}

TEST(DropWhileTest, DropsElementsWhilePredicateIsTrue) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    auto result = dropWhile(numbers, [](int x) { return x < 4; });
    std::vector<int> expected = {4, 5, 6};
    std::vector<int> actual(result.begin(), result.end());
    EXPECT_EQ(actual, expected);
}

TEST(ReverseTest, ReversesElementsInRange) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = reverse(numbers);
    std::vector<int> expected = {5, 4, 3, 2, 1};
    std::vector<int> actual(result.begin(), result.end());
    EXPECT_EQ(actual, expected);
}

TEST(AccumulateTest, AccumulatesElements) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = accumulate(numbers, 0, std::plus<>{});
    int expected = 15;
    EXPECT_EQ(result, expected);
}

TEST(SliceTest, SlicesElementsFromRange) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = slice(numbers, 1, 4);
    std::vector<int> expected = {2, 3, 4};
    EXPECT_EQ(result, expected);
}

TEST(SliceTest, SlicesElementsFromRangeWithIterator) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = slice(numbers.begin(), numbers.end(), 1, 3);
    std::vector<int> expected = {2, 3, 4};
    EXPECT_EQ(result, expected);
}

TEST(MergeViewTest, MergesTwoRanges) {
    std::vector<int> range1 = {1, 3, 5};
    std::vector<int> range2 = {2, 4, 6};
    std::vector<int> expected = {1, 2, 3, 4, 5, 6};
    MergeViewImpl mergeView;
    auto result = mergeView(range1, range2);
    for (const auto i : result) {
        EXPECT_EQ(i, expected[i - 1]);
    }
}

TEST(ZipViewTest, ZipsMultipleRanges) {
    std::vector<int> range1 = {1, 2, 3};
    std::vector<std::string> range2 = {"a", "b", "c"};
    auto result = ZipViewImpl{}(range1, range2);
    std::vector<std::tuple<int, std::string>> expected = {
        {1, "a"}, {2, "b"}, {3, "c"}};
    std::vector<std::tuple<int, std::string>> actual;
    for (const auto& i : result) {
        actual.push_back(i);
    }
    EXPECT_EQ(actual, expected);
}

TEST(ChunkViewTest, ChunksRangeIntoSmallerRanges) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto result = ChunkViewImpl{}(numbers, 3);
    std::vector<std::vector<int>> expected = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    std::vector<std::vector<int>> actual;
    for (const auto& i : result) {
        actual.push_back(toVector(i));
    }
    EXPECT_EQ(actual, expected);
}

TEST(FilterViewTest, FiltersElementsInRange) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = FilterViewImpl{}(numbers, [](int x) { return x % 2 == 0; });
    std::vector<int> expected = {2, 4};
    std::vector<int> actual;
    for (const auto i : result) {
        actual.push_back(i);
    }
    EXPECT_EQ(actual, expected);
}

TEST(TransformViewTest, TransformsElementsInRange) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = TransformViewImpl{}(numbers, [](int x) { return x * 2; });
    std::vector<int> expected = {2, 4, 6, 8, 10};
    std::vector<int> actual;
    for (const auto i : result) {
        actual.push_back(i);
    }
    EXPECT_EQ(actual, expected);
}

TEST(AdjacentViewTest, ProducesAdjacentPairsInRange) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto result = AdjacentViewImpl{}(numbers);
    std::vector<std::pair<int, int>> expected = {
        {1, 2}, {2, 3}, {3, 4}, {4, 5}};
    std::vector<std::pair<int, int>> actual;
    for (const auto& i : result) {
        actual.push_back(i);
    }
    EXPECT_EQ(actual, expected);
}
