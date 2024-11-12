#include <gtest/gtest.h>
#include "qvariant.hpp"

using namespace atom::type;

// Test default constructor
TEST(VariantWrapperTest, DefaultConstructor) {
    VariantWrapper<int, double, std::string> variant;
    EXPECT_EQ(variant.index(), 0);
    EXPECT_FALSE(variant.hasValue());
}

// Test constructor with initial value
TEST(VariantWrapperTest, ConstructorWithValue) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_EQ(variant.index(), 1);
    EXPECT_TRUE(variant.hasValue());
    EXPECT_EQ(variant.get<int>(), 42);
}

// Test copy constructor
TEST(VariantWrapperTest, CopyConstructor) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2(variant1);
    EXPECT_EQ(variant2.index(), 1);
    EXPECT_EQ(variant2.get<int>(), 42);
}

// Test move constructor
TEST(VariantWrapperTest, MoveConstructor) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2(std::move(variant1));
    EXPECT_EQ(variant2.index(), 1);
    EXPECT_EQ(variant2.get<int>(), 42);
}

// Test copy assignment operator
TEST(VariantWrapperTest, CopyAssignmentOperator) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2;
    variant2 = variant1;
    EXPECT_EQ(variant2.index(), 1);
    EXPECT_EQ(variant2.get<int>(), 42);
}

// Test move assignment operator
TEST(VariantWrapperTest, MoveAssignmentOperator) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2;
    variant2 = std::move(variant1);
    EXPECT_EQ(variant2.index(), 1);
    EXPECT_EQ(variant2.get<int>(), 42);
}

// Test assignment operator for a value
TEST(VariantWrapperTest, AssignmentOperatorForValue) {
    VariantWrapper<int, double, std::string> variant;
    variant = 42;
    EXPECT_EQ(variant.index(), 1);
    EXPECT_EQ(variant.get<int>(), 42);
}

// Test typeName method
TEST(VariantWrapperTest, TypeName) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_EQ(variant.typeName(), typeid(int).name());
}

// Test get method
TEST(VariantWrapperTest, Get) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_EQ(variant.get<int>(), 42);
}

// Test is method
TEST(VariantWrapperTest, Is) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_TRUE(variant.is<int>());
    EXPECT_FALSE(variant.is<double>());
}

// Test print method
TEST(VariantWrapperTest, Print) {
    VariantWrapper<int, double, std::string> variant(42);
    testing::internal::CaptureStdout();
    variant.print();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "Current value: 42\n");
}

// Test equality operator
TEST(VariantWrapperTest, EqualityOperator) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2(42);
    EXPECT_TRUE(variant1 == variant2);
}

// Test inequality operator
TEST(VariantWrapperTest, InequalityOperator) {
    VariantWrapper<int, double, std::string> variant1(42);
    VariantWrapper<int, double, std::string> variant2(43);
    EXPECT_TRUE(variant1 != variant2);
}

// Test visit method
/*
// TODO: Fix this test
TEST(VariantWrapperTest, Visit) {
    VariantWrapper<int, double, std::string> variant(42);
    auto result = variant.visit([](auto&& arg) { return arg + 1; });
    EXPECT_EQ(result, 43);
}
*/

// Test index method
TEST(VariantWrapperTest, Index) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_EQ(variant.index(), 1);
}

// Test tryGet method
TEST(VariantWrapperTest, TryGet) {
    VariantWrapper<int, double, std::string> variant(42);
    auto value = variant.tryGet<int>();
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 42);
}

// Test toInt method
TEST(VariantWrapperTest, ToInt) {
    VariantWrapper<int, double, std::string> variant(42);
    auto value = variant.toInt();
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 42);
}

// Test toDouble method
TEST(VariantWrapperTest, ToDouble) {
    VariantWrapper<int, double, std::string> variant(42.0);
    auto value = variant.toDouble();
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 42.0);
}

// Test toBool method
TEST(VariantWrapperTest, ToBool) {
    VariantWrapper<int, double, std::string> variant(true);
    auto value = variant.toBool();
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), true);
}

// Test toString method
TEST(VariantWrapperTest, ToString) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_EQ(variant.toString(), "42");
}

// Test reset method
TEST(VariantWrapperTest, Reset) {
    VariantWrapper<int, double, std::string> variant(42);
    variant.reset();
    EXPECT_EQ(variant.index(), 0);
    EXPECT_FALSE(variant.hasValue());
}

// Test hasValue method
TEST(VariantWrapperTest, HasValue) {
    VariantWrapper<int, double, std::string> variant(42);
    EXPECT_TRUE(variant.hasValue());
    variant.reset();
    EXPECT_FALSE(variant.hasValue());
}

// Test stream insertion operator
TEST(VariantWrapperTest, StreamInsertionOperator) {
    VariantWrapper<int, double, std::string> variant(42);
    std::ostringstream oss;
    oss << variant;
    EXPECT_EQ(oss.str(), "Current value: 42\n");
}