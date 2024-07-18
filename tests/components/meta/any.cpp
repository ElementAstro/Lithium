#include "atom/function/any.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

TEST(BoxedValueTest, BasicTypeTest) {
    BoxedValue intBox = var(42);
    EXPECT_EQ(intBox.getTypeInfo().name(), "int");
    EXPECT_TRUE(intBox.canCast<int>());
    EXPECT_EQ(intBox.tryCast<int>().value(), 42);

    BoxedValue doubleBox = var(3.14);
    EXPECT_EQ(doubleBox.getTypeInfo().name(), "double");
    EXPECT_TRUE(doubleBox.canCast<double>());
    EXPECT_EQ(doubleBox.tryCast<double>().value(), 3.14);
}

/*
TEST(BoxedValueTest, ConstTypeTest) {
    const int CONST_INT = 100;
    BoxedValue constIntBox = constVar(CONST_INT);
    EXPECT_EQ(constIntBox.getTypeInfo().name(),
              "std::reference_wrapper<int const>");
    EXPECT_TRUE(constIntBox.isConst());
    EXPECT_TRUE(constIntBox.canCast<const int>());
    EXPECT_EQ(constIntBox.tryCast<const int>().value(), 100);
}
*/

TEST(BoxedValueTest, VoidTypeTest) {
    BoxedValue voidBox = voidVar();
    EXPECT_TRUE(voidBox.isUndef());
    EXPECT_EQ(voidBox.getTypeInfo().name(), "atom::meta::BoxedValue::VoidType");
}

/*
Max: Fix it
TEST(BoxedValueTest, ReferenceTypeTest) {
    int x = 10;
    BoxedValue refBox = var(std::ref(x));
    EXPECT_TRUE(refBox.isRef());
    EXPECT_FALSE(refBox.canCast<int>());
    EXPECT_EQ(refBox.tryCast<int>().value(), 10);

    x = 20;
    EXPECT_EQ(refBox.tryCast<int>().value(), 20);
}

TEST(BoxedValueTest, AttributeTest) {
    BoxedValue obj = var(42);
    obj.setAttr("name", var(std::string("answer")));
    EXPECT_TRUE(obj.hasAttr("name"));
    EXPECT_FALSE(obj.hasAttr("age"));
    auto objA = obj.getAttr("name");
    EXPECT_TRUE(objA.isType(userType<std::string>()));
    EXPECT_EQ(objA.tryCast<std::string>().value(), "answer");
}
*/

TEST(BoxedValueTest, RemoveAttributeTest) {
    BoxedValue obj = var(42);
    obj.setAttr("name", var(std::string("answer")));
    obj.removeAttr("name");
    EXPECT_FALSE(obj.hasAttr("name"));
}

TEST(BoxedValueTest, ListAttributesTest) {
    BoxedValue obj = var(42);
    obj.setAttr("name", var(std::string("answer")));
    obj.setAttr("value", var(100));

    std::vector<std::string> attrs = obj.listAttrs();
    EXPECT_EQ(attrs.size(), 2);
    EXPECT_TRUE(std::find(attrs.begin(), attrs.end(), "name") != attrs.end());
    EXPECT_TRUE(std::find(attrs.begin(), attrs.end(), "value") != attrs.end());
}

TEST(BoxedValueTest, NullTest) {
    BoxedValue nullBox = voidVar();
    EXPECT_TRUE(nullBox.isUndef());
}

TEST(BoxedValueTest, DebugStringTest) {
    BoxedValue intBox = var(42);
    EXPECT_EQ(intBox.debugString(), "BoxedValue<int>: 42");

    BoxedValue stringBox = var(std::string("hello"));
    EXPECT_EQ(stringBox.debugString(),
              "BoxedValue<std::__cxx11::basic_string<char, "
              "std::char_traits<char>, std::allocator<char> >>: hello");
}

/*
TEST(BoxedValueTest, VisitTest) {
    BoxedValue intBox = var(42);
    int result = 0;
    intBox.visit([&result](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_sameV<T, int>) {
            result = value;
        }
    });
    EXPECT_EQ(result, 42);
*/
