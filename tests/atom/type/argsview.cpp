#include "atom/type/argsview.hpp"

#include <gtest/gtest.h>

// 测试 ArgsView 的构造函数
TEST(ArgsViewTest, Constructor) {
    ArgsView<int, double, std::string> args(1, 2.5, "test");
    EXPECT_EQ(args.size(), 3);
    EXPECT_EQ(args.get<0>(), 1);
    EXPECT_EQ(args.get<1>(), 2.5);
    EXPECT_EQ(args.get<2>(), "test");
}

// 测试 from tuple 的构造函数
TEST(ArgsViewTest, ConstructorFromTuple) {
    std::tuple<int, double, std::string> tpl(1, 2.5, "test");
    ArgsView<int, double, std::string> args(tpl);
    EXPECT_EQ(args.size(), 3);
    EXPECT_EQ(args.get<0>(), 1);
    EXPECT_EQ(args.get<1>(), 2.5);
    EXPECT_EQ(args.get<2>(), "test");
}

// 测试 forEach 方法
TEST(ArgsViewTest, ForEach) {
    ArgsView<int, double, std::string> args(1, 2.5, "test");
    std::vector<std::string> results;
    args.forEach([&results](const auto& arg) {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                     std::string>) {
            results.push_back(arg);
        } else {
            results.push_back(std::to_string(arg));
        }
    });
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], "1");
    EXPECT_EQ(results[1], "2.500000");
    EXPECT_EQ(results[2], "test");
}

// 测试 transform 方法
TEST(ArgsViewTest, Transform) {
    ArgsView<int, double> args(1, 2.5);
    auto transformed =
        args.transform([](const auto& arg) { return std::to_string(arg); });
    EXPECT_EQ(transformed.size(), 2);
    EXPECT_EQ(transformed.get<0>(), "1");
    EXPECT_EQ(transformed.get<1>(), "2.500000");
}

// 测试 accumulate 方法
TEST(ArgsViewTest, Accumulate) {
    ArgsView<int, int, int> args(1, 2, 3);
    int sum = args.accumulate([](int a, int b) { return a + b; }, 0);
    EXPECT_EQ(sum, 6);
}

// 测试 apply 方法
TEST(ArgsViewTest, Apply) {
    ArgsView<int, double> args(1, 2.5);
    auto result = args.apply(
        [](const auto&... args) { return std::make_tuple(args...); });
    EXPECT_EQ(std::get<0>(result), 1);
    EXPECT_EQ(std::get<1>(result), 2.5);
}

// 测试运算符==
TEST(ArgsViewTest, EqualityOperator) {
    ArgsView<int, double> args1(1, 2.5);
    ArgsView<int, double> args2(1, 2.5);
    ArgsView<int, double> args3(1, 3.5);
    EXPECT_TRUE(args1 == args2);
    EXPECT_FALSE(args1 == args3);
}

// 测试运算符!=
TEST(ArgsViewTest, InequalityOperator) {
    ArgsView<int, double> args1(1, 2.5);
    ArgsView<int, double> args2(1, 2.5);
    ArgsView<int, double> args3(1, 3.5);
    EXPECT_FALSE(args1 != args2);
    EXPECT_TRUE(args1 != args3);
}

// 测试运算符 <
TEST(ArgsViewTest, LessThanOperator) {
    ArgsView<int, double> args1(1, 2.5);
    ArgsView<int, double> args2(1, 3.5);
    EXPECT_TRUE(args1 < args2);
    EXPECT_FALSE(args2 < args1);
}

// 测试运算符<=
TEST(ArgsViewTest, LessThanOrEqualOperator) {
    ArgsView<int, double> args1(1, 2.5);
    ArgsView<int, double> args2(1, 3.5);
    ArgsView<int, double> args3(1, 2.5);
    EXPECT_TRUE(args1 <= args2);
    EXPECT_TRUE(args1 <= args3);
    EXPECT_FALSE(args2 <= args1);
}

// 测试运算符>
TEST(ArgsViewTest, GreaterThanOperator) {
    ArgsView<int, double> args1(1, 3.5);
    ArgsView<int, double> args2(1, 2.5);
    EXPECT_TRUE(args1 > args2);
    EXPECT_FALSE(args2 > args1);
}

// 测试运算符>=
TEST(ArgsViewTest, GreaterThanOrEqualOperator) {
    ArgsView<int, double> args1(1, 3.5);
    ArgsView<int, double> args2(1, 2.5);
    ArgsView<int, double> args3(1, 3.5);
    EXPECT_TRUE(args1 >= args2);
    EXPECT_TRUE(args1 >= args3);
    EXPECT_FALSE(args2 >= args1);
}

// 测试 hash 特化
TEST(ArgsViewTest, Hash) {
    ArgsView<int, double> args(1, 2.5);
    std::hash<ArgsView<int, double>> hasher;
    EXPECT_NE(hasher(args), 0);
}

#ifdef __DEBUG__
TEST(ArgsViewTest, Print) {
    std::ostringstream oss;
    auto coutbuf = std::cout.rdbuf(oss.rdbuf());
    print(1, 2.5, "test");
    std::cout.rdbuf(coutbuf);
    EXPECT_EQ(oss.str(), "1 2.5 test \n");
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
