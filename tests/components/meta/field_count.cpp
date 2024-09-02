#include <gtest/gtest.h>
#include "atom/function/field_count.hpp"

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
    constexpr auto COUNT = fieldCountOf<SimpleStruct>();
    EXPECT_EQ(COUNT, 2);
}

TEST(FieldCountTest, NestedStructTest) {
    constexpr auto COUNT = fieldCountOf<NestedStruct>();
    EXPECT_EQ(COUNT, 3);
}

TEST(FieldCountTest, ArrayStructTest) {
    constexpr auto COUNT = fieldCountOf<ArrayStruct>();
    EXPECT_EQ(COUNT, 4);
}

TEST(FieldCountTest, ComplexStructTest) {
    constexpr auto COUNT = fieldCountOf<ComplexStruct>();
    EXPECT_EQ(COUNT, 4);
}

TEST(FieldCountTest, EmptyStructTest) {
    struct EmptyStruct {};

    constexpr auto COUNT = fieldCountOf<EmptyStruct>();
    EXPECT_EQ(COUNT, 0);
}

TEST(FieldCountTest, SingleFieldStructTest) {
    struct SingleFieldStruct {
        int a;
    };

    constexpr auto COUNT = fieldCountOf<SingleFieldStruct>();
    EXPECT_EQ(COUNT, 1);
}

TEST(FieldCountTest, NonAggregateStructTest) {
    struct NonAggregateStruct {
        NonAggregateStruct(int x) : a(x) {}
        int a;
    };

    // Non-aggregate types should not be counted
    constexpr auto count = fieldCountOf<NonAggregateStruct>();
    EXPECT_EQ(count, 0);
}
