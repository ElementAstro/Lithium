#include "atom/function/bind_first.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper functions and classes for testing
auto freeFunction(int a, int b) -> int { return a + b; }

struct TestClass {
    auto memberFunction(int a, int b) -> int { return a * b; }

    auto constMemberFunction(int a, int b) const -> int { return a - b; }
};

TEST(BindFirstTest, FreeFunctionTest) {
    auto boundFunc = bindFirst(freeFunction, 5);
    EXPECT_EQ(boundFunc(3), 8);
}

TEST(BindFirstTest, MemberFunctionTest) {
    TestClass obj;
    auto boundFunc = bindFirst(&TestClass::memberFunction, obj);
    EXPECT_EQ(boundFunc(4, 2), 8);
}

TEST(BindFirstTest, ConstMemberFunctionTest) {
    const TestClass OBJ;
    auto boundFunc = bindFirst(&TestClass::constMemberFunction, OBJ);
    EXPECT_EQ(boundFunc(7, 3), 4);
}

TEST(BindFirstTest, FunctionObjectTest) {
    auto lambda = [](int a, int b) { return a / b; };
    auto boundFunc = bindFirst(std::function<int(int, int)>(lambda), 10);
    EXPECT_EQ(boundFunc(2), 5);
}

TEST(BindFirstTest, StdFunctionTest) {
    std::function<int(int, int)> func = freeFunction;
    auto boundFunc = bindFirst(func, 6);
    EXPECT_EQ(boundFunc(4), 10);
}

TEST(BindFirstTest, ReferenceWrapperTest) {
    TestClass obj;
    std::reference_wrapper<TestClass> ref(obj);
    auto boundFunc = bindFirst(&TestClass::memberFunction, ref);
    EXPECT_EQ(boundFunc(3, 3), 9);
}

TEST(BindFirstTest, PointerTest) {
    TestClass obj;
    auto boundFunc = bindFirst(&TestClass::memberFunction, &obj);
    EXPECT_EQ(boundFunc(2, 5), 10);
}

TEST(BindFirstTest, ConstPointerTest) {
    const TestClass obj;
    auto boundFunc = bindFirst(&TestClass::constMemberFunction, &obj);
    EXPECT_EQ(boundFunc(10, 5), 5);
}
