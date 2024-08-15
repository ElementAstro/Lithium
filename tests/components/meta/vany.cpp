#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "atom/function/vany.hpp"
#include "atom/macro.hpp"

// 测试默认构造函数
TEST(AnyTest, DefaultConstructor) {
    atom::meta::Any any;
    EXPECT_FALSE(any.hasValue());
    EXPECT_THROW(any.type(), std::bad_typeid);
    EXPECT_EQ(any.toString(), "Empty Any");
}

// 测试存储整数
TEST(AnyTest, StoreInteger) {
    atom::meta::Any any(42);
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<int>());
    EXPECT_EQ(any.cast<int>(), 42);
    EXPECT_EQ(any.toString(), "42");
}

// 测试存储字符串
TEST(AnyTest, StoreString) {
    std::string str = "Hello, World!";
    atom::meta::Any any(str);
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<std::string>());
    EXPECT_EQ(any.cast<std::string>(), str);
    EXPECT_EQ(any.toString(), str);
}

// 测试存储浮点数
TEST(AnyTest, StoreFloat) {
    atom::meta::Any any(3.14f);
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<float>());
    EXPECT_FLOAT_EQ(any.cast<float>(), 3.14f);
    EXPECT_EQ(any.toString(), "3.140000");
}

// 测试拷贝构造
/*
TEST(AnyTest, CopyConstructor) {
    atom::meta::Any original(42);
    atom::meta::Any copy = original;
    EXPECT_TRUE(copy.hasValue());
    EXPECT_TRUE(copy.is<int>());
    EXPECT_EQ(copy.cast<int>(), 42);
    EXPECT_EQ(copy.toString(), "42");
}
*/


// 测试移动构造
TEST(AnyTest, MoveConstructor) {
    atom::meta::Any original(42);
    atom::meta::Any moved = std::move(original);
    EXPECT_FALSE(original.hasValue());
    EXPECT_TRUE(moved.hasValue());
    EXPECT_TRUE(moved.is<int>());
    EXPECT_EQ(moved.cast<int>(), 42);
}

// 测试拷贝赋值操作符
/*
TEST(AnyTest, CopyAssignment) {
    atom::meta::Any any;
    atom::meta::Any other(42);
    any = other;
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<int>());
    EXPECT_EQ(any.cast<int>(), 42);
}
*/


// 测试移动赋值操作符
TEST(AnyTest, MoveAssignment) {
    atom::meta::Any any;
    atom::meta::Any other(42);
    any = std::move(other);
    EXPECT_FALSE(other.hasValue());
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<int>());
    EXPECT_EQ(any.cast<int>(), 42);
}

// 测试 reset 函数
TEST(AnyTest, ResetFunction) {
    atom::meta::Any any(42);
    EXPECT_TRUE(any.hasValue());
    any.reset();
    EXPECT_FALSE(any.hasValue());
    EXPECT_EQ(any.toString(), "Empty Any");
}

// 测试类型不匹配时的 cast
TEST(AnyTest, BadCast) {
    atom::meta::Any any(42);
    EXPECT_THROW(any.cast<std::string>(), std::bad_cast);
}

// 测试小对象优化
TEST(AnyTest, SmallObjectOptimization) {
    struct SmallObject {
        int x;
        float y;
    };

    atom::meta::Any any(SmallObject{1, 2.0f});
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<SmallObject>());
    const auto& obj = any.cast<SmallObject>();
    EXPECT_EQ(obj.x, 1);
    EXPECT_FLOAT_EQ(obj.y, 2.0f);
}

// 测试大对象
TEST(AnyTest, LargeObjectStorage) {
    struct LargeObject {
        int data[1000];
    };

    atom::meta::Any any(LargeObject{});
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<LargeObject>());
}

// 测试 foreach 和 iterable
TEST(AnyTest, ForeachFunction) {
    std::vector<int> vec = {1, 2, 3};
    atom::meta::Any any(vec);
    std::vector<int> result;

    any.foreach ([&result](const atom::meta::Any& element) {
        result.push_back(element.cast<int>());
    });

    EXPECT_EQ(result, vec);
}

// 测试非 iterable 类型上的 foreach
TEST(AnyTest, ForeachOnNonIterable) {
    atom::meta::Any any(42);
    EXPECT_THROW(any.foreach ([](const atom::meta::Any&) {}),
                 atom::error::InvalidArgument);
}

// 测试异常处理
TEST(AnyTest, ExceptionHandling) {
    try {
        atom::meta::Any any(42);
        any.cast<std::string>();
        FAIL() << "Expected std::bad_cast";
    } catch (const std::bad_cast& err) {
        EXPECT_EQ(err.what(), std::string("std::bad_cast"));
    } catch (...) {
        FAIL() << "Expected std::bad_cast";
    }
}

// 测试 invoke 函数
TEST(AnyTest, InvokeFunction) {
    atom::meta::Any any(42);
    int result = 0;
    any.invoke(
        [&result](const void* ptr) { result = *static_cast<const int*>(ptr); });
    EXPECT_EQ(result, 42);
}
