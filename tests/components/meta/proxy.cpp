#include "atom/function/proxy.hpp"
#include <gtest/gtest.h>
#include <any>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

// 示例函数和类，用于测试
int add(int a, int b) { return a + b; }

void voidFunction(int& a, int b) { a += b; }

class TestClass {
public:
    int multiply(int a, int b) const { return a * b; }

    void setValue(int value) { value_ = value; }

    int getValue() const { return value_; }

private:
    int value_ = 0;
};

// 测试非成员函数代理
TEST(ProxyFunctionTest, NonMemberFunction) {
    atom::meta::ProxyFunction proxy(add);

    std::vector<std::any> args{2, 3};
    auto result = proxy(args);

    EXPECT_EQ(std::any_cast<int>(result), 5);
}

// 测试void非成员函数代理
/*
TEST(ProxyFunctionTest, VoidNonMemberFunction) {
    int a = 1;
    atom::meta::ProxyFunction proxy(voidFunction);

    std::vector<std::any> args{std::ref(a), 4};
    auto result = proxy(args);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(a, 5);
}
*/

// 测试成员函数代理
TEST(ProxyFunctionTest, MemberFunction) {
    TestClass obj;
    atom::meta::ProxyFunction proxy(&TestClass::multiply);

    std::vector<std::any> args{std::ref(obj), 4, 5};
    auto result = proxy(args);

    EXPECT_EQ(std::any_cast<int>(result), 20);
}

// 测试void成员函数代理
TEST(ProxyFunctionTest, VoidMemberFunction) {
    TestClass obj;
    atom::meta::ProxyFunction proxy(&TestClass::setValue);

    std::vector<std::any> args{std::ref(obj), 42};
    auto result = proxy(args);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(obj.getValue(), 42);
}

// 测试参数数量不足的异常情况
TEST(ProxyFunctionTest, IncorrectNumberOfArguments) {
    atom::meta::ProxyFunction proxy(add);

    std::vector<std::any> args{2};  // 缺少一个参数
    EXPECT_THROW(proxy(args), atom::error::Exception);
}

// 测试成员函数参数数量不足的异常情况
TEST(ProxyFunctionTest, IncorrectNumberOfArgumentsMemberFunction) {
    TestClass obj;
    atom::meta::ProxyFunction proxy(&TestClass::multiply);

    std::vector<std::any> args{std::ref(obj), 4};  // 缺少一个参数
    EXPECT_THROW(proxy(args), atom::error::Exception);
}

// 测试返回类型不匹配的情况
TEST(ProxyFunctionTest, InvalidReturnType) {
    atom::meta::ProxyFunction proxy(add);

    std::vector<std::any> args{2, 3};
    auto result = proxy(args);

    // 尝试提取错误的返回类型
    EXPECT_THROW(std::any_cast<std::string>(result), std::bad_any_cast);
}

// 测试定时器代理函数
TEST(TimerProxyFunctionTest, NonMemberFunctionWithTimeout) {
    atom::meta::TimerProxyFunction proxy(add);

    std::vector<std::any> args{2, 3};
    auto result = proxy(args, std::chrono::milliseconds(100));

    EXPECT_EQ(std::any_cast<int>(result), 5);
}

// 测试void定时器代理函数
/* TODO: FIX ME
TEST(TimerProxyFunctionTest, VoidNonMemberFunctionWithTimeout) {
    int a = 1;
    atom::meta::TimerProxyFunction proxy(voidFunction);

    std::vector<std::any> args{std::ref(a), 4};
    auto result = proxy(args, std::chrono::milliseconds(100));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(a, 5);
}
*/

// 测试超时情况
/* TODO: FIX ME
TEST(TimerProxyFunctionTest, FunctionTimeout) {
    atom::meta::TimerProxyFunction proxy([](int a, int b) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return a + b;
    });

    std::vector<std::any> args{2, 3};
    EXPECT_THROW(proxy(args, std::chrono::milliseconds(500)),
                 atom::error::Exception);  // 500ms超时
}
*/

// 测试成员函数定时器代理
TEST(TimerProxyFunctionTest, MemberFunctionWithTimeout) {
    TestClass obj;
    atom::meta::TimerProxyFunction proxy(&TestClass::multiply);

    std::vector<std::any> args{std::ref(obj), 4, 5};
    auto result = proxy(args, std::chrono::milliseconds(100));

    EXPECT_EQ(std::any_cast<int>(result), 20);
}

// 测试void成员函数定时器代理
TEST(TimerProxyFunctionTest, VoidMemberFunctionWithTimeout) {
    TestClass obj;
    atom::meta::TimerProxyFunction proxy(&TestClass::setValue);

    std::vector<std::any> args{std::ref(obj), 42};
    auto result = proxy(args, std::chrono::milliseconds(100));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(obj.getValue(), 42);
}
