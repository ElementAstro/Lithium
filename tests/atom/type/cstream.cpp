#include "atom/type/cstream.hpp"

#include <gtest/gtest.h>
#include <vector>

using namespace atom::type;

TEST(CStreamTest, SortedTest) {
    std::vector<int> numbers = {3, 1, 4, 1, 5, 9, 2, 6, 5};
    auto stream = makeStream(numbers);
    stream.sorted();
    std::vector<int> expected = {1, 1, 2, 3, 4, 5, 5, 6, 9};
    EXPECT_EQ(stream.getRef(), expected);
}

TEST(CStreamTest, FilterTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto stream = makeStream(numbers);
    auto evenStream = stream.cpFilter([](int n) { return n % 2 == 0; });
    std::vector<int> expected = {2, 4, 6, 8, 10};
    EXPECT_EQ(evenStream.getRef(), expected);
}

TEST(CStreamTest, TransformTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto stream = makeStream(numbers);
    auto transformedStream =
        stream.transform<std::vector<int>>([](int n) { return n * 2; });
    std::vector<int> expected = {2, 4, 6, 8, 10};
    EXPECT_EQ(transformedStream.getRef(), expected);
}

TEST(CStreamTest, AccumulateTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto stream = makeStream(numbers);
    int sum = stream.accumulate();
    EXPECT_EQ(sum, 15);
}

TEST(CStreamTest, RemoveTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto stream = makeStream(numbers);
    stream.remove([](int n) { return n > 5; });
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(stream.getRef(), expected);
}

TEST(CStreamTest, MeanTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto stream = makeStream(numbers);
    double mean = stream.mean();
    EXPECT_DOUBLE_EQ(mean, 3.0);
}

TEST(CStreamTest, FirstTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto stream = makeStream(numbers);
    auto firstGt3 = stream.first([](int n) { return n > 3; });
    ASSERT_TRUE(firstGt3.has_value());
    EXPECT_EQ(firstGt3.value(), 4);
}

TEST(CStreamTest, ContainsTest) {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    auto stream = makeStream(numbers);
    EXPECT_TRUE(stream.contains(3));
    EXPECT_FALSE(stream.contains(6));
}
