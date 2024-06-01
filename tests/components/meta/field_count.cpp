#include <gtest/gtest.h>
#include "atom/function/filed_count.hpp"

using namespace atom::meta;

// 测试结构体
struct SimpleStruct {
    int a;
    double b;
};

struct NestedStruct {
    int a;
    SimpleStruct b;
    float c;
};

struct ArrayStruct {
    int a[3];
    double b;
};

struct ComplexStruct {
    int a;
    int b[2];
    NestedStruct c;
};

TEST(FieldCountTest, SimpleStructTest) {
    constexpr auto count = field_count_of<SimpleStruct>();
    EXPECT_EQ(count, 2);
}

TEST(FieldCountTest, NestedStructTest) {
    constexpr auto count = field_count_of<NestedStruct>();
    EXPECT_EQ(count, 3);
}

TEST(FieldCountTest, ArrayStructTest) {
    constexpr auto count = field_count_of<ArrayStruct>();
    EXPECT_EQ(count, 2);
}

TEST(FieldCountTest, ComplexStructTest) {
    constexpr auto count = field_count_of<ComplexStruct>();
    EXPECT_EQ(count, 3);
}

TEST(FieldCountTest, EmptyStructTest) {
    struct EmptyStruct {};

    constexpr auto count = field_count_of<EmptyStruct>();
    EXPECT_EQ(count, 0);
}

TEST(FieldCountTest, SingleFieldStructTest) {
    struct SingleFieldStruct {
        int a;
    };

    constexpr auto count = field_count_of<SingleFieldStruct>();
    EXPECT_EQ(count, 1);
}

/*
TEST(FieldCountTest, NonAggregateStructTest) {
    struct NonAggregateStruct {
        NonAggregateStruct(int x) : a(x) {}
        int a;
    };

    // Non-aggregate types should not be counted
    constexpr auto count = field_count_of<NonAggregateStruct>();
    EXPECT_EQ(count, 0);
}
*/
