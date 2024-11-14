#include <gtest/gtest.h>

#include "atom/function/field_count.hpp"

namespace atom::meta::test {

// Test helper types
struct Empty {};

struct OneField {
    int x;
};

struct TwoFields {
    int x;
    double y;
};

struct ThreeFields {
    int x;
    double y;
    char z;
};

struct NestedStruct {
    int x;
    TwoFields nested;
    double z;
};

struct WithArray {
    int arr[3];
    double x;
};

struct WithNestedArray {
    int x;
    int arr[2][3];
    double y;
};

class NonAggregate {
    int x;
public:
    NonAggregate() : x(0) {}
};

struct DerivedEmpty : Empty {
    int x;
};

struct ComplexNested {
    TwoFields a;
    ThreeFields b;
    Empty c;
};

class FieldCountTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Basic field counting tests
TEST_F(FieldCountTest, EmptyStructHasZeroFields) {
    constexpr auto count = fieldCountOf<Empty>();
    EXPECT_EQ(count, 0);
}

TEST_F(FieldCountTest, BasicStructFieldCounts) {
    constexpr auto one = fieldCountOf<OneField>();
    constexpr auto two = fieldCountOf<TwoFields>();
    constexpr auto three = fieldCountOf<ThreeFields>();
    
    EXPECT_EQ(one, 1);
    EXPECT_EQ(two, 2);
    EXPECT_EQ(three, 3);
}

// Nested struct tests
TEST_F(FieldCountTest, NestedStructFieldCount) {
    constexpr auto count = fieldCountOf<NestedStruct>();
    EXPECT_EQ(count, 3); // x, nested, z
}

TEST_F(FieldCountTest, ComplexNestedStructFieldCount) {
    constexpr auto count = fieldCountOf<ComplexNested>();
    EXPECT_EQ(count, 3); // a, b, c
}

// Array member tests
TEST_F(FieldCountTest, StructWithArrayFieldCount) {
    constexpr auto count = fieldCountOf<WithArray>();
    EXPECT_EQ(count, 2); // arr, x
}

TEST_F(FieldCountTest, StructWithNestedArrayFieldCount) {
    constexpr auto count = fieldCountOf<WithNestedArray>();
    EXPECT_EQ(count, 3); // x, arr, y
}

// Non-aggregate type tests
TEST_F(FieldCountTest, NonAggregateTypeHasZeroFields) {
    constexpr auto count = fieldCountOf<NonAggregate>();
    EXPECT_EQ(count, 0);
}

// Inheritance tests
TEST_F(FieldCountTest, DerivedEmptyStructFieldCount) {
    constexpr auto count = fieldCountOf<DerivedEmpty>();
    EXPECT_EQ(count, 1); // Only x, Empty base has no fields
}

// Edge cases
struct BitFields {
    int x : 1;
    int y : 2;
    int z : 3;
};

TEST_F(FieldCountTest, BitFieldsCount) {
    constexpr auto count = fieldCountOf<BitFields>();
    EXPECT_EQ(count, 3);
}

// Custom type_info specialization test
struct CustomStruct {
    int a, b, c;
};

TEST_F(FieldCountTest, CustomTypeInfoSpecialization) {
    constexpr auto count = fieldCountOf<CustomStruct>();
    EXPECT_EQ(count, 3);
}

// Compile-time evaluation test
TEST_F(FieldCountTest, CompileTimeEvaluation) {
    static_assert(fieldCountOf<Empty>() == 0);
    static_assert(fieldCountOf<OneField>() == 1);
    static_assert(fieldCountOf<TwoFields>() == 2);
    static_assert(fieldCountOf<ThreeFields>() == 3);
}

// Type trait tests
TEST_F(FieldCountTest, TypeTraitBehavior) {
    static_assert(std::is_aggregate_v<Empty>);
    static_assert(std::is_aggregate_v<TwoFields>);
    static_assert(!std::is_aggregate_v<NonAggregate>);
}

// Additional edge cases
struct EmptyArrayStruct {
    int arr[0];
};

struct ZeroLengthArrayStruct {
    int x;
    int arr[];
};

TEST_F(FieldCountTest, SpecialArrayCases) {
    constexpr auto empty_arr_count = fieldCountOf<EmptyArrayStruct>();
    EXPECT_EQ(empty_arr_count, 1);
    
    constexpr auto zero_length_count = fieldCountOf<ZeroLengthArrayStruct>();
    EXPECT_EQ(zero_length_count, 2);
}

// Stress test with many fields
struct ManyFields {
    int f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
};

TEST_F(FieldCountTest, LargeStructFieldCount) {
    constexpr auto count = fieldCountOf<ManyFields>();
    EXPECT_EQ(count, 20);
}

} // namespace atom::meta::test