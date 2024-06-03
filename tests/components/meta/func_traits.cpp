#include "atom/function/func_traits.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper functions for testing
int freeFunction(int a, double b) { return a + static_cast<int>(b); }

struct TestClass {
    int memberFunction(int a, double b) { return a + static_cast<int>(b); }
    int constMemberFunction(int a, double b) const {
        return a + static_cast<int>(b);
    }
    int volatileMemberFunction(int a, double b) volatile {
        return a + static_cast<int>(b);
    }
    int constVolatileMemberFunction(int a, double b) const volatile {
        return a + static_cast<int>(b);
    }
    int lvalueReferenceMemberFunction(int a, double b) & {
        return a + static_cast<int>(b);
    }
    int rvalueReferenceMemberFunction(int a, double b) && {
        return a + static_cast<int>(b);
    }
    int noexceptMemberFunction(int a, double b) noexcept {
        return a + static_cast<int>(b);
    }
    int variadicMemberFunction(int a, ...) { return a; }
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
    EXPECT_EQ(DemangleHelper::DemangleType<Traits::class_type>(), "TestClass");
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