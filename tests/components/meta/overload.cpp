#include "atom/function/overload.hpp"
#include <gtest/gtest.h>

class MyClass {
public:
    // Non-const member function
    void foo(int x) {
        last_called = "foo(int)";
        last_int = x;
    }

    // Const member function
    void foo(double x) const {
        last_called = "foo(double)";
        last_double = x;
    }

    // Volatile member function
    void foo(float x) volatile {
        last_called = "foo(float)";
        last_float = x;
    }

    // Const volatile member function
    void foo(const char* x) const volatile {
        last_called = "foo(const char*)";
        last_string = x;
    }

    // Regular free-standing function
    static void bar(int x) {
        last_called = "bar(int)";
        last_int = x;
    }

    // Static noexcept function
    static void bar(double x) noexcept {
        last_called = "bar(double)";
        last_double = x;
    }

    static inline std::string last_called;
    static inline int last_int;
    static inline double last_double;
    static inline float last_float;
    static inline const char* last_string;
};

// Test non-const member function
TEST(OverloadCastTest, NonConstMemberFunction) {
    MyClass obj;
    auto non_const_foo = atom::meta::overload_cast<int>(&MyClass::foo);
    (obj.*non_const_foo)(42);

    EXPECT_EQ(MyClass::last_called, "foo(int)");
    EXPECT_EQ(MyClass::last_int, 42);
}

// Test const member function
TEST(OverloadCastTest, ConstMemberFunction) {
    const MyClass obj;
    auto const_foo = atom::meta::overload_cast<double>(&MyClass::foo);
    (obj.*const_foo)(3.14);

    EXPECT_EQ(MyClass::last_called, "foo(double)");
    EXPECT_DOUBLE_EQ(MyClass::last_double, 3.14);
}

// Test volatile member function
TEST(OverloadCastTest, VolatileMemberFunction) {
    volatile MyClass obj;
    auto volatile_foo = atom::meta::overload_cast<float>(&MyClass::foo);
    (obj.*volatile_foo)(5.67f);

    EXPECT_EQ(MyClass::last_called, "foo(float)");
    EXPECT_FLOAT_EQ(MyClass::last_float, 5.67f);
}

// Test const volatile member function
TEST(OverloadCastTest, ConstVolatileMemberFunction) {
    const volatile MyClass obj;
    auto const_volatile_foo =
        atom::meta::overload_cast<const char*>(&MyClass::foo);
    (obj.*const_volatile_foo)("Test string");

    EXPECT_EQ(MyClass::last_called, "foo(const char*)");
    EXPECT_STREQ(MyClass::last_string, "Test string");
}

// Test regular static function
TEST(OverloadCastTest, StaticFunction) {
    auto static_bar = atom::meta::overload_cast<int>(&MyClass::bar);
    static_bar(100);

    EXPECT_EQ(MyClass::last_called, "bar(int)");
    EXPECT_EQ(MyClass::last_int, 100);
}

// Test noexcept static function
TEST(OverloadCastTest, StaticNoexceptFunction) {
    auto static_noexcept_bar = atom::meta::overload_cast<double>(&MyClass::bar);
    static_noexcept_bar(2.718);

    EXPECT_EQ(MyClass::last_called, "bar(double)");
    EXPECT_DOUBLE_EQ(MyClass::last_double, 2.718);
}

// Test that overload fails gracefully with incorrect signature
TEST(OverloadCastTest, CorrectOverloadResolution) {
    auto non_const_foo = atom::meta::overload_cast<int>(&MyClass::foo);
    using ExpectedType = void (MyClass::*)(int);
    EXPECT_TRUE((std::is_same_v<decltype(non_const_foo), ExpectedType>));

    auto const_foo = atom::meta::overload_cast<double>(&MyClass::foo);
    using ExpectedConstType = void (MyClass::*)(double) const;
    EXPECT_TRUE((std::is_same_v<decltype(const_foo), ExpectedConstType>));

    auto volatile_foo = atom::meta::overload_cast<float>(&MyClass::foo);
    using ExpectedVolatileType = void (MyClass::*)(float) volatile;
    EXPECT_TRUE((std::is_same_v<decltype(volatile_foo), ExpectedVolatileType>));

    auto const_volatile_foo =
        atom::meta::overload_cast<const char*>(&MyClass::foo);
    using ExpectedConstVolatileType =
        void (MyClass::*)(const char*) const volatile;
    EXPECT_TRUE((std::is_same_v<decltype(const_volatile_foo),
                                ExpectedConstVolatileType>));
}

// Test overloaded function on different types and ensure correctness
TEST(OverloadCastTest, OverloadedFreeFunction) {
    MyClass::last_called.clear();
    MyClass::last_int = 0;

    auto static_bar_int = atom::meta::overload_cast<int>(&MyClass::bar);
    auto static_bar_double = atom::meta::overload_cast<double>(&MyClass::bar);

    static_bar_int(200);
    EXPECT_EQ(MyClass::last_called, "bar(int)");
    EXPECT_EQ(MyClass::last_int, 200);

    static_bar_double(3.14);
    EXPECT_EQ(MyClass::last_called, "bar(double)");
    EXPECT_DOUBLE_EQ(MyClass::last_double, 3.14);
}
