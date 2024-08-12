#include <gtest/gtest.h>
#include <functional>
#include <iostream>
#include <string>
#include "atom/function/property.hpp"

// Test suite for the Property class
class PropertyTest : public ::testing::Test {
protected:
    Property<int> prop_int;
    Property<std::string> prop_string;
};

// Test default constructor and assignment
TEST_F(PropertyTest, DefaultConstructor) {
    Property<int> prop;
    EXPECT_THROW(static_cast<int>(prop), atom::error::Exception);

    Property<std::string> prop_str;
    EXPECT_THROW(static_cast<std::string>(prop_str), atom::error::Exception);
}

// Test setting and getting a simple value
TEST_F(PropertyTest, SetGetValue) {
    prop_int = 10;
    EXPECT_EQ(static_cast<int>(prop_int), 10);

    prop_string = "Hello";
    EXPECT_EQ(static_cast<std::string>(prop_string), "Hello");
}

// Test property with a getter
/* TODO: FIX ME
TEST_F(PropertyTest, GetterOnly) {
    int value = 5;
    Property<int> readonly_prop([&value]() { return value; });

    EXPECT_EQ(static_cast<int>(readonly_prop), 5);
    value = 10;
    EXPECT_EQ(static_cast<int>(readonly_prop), 10);

    // Expect exception when trying to assign a value
    EXPECT_THROW(readonly_prop = 20, atom::error::Exception);
}
*/

// Test property with a setter
TEST_F(PropertyTest, SetterOnly) {
    int value = 0;
    Property<int> writeonly_prop(nullptr, [&value](int newValue) { value = newValue; });

    writeonly_prop = 15;
    EXPECT_EQ(value, 15);

    // Expect exception when trying to get a value
    EXPECT_THROW(static_cast<int>(writeonly_prop), atom::error::Exception);
}

// Test property with both getter and setter
TEST_F(PropertyTest, GetterAndSetter) {
    int value = 0;
    Property<int> prop([&value]() { return value; },
                       [&value](int newValue) { value = newValue; });

    prop = 20;
    EXPECT_EQ(static_cast<int>(prop), 20);
    EXPECT_EQ(value, 20);
}

// Test property with onChange callback
TEST_F(PropertyTest, OnChangeCallback) {
    int value = 0;
    int callbackValue = 0;
    prop_int = Property<int>([&value]() { return value; },
                             [&value](int newValue) { value = newValue; });
    prop_int.setOnChange([&callbackValue](const int& newValue) {
        callbackValue = newValue;
    });

    prop_int = 30;
    EXPECT_EQ(callbackValue, 30);
}

/* TODO: FIX ME
// Test makeReadonly and makeWriteonly
TEST_F(PropertyTest, MakeReadonlyWriteonly) {
    prop_int = 40;
    prop_int.makeReadonly();
    EXPECT_EQ(static_cast<int>(prop_int), 40);
    EXPECT_THROW(prop_int = 50, atom::error::Exception);

    prop_int.makeWriteonly();
    EXPECT_THROW(static_cast<int>(prop_int), atom::error::Exception);
}
*/


// Test move constructor and assignment
TEST_F(PropertyTest, MoveConstructorAndAssignment) {
    Property<int> original(100);
    Property<int> moved(std::move(original));
    EXPECT_EQ(static_cast<int>(moved), 100);

    Property<int> another = std::move(moved);
    EXPECT_EQ(static_cast<int>(another), 100);
}

// Test arithmetic operators
TEST_F(PropertyTest, ArithmeticOperators) {
    prop_int = 5;
    prop_int += 10;
    EXPECT_EQ(static_cast<int>(prop_int), 15);

    prop_int -= 5;
    EXPECT_EQ(static_cast<int>(prop_int), 10);

    prop_int *= 2;
    EXPECT_EQ(static_cast<int>(prop_int), 20);

    prop_int /= 4;
    EXPECT_EQ(static_cast<int>(prop_int), 5);

    prop_int %= 3;
    EXPECT_EQ(static_cast<int>(prop_int), 2);
}

// Test comparison operators
TEST_F(PropertyTest, ComparisonOperators) {
    prop_int = 10;
    EXPECT_TRUE(prop_int == 10);
    EXPECT_TRUE(prop_int != 5);
    EXPECT_TRUE(prop_int <=> 10 == std::strong_ordering::equal);
    EXPECT_TRUE(prop_int <=> 5 == std::strong_ordering::greater);
    EXPECT_TRUE(prop_int <=> 15 == std::strong_ordering::less);
}

// Test stream output operator
TEST_F(PropertyTest, StreamOutputOperator) {
    prop_int = 42;
    std::ostringstream os;
    os << prop_int;
    EXPECT_EQ(os.str(), "42");
}

// Test defining read-write property using macro
TEST_F(PropertyTest, DefineRWPropertyMacro) {
    struct TestClass {
        DEFINE_RW_PROPERTY(int, Value)
    } obj;

    obj.Value = 10;
    EXPECT_EQ(static_cast<int>(obj.Value), 10);

    obj.Value += 5;
    EXPECT_EQ(static_cast<int>(obj.Value), 15);
}

/* TODO: FIX ME
// Test defining read-only property using macro
TEST_F(PropertyTest, DefineROPropertyMacro) {
    struct TestClass {
        DEFINE_RO_PROPERTY(int, Value)
        TestClass(int value) : Value(value) {}
    } obj(20);

    EXPECT_EQ(static_cast<int>(obj.Value), 20);
    EXPECT_THROW(obj.Value = 30, atom::error::Exception);
}
*/

// Test defining write-only property using macro
TEST_F(PropertyTest, DefineWOPropertyMacro) {
    struct TestClass {
        DEFINE_WO_PROPERTY(int, Value)
    } obj;

    EXPECT_THROW(static_cast<int>(obj.Value), atom::error::Exception);
    obj.Value = 25;
}
