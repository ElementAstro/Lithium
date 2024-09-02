#include "atom/function/func_traits.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper functions for testing
int freeFunction(int a, double b) { return a + static_cast<int>(b); }

struct TestClass {
    auto memberFunction(int a, double b) -> int {
        return a + static_cast<int>(b);
    }
    auto constMemberFunction(int a, double b) const -> int {
        return a + static_cast<int>(b);
    }
    auto volatileMemberFunction(int a, double b) volatile -> int {
        return a + static_cast<int>(b);
    }
    auto constVolatileMemberFunction(int a, double b) const volatile -> int {
        return a + static_cast<int>(b);
    }
    auto lvalueReferenceMemberFunction(int a, double b) & -> int {
        return a + static_cast<int>(b);
    }
    auto rvalueReferenceMemberFunction(int a, double b) && -> int {
        return a + static_cast<int>(b);
    }
    auto noexceptMemberFunction(int a, double b) noexcept -> int {
        return a + static_cast<int>(b);
    }
    auto variadicMemberFunction(int a, ...) -> int { return a; }
};

TEST(FunctionTraitsTest, FreeFunction) {
    using Traits = FunctionTraits<decltype(freeFunction)>;

    static_assert(std::is_same_v<Traits::return_type, int>);
    static_assert(std::is_same_v<Traits::argument_t<0>, int>);
    static_assert(std::is_same_v<Traits::argument_t<1>, double>);
    static_assert(Traits::arity == 2);
    static_assert(!Traits::is_member_function);

    EXPECT_EQ(Traits::full_name, "int (int, double)");
}

TEST(FunctionTraitsTest, StdFunction) {
    using FuncType = std::function<int(int, double)>;
    using Traits = FunctionTraits<FuncType>;

    static_assert(std::is_same_v<Traits::return_type, int>);
    static_assert(std::is_same_v<Traits::argument_t<0>, int>);
    static_assert(std::is_same_v<Traits::argument_t<1>, double>);
    static_assert(Traits::arity == 2);
    static_assert(!Traits::is_member_function);

    EXPECT_EQ(Traits::full_name, "int (int, double)");
}

TEST(FunctionTraitsTest, MemberFunction) {
    using Traits = FunctionTraits<decltype(&TestClass::memberFunction)>;

    static_assert(std::is_same_v<Traits::return_type, int>);
    static_assert(std::is_same_v<Traits::argument_t<0>, int>);
    static_assert(std::is_same_v<Traits::argument_t<1>, double>);
    static_assert(Traits::arity == 2);
    static_assert(Traits::is_member_function);
    static_assert(!Traits::is_const_member_function);

    EXPECT_EQ(Traits::full_name, "int (int, double)");
    EXPECT_EQ(DemangleHelper::demangleType<Traits::class_type>(), "TestClass");
}

TEST(FunctionTraitsTest, ConstMemberFunction) {
    using Traits = FunctionTraits<decltype(&TestClass::constMemberFunction)>;

    static_assert(Traits::is_const_member_function);
}

TEST(FunctionTraitsTest, VolatileMemberFunction) {
    using Traits = FunctionTraits<decltype(&TestClass::volatileMemberFunction)>;

    static_assert(Traits::is_volatile_member_function);
}

TEST(FunctionTraitsTest, ConstVolatileMemberFunction) {
    using Traits =
        FunctionTraits<decltype(&TestClass::constVolatileMemberFunction)>;

    static_assert(Traits::is_const_member_function);
    static_assert(Traits::is_volatile_member_function);
}

TEST(FunctionTraitsTest, LvalueReferenceMemberFunction) {
    using Traits =
        FunctionTraits<decltype(&TestClass::lvalueReferenceMemberFunction)>;

    static_assert(Traits::is_lvalue_reference_member_function);
}

TEST(FunctionTraitsTest, RvalueReferenceMemberFunction) {
    using Traits =
        FunctionTraits<decltype(&TestClass::rvalueReferenceMemberFunction)>;

    static_assert(Traits::is_rvalue_reference_member_function);
}

TEST(FunctionTraitsTest, NoexceptMemberFunction) {
    using Traits = FunctionTraits<decltype(&TestClass::noexceptMemberFunction)>;

    static_assert(Traits::is_noexcept);
}

TEST(FunctionTraitsTest, LambdaFunction) {
    auto lambda = [](int a, double b) -> int {
        return a + static_cast<int>(b);
    };
    using Traits = FunctionTraits<decltype(lambda)>;

    static_assert(std::is_same_v<Traits::return_type, int>);
    static_assert(std::is_same_v<Traits::argument_t<0>, int>);
    static_assert(std::is_same_v<Traits::argument_t<1>, double>);
    static_assert(Traits::arity == 2);
    // static_assert(!Traits::is_member_function);
}

TEST(FunctionTraitsTest, FunctionReference) {
    using Traits = FunctionTraits<int (&)(int, double)>;

    static_assert(std::is_same_v<Traits::return_type, int>);
    static_assert(std::is_same_v<Traits::argument_t<0>, int>);
    static_assert(std::is_same_v<Traits::argument_t<1>, double>);
    static_assert(Traits::arity == 2);
    static_assert(!Traits::is_member_function);
}

// Example classes
struct WithFoo {
    void foo() {}
    static void static_foo() {}
};

struct WithBar {
    int bar(int, double) { return 42; }
    static int static_foo(int) { return 42; }
};

struct WithConstFoo {
    void foo() const {}
};

struct WithoutConstFoo {
    void foo() {}
};

struct NoMethod {};

DEFINE_HAS_METHOD(foo);
DEFINE_HAS_METHOD(bar);
DEFINE_HAS_STATIC_METHOD(static_foo);
DEFINE_HAS_CONST_METHOD(foo);

TEST(MemberFunctionDetection, NonStaticMemberFunctions) {
    // Check for foo()
    EXPECT_TRUE((has_foo<WithFoo, void>::value));
    EXPECT_FALSE((has_foo<WithBar, void>::value));

    // Check for bar(int, double)
    EXPECT_TRUE((has_bar<WithBar, int, int, double>::value));
    EXPECT_FALSE((has_bar<WithFoo, int, int, double>::value));

    // Check for nonexistent methods
    EXPECT_FALSE((has_foo<NoMethod, void>::value));
}

TEST(MemberFunctionDetection, StaticMemberFunctions) {
    // Check for static_foo()
    EXPECT_TRUE((has_static_static_foo<WithFoo, void>::value));
    EXPECT_TRUE((has_static_static_foo<WithBar, int, int>::value));

    // Check for nonexistent static methods
    EXPECT_FALSE((has_static_static_foo<NoMethod, void>::value));
}

TEST(MemberFunctionDetection, ConstMemberFunctions) {
    // Check for const foo()
    EXPECT_TRUE((has_const_foo<WithConstFoo, void>::value));
    EXPECT_FALSE((has_const_foo<WithoutConstFoo, void>::value));
    EXPECT_FALSE((has_const_foo<NoMethod, void>::value));
}
