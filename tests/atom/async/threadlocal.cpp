#include "atom/async/threadlocal.hpp"
#include <gtest/gtest.h>


class ThreadLocalTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 可选：在每个测试用例之前设置通用的初始化代码
    }

    void TearDown() override {
        // 可选：在每个测试用例之后设置通用的清理代码
    }
};

// 测试默认构造函数和 get 函数
TEST_F(ThreadLocalTest, DefaultConstructor) {
    ThreadLocal<int> threadLocalInt;
    EXPECT_THROW(threadLocalInt.get(), std::bad_optional_access);
}

// 测试带初始化函数的构造函数和 get 函数
TEST_F(ThreadLocalTest, ConstructorWithInitializer) {
    ThreadLocal<int> threadLocalInt([] { return 42; });
    EXPECT_EQ(threadLocalInt.get(), 42);
}

// 测试 reset 函数
TEST_F(ThreadLocalTest, Reset) {
    ThreadLocal<int> threadLocalInt([] { return 42; });
    threadLocalInt.reset(100);
    EXPECT_EQ(threadLocalInt.get(), 100);
}

// 测试 hasValue 函数
TEST_F(ThreadLocalTest, HasValue) {
    ThreadLocal<int> threadLocalInt;
    EXPECT_FALSE(threadLocalInt.hasValue());
    threadLocalInt.reset(10);
    EXPECT_TRUE(threadLocalInt.hasValue());
}

// 测试 getPointer 函数
TEST_F(ThreadLocalTest, GetPointer) {
    ThreadLocal<int> threadLocalInt([] { return 42; });
    int* ptr = threadLocalInt.getPointer();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);

    const ThreadLocal<int>& constThreadLocalInt = threadLocalInt;
    const int* constPtr = constThreadLocalInt.getPointer();
    ASSERT_NE(constPtr, nullptr);
    EXPECT_EQ(*constPtr, 42);
}

// 测试 forEach 函数
TEST_F(ThreadLocalTest, ForEach) {
    ThreadLocal<int> threadLocalInt([] { return 42; });
    threadLocalInt.reset(100);

    int sum = 0;
    threadLocalInt.forEach([&sum](int value) { sum += value; });
    EXPECT_EQ(sum, 100);
}

// 测试 clear 函数
TEST_F(ThreadLocalTest, Clear) {
    ThreadLocal<int> threadLocalInt([] { return 42; });
    threadLocalInt.reset(100);
    threadLocalInt.clear();
    EXPECT_FALSE(threadLocalInt.hasValue());
}

// 测试多线程环境下的功能
TEST_F(ThreadLocalTest, MultiThreaded) {
    ThreadLocal<int> threadLocalInt([] { return 42; });

    auto threadFunc = [&threadLocalInt](int id) {
        threadLocalInt.reset(id);
        EXPECT_EQ(threadLocalInt.get(), id);
    };

    std::thread thread1(threadFunc, 1);
    std::thread thread2(threadFunc, 2);

    thread1.join();
    thread2.join();

    // 主线程应保持原始状态
    EXPECT_EQ(threadLocalInt.get(), 42);
}
