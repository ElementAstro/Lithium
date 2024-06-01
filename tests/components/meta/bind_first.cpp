#include "atom/function/bind_first.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper functions and classes for testing
int free_function(int a, int b) { return a + b; }

struct TestClass {
    int member_function(int a, int b) { return a * b; }

    int const_member_function(int a, int b) const { return a - b; }
};

TEST(BindFirstTest, FreeFunctionTest) {
    auto bound_func = bind_first(free_function, 5);
    EXPECT_EQ(bound_func(3), 8);
}

TEST(BindFirstTest, MemberFunctionTest) {
    TestClass obj;
    auto bound_func = bind_first(&TestClass::member_function, obj);
    EXPECT_EQ(bound_func(4, 2), 8);
}

TEST(BindFirstTest, ConstMemberFunctionTest) {
    const TestClass obj;
    auto bound_func = bind_first(&TestClass::const_member_function, obj);
    EXPECT_EQ(bound_func(7, 3), 4);
}

TEST(BindFirstTest, FunctionObjectTest) {
    auto lambda = [](int a, int b) { return a / b; };
    auto bound_func = bind_first(std::function<int(int, int)>(lambda), 10);
    EXPECT_EQ(bound_func(2), 5);
}

TEST(BindFirstTest, StdFunctionTest) {
    std::function<int(int, int)> func = free_function;
    auto bound_func = bind_first(func, 6);
    EXPECT_EQ(bound_func(4), 10);
}

TEST(BindFirstTest, ReferenceWrapperTest) {
    TestClass obj;
    std::reference_wrapper<TestClass> ref(obj);
    auto bound_func = bind_first(&TestClass::member_function, ref);
    EXPECT_EQ(bound_func(3, 3), 9);
}

TEST(BindFirstTest, PointerTest) {
    TestClass obj;
    auto bound_func = bind_first(&TestClass::member_function, &obj);
    EXPECT_EQ(bound_func(2, 5), 10);
}

TEST(BindFirstTest, ConstPointerTest) {
    const TestClass obj;
    auto bound_func = bind_first(&TestClass::const_member_function, &obj);
    EXPECT_EQ(bound_func(10, 5), 5);
}
