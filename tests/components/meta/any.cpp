#include "atom/function/any.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

TEST(BoxedValueTest, BasicTypeTest) {
    BoxedValue intBox = var(42);
    EXPECT_EQ(intBox.get_type_info().name(), "int");
    EXPECT_TRUE(intBox.can_cast<int>());
    EXPECT_EQ(intBox.try_cast<int>().value(), 42);

    BoxedValue doubleBox = var(3.14);
    EXPECT_EQ(doubleBox.get_type_info().name(), "double");
    EXPECT_TRUE(doubleBox.can_cast<double>());
    EXPECT_EQ(doubleBox.try_cast<double>().value(), 3.14);
}

TEST(BoxedValueTest, ConstTypeTest) {
    const int constInt = 100;
    BoxedValue constIntBox = const_var(constInt);
    EXPECT_EQ(constIntBox.get_type_info().name(),
              "std::reference_wrapper<int const>");
    EXPECT_TRUE(constIntBox.is_const());
    EXPECT_TRUE(constIntBox.can_cast<const int>());
    EXPECT_EQ(constIntBox.try_cast<const int>().value(), 100);
}

TEST(BoxedValueTest, VoidTypeTest) {
    BoxedValue voidBox = void_var();
    EXPECT_TRUE(voidBox.is_undef());
    EXPECT_EQ(voidBox.get_type_info().name(),
              "atom::meta::BoxedValue::Void_Type");
}

TEST(BoxedValueTest, ReferenceTypeTest) {
    int x = 10;
    BoxedValue refBox = var(std::ref(x));
    EXPECT_TRUE(refBox.is_ref());
    EXPECT_FALSE(refBox.can_cast<int>());
    EXPECT_EQ(refBox.try_cast<int>().value(), 10);

    x = 20;
    EXPECT_EQ(refBox.try_cast<int>().value(), 20);
}

TEST(BoxedValueTest, AttributeTest) {
    BoxedValue obj = var(42);
    obj.set_attr("name", var(std::string("answer")));
    EXPECT_TRUE(obj.has_attr("name"));
    EXPECT_FALSE(obj.has_attr("age"));
    auto obj_a = obj.get_attr("name");
    EXPECT_TRUE(obj_a.is_type(user_type<std::string>()));
    EXPECT_EQ(obj_a.try_cast<std::string>().value(), "answer");
}

TEST(BoxedValueTest, RemoveAttributeTest) {
    BoxedValue obj = var(42);
    obj.set_attr("name", var(std::string("answer")));
    obj.remove_attr("name");
    EXPECT_FALSE(obj.has_attr("name"));
}

TEST(BoxedValueTest, ListAttributesTest) {
    BoxedValue obj = var(42);
    obj.set_attr("name", var(std::string("answer")));
    obj.set_attr("value", var(100));

    std::vector<std::string> attrs = obj.list_attrs();
    EXPECT_EQ(attrs.size(), 2);
    EXPECT_TRUE(std::find(attrs.begin(), attrs.end(), "name") != attrs.end());
    EXPECT_TRUE(std::find(attrs.begin(), attrs.end(), "value") != attrs.end());
}

TEST(BoxedValueTest, NullTest) {
    BoxedValue nullBox = void_var();
    EXPECT_TRUE(nullBox.is_undef());
}

TEST(BoxedValueTest, DebugStringTest) {
    BoxedValue intBox = var(42);
    EXPECT_EQ(intBox.debug_string(), "BoxedValue<int>: 42");

    BoxedValue stringBox = var(std::string("hello"));
    EXPECT_EQ(stringBox.debug_string(),
              "BoxedValue<std::__cxx11::basic_string<char, "
              "std::char_traits<char>, std::allocator<char> >>: hello");
}

/*
TEST(BoxedValueTest, VisitTest) {
    BoxedValue intBox = var(42);
    int result = 0;
    intBox.visit([&result](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            result = value;
        }
    });
    EXPECT_EQ(result, 42);
*/
