#include "atom/function/overload.hpp"
#include <gtest/gtest.h>

class TestClass {
public:
    int func(int x) { return x + 1; }
    int func(int x) const { return x + 2; }
    int func(int x, double y) { return static_cast<int>(x + y); }
    int func(int x, double y) const noexcept {
        return static_cast<int>(x + y + 1);
    }
    static int staticFunc(int x) { return x * 2; }
    int funcNoexcept(int x) noexcept { return x * 3; }
    int funcConstVolatile(int x) const volatile { return x * 4; }
};

int globalFunc(int x) { return x + 5; }
int globalFuncNoexcept(int x) noexcept { return x + 6; }

TEST(OverloadCastTest, MemberFunctionOverload) {
    TestClass obj;

    // Test non-const member function
    auto nonConstFunc = atom::meta::overload_cast<int>(
        static_cast<int (TestClass::*)(int)>(&TestClass::func));
    EXPECT_EQ((obj.*nonConstFunc)(10), 11);

    // Test const member function
    auto constFunc = atom::meta::overload_cast<int>(
        static_cast<int (TestClass::*)(int) const>(&TestClass::func));
    EXPECT_EQ((static_cast<const TestClass*>(&obj)->*constFunc)(10), 12);

    // Test member function with two arguments
    auto twoArgsFunc = atom::meta::overload_cast<int, double>(
        static_cast<int (TestClass::*)(int, double)>(&TestClass::func));
    EXPECT_EQ((obj.*twoArgsFunc)(10, 2.5), 12);

    // Test const noexcept member function with two arguments
    auto constNoexceptFunc = atom::meta::overload_cast<int, double>(
        static_cast<int (TestClass::*)(int, double) const noexcept>(
            &TestClass::func));
    EXPECT_EQ(
        (static_cast<const TestClass*>(&obj)->*constNoexceptFunc)(10, 2.5), 13);
}

TEST(OverloadCastTest, StaticMemberFunction) {
    // Test static member function
    auto staticFunc = atom::meta::overload_cast<int>(
        static_cast<int (*)(int)>(&TestClass::staticFunc));
    EXPECT_EQ(staticFunc(5), 10);
}

TEST(OverloadCastTest, NoexceptMemberFunction) {
    TestClass obj;

    // Test noexcept member function
    auto noexceptFunc = atom::meta::overload_cast<int>(
        static_cast<int (TestClass::*)(int) noexcept>(
            &TestClass::funcNoexcept));
    EXPECT_EQ((obj.*noexceptFunc)(7), 21);
}

/* TODO: FIX ME
TEST(OverloadCastTest, ConstVolatileMemberFunction) {
    volatile TestClass obj;

    // Test const volatile member function
    auto constVolatileFunc = atom::meta::overload_cast<int>(
        static_cast<int (TestClass::*)(int) const volatile>(
            &TestClass::funcConstVolatile));
    EXPECT_EQ((obj.*constVolatileFunc)(3), 12);
}
*/

TEST(OverloadCastTest, GlobalFunction) {
    // Test global function
    auto globalFuncPtr =
        atom::meta::overload_cast<int>(static_cast<int (*)(int)>(&globalFunc));
    EXPECT_EQ(globalFuncPtr(15), 20);

    // Test noexcept global function
    auto globalFuncNoexceptPtr = atom::meta::overload_cast<int>(
        static_cast<int (*)(int) noexcept>(&globalFuncNoexcept));
    EXPECT_EQ(globalFuncNoexceptPtr(15), 21);
}

TEST(OverloadCastTest, DifferentArgumentTypes) {
    TestClass obj;

    // Test member function with int and double arguments
    auto funcIntDouble = atom::meta::overload_cast<int, double>(
        static_cast<int (TestClass::*)(int, double)>(&TestClass::func));
    EXPECT_EQ((obj.*funcIntDouble)(10, 1.5), 11);
}

TEST(OverloadCastTest, InvalidFunctionPointer) {
    // Ensure the code compiles with invalid pointers and does not throw an
    // exception
    EXPECT_NO_THROW({
        auto invalidFunc = atom::meta::overload_cast<int>(
            static_cast<int (*)(int)>(&globalFuncNoexcept));
    });
}