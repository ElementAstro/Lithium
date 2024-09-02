#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

#include "atom/function/invoke.hpp"

// 测试辅助函数
int add_t(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
void throwError() { throw std::runtime_error("Error!"); }

class TestClass {
public:
    int multiply(int a, int b) { return a * b; }
    int addConst(int a) const { return a + 10; }
    static int divide(int a, int b) { return a / b; }
    int memberVar = 42;
};

TEST(InvokeTest, DelayInvoke) {
    auto delayedAdd = delayInvoke(add_t, 5, 3);
    EXPECT_EQ(delayedAdd(), 8);

    auto delayedSubtract = delayInvoke(subtract, 10, 2);
    EXPECT_EQ(delayedSubtract(), 8);
}

TEST(InvokeTest, DelayMemInvoke) {
    TestClass obj;
    auto delayedMultiply = delayMemInvoke(&TestClass::multiply, &obj);
    EXPECT_EQ(delayedMultiply(4, 5), 20);

    const TestClass constObj;
    auto delayedAddConst = delayMemInvoke(&TestClass::addConst, &constObj);
    EXPECT_EQ(delayedAddConst(5), 15);
}

TEST(InvokeTest, DelayStaticMemInvoke) {
    TestClass obj;
    auto delayedDivide = delayStaticMemInvoke(&TestClass::divide, &obj);
    EXPECT_EQ(delayedDivide(20, 5), 4);
}

TEST(InvokeTest, DelayMemberVarInvoke) {
    TestClass obj;
    auto delayedMemberVar = delayMemberVarInvoke(&TestClass::memberVar, &obj);
    EXPECT_EQ(delayedMemberVar(), 42);
}

TEST(InvokeTest, SafeCall) {
    EXPECT_EQ(safeCall(add_t, 3, 7), 10);

    // Test safe call with exception thrown
    EXPECT_THROW(safeCall(throwError), atom::error::RuntimeError);

    // Test safe call with default constructible return type
    auto result = safeCall([]() -> int {
        throw std::runtime_error("Error!");
        return 0;
    });
    EXPECT_EQ(result, 0);
}

TEST(InvokeTest, SafeTryCatch) {
    auto successResult = safeTryCatch(add_t, 10, 5);
    EXPECT_TRUE(std::holds_alternative<int>(successResult));
    EXPECT_EQ(std::get<int>(successResult), 15);
}

TEST(InvokeTest, SafeTryCatchOrDefault) {
    // Test successful call
    EXPECT_EQ(safeTryCatchOrDefault(add_t, 0, 3, 2), 5);
}

TEST(InvokeTest, SafeTryCatchWithCustomHandler) {
    bool exceptionHandled = false;

    auto handler = [&exceptionHandled](std::exception_ptr eptr) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::runtime_error& e) {
            exceptionHandled = true;
        }
    };

    EXPECT_EQ(safeTryCatchWithCustomHandler(add_t, handler, 3, 4), 7);
    EXPECT_FALSE(exceptionHandled);

    /* TODO: FIX ME
        // Test with exception and custom handler
        EXPECT_NO_THROW(safeTryCatchWithCustomHandler(throwError, handler));
        EXPECT_TRUE(exceptionHandled);
    */
}
