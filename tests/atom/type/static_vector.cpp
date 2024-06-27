#include "atom/type/static_vector.hpp"
#include <gtest/gtest.h>

template <typename T>
using SV = StaticVector<T, 10>;  // 使用一个固定容量为10的StaticVector来进行测试

// 测试默认构造函数
TEST(StaticVectorTest, DefaultConstructor) {
    SV<int> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.capacity(), 10);
}

// 测试初始化列表构造函数
TEST(StaticVectorTest, InitializerListConstructor) {
    SV<int> vec{1, 2, 3};
    EXPECT_EQ(vec.size(), 3);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

// 测试pushBack
TEST(StaticVectorTest, PushBack) {
    SV<int> vec;
    vec.pushBack(1);
    vec.pushBack(2);

    ASSERT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

// 测试emplaceBack
TEST(StaticVectorTest, EmplaceBack) {
    SV<std::pair<int, int>> vec;
    vec.emplaceBack(1, 2);

    ASSERT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], std::make_pair(1, 2));
}

// 测试popBack
TEST(StaticVectorTest, PopBack) {
    SV<int> vec{1, 2, 3};
    vec.popBack();

    EXPECT_EQ(vec.size(), 2);
    EXPECT_THROW(vec.at(2), std::out_of_range);
}

// 测试下标运算符和at方法
TEST(StaticVectorTest, ElementAccess) {
    SV<int> vec{10, 20, 30};
    EXPECT_EQ(vec[1], 20);
    EXPECT_EQ(vec.at(1), 20);

    EXPECT_THROW(vec.at(3), std::out_of_range);
}

// 测试迭代器
TEST(StaticVectorTest, Iterators) {
    SV<int> vec{1, 2, 3};
    int sum = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 6);
}

// 测试反向迭代器
TEST(StaticVectorTest, ReverseIterators) {
    SV<int> vec{1, 2, 3};
    int sum = 0;
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 6);
}

// 测试比较运算符
TEST(StaticVectorTest, Comparison) {
    SV<int> vec1{1, 2, 3};
    SV<int> vec2{1, 2, 3};
    SV<int> vec3{3, 2, 1};

    EXPECT_TRUE(vec1 == vec2);
    EXPECT_FALSE(vec1 == vec3);
    EXPECT_TRUE(vec1 != vec3);
}

// 测试swap功能
TEST(StaticVectorTest, Swap) {
    SV<int> vec1{1, 2, 3};
    SV<int> vec2{4, 5};
    vec1.swap(vec2);

    EXPECT_EQ(vec1.size(), 2);
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec1[0], 4);
    EXPECT_EQ(vec2[0], 1);
}
