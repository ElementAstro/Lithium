#include <gtest/gtest.h>

#include "atom/extra/inicpp/field.hpp"

#include <stdexcept>
#include <string>

using namespace inicpp;

// Test default constructor
TEST(IniFieldTest, DefaultConstructor) {
    IniField field;
    EXPECT_NO_THROW(field.as<std::string>());
}

// Test constructor with value
TEST(IniFieldTest, ConstructorWithValue) {
    IniField field("test_value");
    EXPECT_EQ(field.as<std::string>(), "test_value");
}

// Test copy constructor
TEST(IniFieldTest, CopyConstructor) {
    IniField field1("test_value");
    IniField field2(field1);
    EXPECT_EQ(field2.as<std::string>(), "test_value");
}

// Test as method for different types
TEST(IniFieldTest, AsMethod) {
    IniField field1("true");
    EXPECT_EQ(field1.as<bool>(), true);

    IniField field2("A");
    EXPECT_EQ(field2.as<char>(), 'A');

    IniField field3("123");
    EXPECT_EQ(field3.as<int>(), 123);

    IniField field4("123.45");
    EXPECT_EQ(field4.as<double>(), 123.45);

    IniField field5("test_string");
    EXPECT_EQ(field5.as<std::string>(), "test_string");
}

// Test as method with invalid conversion
TEST(IniFieldTest, AsMethodInvalidConversion) {
    IniField field("invalid");
    EXPECT_THROW(field.as<int>(), std::invalid_argument);
}

// Test assignment operator for different types
TEST(IniFieldTest, AssignmentOperator) {
    IniField field;
    field = true;
    EXPECT_EQ(field.as<bool>(), true);

    field = 'A';
    EXPECT_EQ(field.as<char>(), 'A');

    field = 123;
    EXPECT_EQ(field.as<int>(), 123);

    field = 123.45;
    EXPECT_EQ(field.as<double>(), 123.45);

    field = std::string("test_string");
    EXPECT_EQ(field.as<std::string>(), "test_string");
}

// Test copy assignment operator
TEST(IniFieldTest, CopyAssignmentOperator) {
    IniField field1("test_value");
    IniField field2;
    field2 = field1;
    EXPECT_EQ(field2.as<std::string>(), "test_value");
}
