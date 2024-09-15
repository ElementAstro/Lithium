#include <gtest/gtest.h>

#include "atom/type/pod_vector.hpp"

using namespace atom::type;

TEST(PodVectorTest, DefaultConstructor) {
    PodVector<int> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_FALSE(vec.empty());
    EXPECT_GE(vec.capacity(), 4);
}

// Test case for initializer list constructor
TEST(PodVectorTest, InitializerListConstructor) {
    PodVector<int> vec = {1, 2, 3, 4};
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[3], 4);
}

// Test case for copy constructor
TEST(PodVectorTest, CopyConstructor) {
    PodVector<int> vec1 = {1, 2, 3, 4};
    PodVector<int> vec2 = vec1;
    EXPECT_EQ(vec2.size(), 4);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
}

// Test case for move constructor
TEST(PodVectorTest, MoveConstructor) {
    PodVector<int> vec1 = {1, 2, 3, 4};
    PodVector<int> vec2 = std::move(vec1);
    EXPECT_EQ(vec2.size(), 4);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
    EXPECT_EQ(vec1.size(), 0);  // vec1 should be empty after move
}

// Test case for pushBack method
TEST(PodVectorTest, PushBack) {
    PodVector<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);
    vec.pushBack(3);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

// Test case for emplaceBack method
TEST(PodVectorTest, EmplaceBack) {
    struct Point {
        int x, y;
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }
    };

    PodVector<Point> vec;
    vec.emplaceBack(1, 2);
    vec.emplaceBack(3, 4);

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], (Point{1, 2}));
    EXPECT_EQ(vec[1], (Point{3, 4}));
}

// Test case for reserve method
TEST(PodVectorTest, Reserve) {
    PodVector<int> vec;
    vec.reserve(100);
    EXPECT_GE(vec.capacity(), 100);
    vec.pushBack(1);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 1);
}

// Test case for resize method
TEST(PodVectorTest, Resize) {
    PodVector<int> vec(10);
    EXPECT_EQ(vec.size(), 10);
    vec.resize(20);
    EXPECT_EQ(vec.size(), 20);
    vec.resize(5);
    EXPECT_EQ(vec.size(), 5);
}

// Test case for popBack method
TEST(PodVectorTest, PopBack) {
    PodVector<int> vec = {1, 2, 3, 4};
    vec.popBack();
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[2], 3);
}

// Test case for popxBack method
TEST(PodVectorTest, PopxBack) {
    PodVector<int> vec = {1, 2, 3, 4};
    int last = vec.popxBack();
    EXPECT_EQ(last, 4);
    EXPECT_EQ(vec.size(), 3);
}

// Test case for erase method
TEST(PodVectorTest, Erase) {
    PodVector<int> vec = {1, 2, 3, 4};
    vec.erase(1);  // Remove the second element
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 3);
    EXPECT_EQ(vec[2], 4);
}

// Test case for insert method
TEST(PodVectorTest, Insert) {
    PodVector<int> vec = {1, 3, 4};
    vec.insert(1, 2);  // Insert 2 at index 1
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[3], 4);
}

// Test case for extend method
TEST(PodVectorTest, Extend) {
    PodVector<int> vec1 = {1, 2, 3};
    PodVector<int> vec2 = {4, 5, 6};
    vec1.extend(vec2);

    EXPECT_EQ(vec1.size(), 6);
    EXPECT_EQ(vec1[0], 1);
    EXPECT_EQ(vec1[1], 2);
    EXPECT_EQ(vec1[2], 3);
    EXPECT_EQ(vec1[3], 4);
    EXPECT_EQ(vec1[4], 5);
    EXPECT_EQ(vec1[5], 6);
}

// Test case for reverse method
TEST(PodVectorTest, Reverse) {
    PodVector<int> vec = {1, 2, 3, 4};
    vec.reverse();
    EXPECT_EQ(vec[0], 4);
    EXPECT_EQ(vec[1], 3);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[3], 1);
}

// Test case for clear method
TEST(PodVectorTest, Clear) {
    PodVector<int> vec = {1, 2, 3, 4};
    vec.clear();
    EXPECT_EQ(vec.size(), 0);
}

// Test case for detach method
TEST(PodVectorTest, Detach) {
    PodVector<int> vec = {1, 2, 3, 4};
    auto [data, size] = vec.detach();
    EXPECT_EQ(size, 4);
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 2);
    EXPECT_EQ(data[2], 3);
    EXPECT_EQ(data[3], 4);
    std::free(data);
}
