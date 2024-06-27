#include "atom/type/small_vector.hpp"
#include <gtest/gtest.h>

TEST(SmallVectorTest, DefaultConstructor) {
    SmallVector<int, 3> v;
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 3);
}

TEST(SmallVectorTest, ConstructorWithInitializerList) {
    SmallVector<int, 3> v{1, 2, 3};
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(SmallVectorTest, CopyConstructor) {
    SmallVector<int, 3> v1{1, 2, 3};
    SmallVector<int, 3> v2(v1);
    EXPECT_EQ(v1, v2);
}

TEST(SmallVectorTest, MoveConstructor) {
    SmallVector<int, 3> v1{1, 2, 3};
    SmallVector<int, 3> v2(std::move(v1));
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2.capacity(), 3);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v2[2], 3);
}

TEST(SmallVectorTest, AssignmentOperator) {
    SmallVector<int, 3> v1{1, 2, 3};
    SmallVector<int, 3> v2;
    v2 = v1;
    EXPECT_EQ(v1, v2);
}

TEST(SmallVectorTest, MoveAssignmentOperator) {
    SmallVector<int, 3> v1{1, 2, 3};
    SmallVector<int, 3> v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2.capacity(), 3);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v2[2], 3);
}

TEST(SmallVectorTest, At) {
    SmallVector<int, 3> v{1, 2, 3};
    EXPECT_EQ(v.at(0), 1);
    EXPECT_EQ(v.at(1), 2);
    EXPECT_EQ(v.at(2), 3);
    EXPECT_THROW(v.at(3), std::out_of_range);
}

TEST(SmallVectorTest, OperatorIndex) {
    SmallVector<int, 3> v{1, 2, 3};
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(SmallVectorTest, Front) {
    SmallVector<int, 3> v{1, 2, 3};
    EXPECT_EQ(v.front(), 1);
}

TEST(SmallVectorTest, Back) {
    SmallVector<int, 3> v{1, 2, 3};
    EXPECT_EQ(v.back(), 3);
}

TEST(SmallVectorTest, Data) {
    SmallVector<int, 3> v{1, 2, 3};
    int* data = v.data();
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 2);
    EXPECT_EQ(data[2], 3);
}

TEST(SmallVectorTest, BeginEnd) {
    SmallVector<int, 3> v{1, 2, 3};
    auto begin = v.begin();
    auto end = v.end();
    EXPECT_EQ(*begin, 1);
    EXPECT_EQ(*(begin + 1), 2);
    EXPECT_EQ(*(begin + 2), 3);
    EXPECT_EQ(end - begin, 3);
}

TEST(SmallVectorTest, Clear) {
    SmallVector<int, 3> v{1, 2, 3};
    v.clear();
    EXPECT_EQ(v.size(), 0);
}

TEST(SmallVectorTest, Reserve) {
    SmallVector<int, 3> v;
    v.reserve(5);
    EXPECT_EQ(v.capacity(), 5);
}

TEST(SmallVectorTest, EmplaceBack) {
    SmallVector<int, 3> v;
    v.emplaceBack(1);
    v.emplaceBack(2);
    v.emplaceBack(3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(SmallVectorTest, PushBack) {
    SmallVector<int, 3> v;
    v.pushBack(1);
    v.pushBack(2);
    v.pushBack(3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(SmallVectorTest, PopBack) {
    SmallVector<int, 3> v{1, 2, 3};
    v.popBack();
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(SmallVectorTest, InsertValue) {
    SmallVector<int, 3> v{1, 3, 4};
    v.insert(v.begin() + 1, 2);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
}

TEST(SmallVectorTest, InsertRange) {
    SmallVector<int, 3> v{1, 3, 4};
    SmallVector<int, 3> v2{2, 5};
    v.insert(v.begin() + 1, v2.begin(), v2.end());
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 5);
    EXPECT_EQ(v[3], 3);
    EXPECT_EQ(v[4], 4);
}

TEST(SmallVectorTest, InsertInitializerList) {
    SmallVector<int, 3> v{1, 3, 4};
    v.insert(v.begin() + 1, {2, 5});
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 5);
    EXPECT_EQ(v[3], 3);
    EXPECT_EQ(v[4], 4);
}

TEST(SmallVectorTest, EraseSingleElement) {
    SmallVector<int, 3> v{1, 2, 3};
    v.erase(v.begin() + 1);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v.capacity(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
}

TEST(SmallVectorTest, EraseRange) {
    SmallVector<int, 3> v{1, 2, 3, 4, 5};
    v.erase(v.begin() + 1, v.begin() + 4);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.capacity(), 5);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 5);
}

TEST(SmallVectorTest, Resize) {
    SmallVector<int, 3> v{1, 2, 3};
    v.resize(5, 4);
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity(), 5);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 4);
}

TEST(SmallVectorTest, Swap) {
    SmallVector<int, 3> v1{1, 2, 3};
    SmallVector<int, 3> v2{4, 5, 6};
    v1.swap(v2);
    EXPECT_EQ(v1.size(), 3);
    EXPECT_EQ(v1.capacity(), 3);
    EXPECT_EQ(v1[0], 4);
    EXPECT_EQ(v1[1], 5);
    EXPECT_EQ(v1[2], 6);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2.capacity(), 3);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v2[2], 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}