#include "argsview.hpp"
#include <gtest/gtest.h>

TEST(ArgsViewTest, ConstructorAndSize) {
    ArgsView<int, double, std::string> view(1, 2.0, "test");
    EXPECT_EQ(view.size(), 3);
}

TEST(ArgsViewTest, Get) {
    ArgsView<int, double, std::string> view(1, 2.0, "test");
    EXPECT_EQ(view.get<0>(), 1);
    EXPECT_EQ(view.get<1>(), 2.0);
    EXPECT_EQ(view.get<2>(), "test");
}

TEST(ArgsViewTest, Empty) {
    ArgsView<> view;
    EXPECT_TRUE(view.empty());
}

TEST(ArgsViewTest, ForEach) {
    ArgsView<int, double, std::string> view(1, 2.0, "test");
    std::vector<std::string> results;
    view.forEach([&results](const auto& arg) {
        if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                     std::string>) {
            results.push_back(arg);
        } else {
            results.push_back(std::to_string(arg));
        }
    });
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], "1");
    EXPECT_EQ(results[1], "2.000000");
    EXPECT_EQ(results[2], "test");
}

TEST(ArgsViewTest, Transform) {
    ArgsView<int, double> view(1, 2.0);
    auto transformed =
        view.transform([](const auto& arg) { return std::to_string(arg); });
    EXPECT_EQ(transformed.get<0>(), "1");
    EXPECT_EQ(transformed.get<1>(), "2.000000");
}

TEST(ArgsViewTest, Accumulate) {
    ArgsView<int, int, int> view(1, 2, 3);
    auto sum = view.accumulate([](int lhs, int rhs) { return lhs + rhs; }, 0);
    EXPECT_EQ(sum, 6);
}

TEST(ArgsViewTest, Apply) {
    ArgsView<int, double> view(1, 2.0);
    auto result = view.apply([](const auto&... args) { return (args + ...); });
    EXPECT_EQ(result, 3.0);
}

TEST(ArgsViewTest, Filter) {
    ArgsView<int, double, int> view(1, 2.0, 3);
    auto filtered = view.filter([](const auto& arg) { return arg > 1; });
    EXPECT_EQ(filtered.size(), 3);
    EXPECT_EQ(filtered.template get<0>(), std::nullopt);
    EXPECT_EQ(filtered.template get<1>(), 2.0);
    EXPECT_EQ(filtered.template get<2>(), 3);
}

TEST(ArgsViewTest, Find) {
    ArgsView<int, double, int> view(1, 2.0, 3);
    auto found = view.find([](const auto& arg) { return arg > 1; });
    EXPECT_EQ(found, 2.0);
}

TEST(ArgsViewTest, Contains) {
    ArgsView<int, double, int> view(1, 2.0, 3);
    EXPECT_TRUE(view.contains(2.0));
    EXPECT_FALSE(view.contains(4));
}

TEST(ArgsViewTest, SumFunction) {
    auto result = sum(1, 2, 3);
    EXPECT_EQ(result, 6);
}

TEST(ArgsViewTest, ConcatFunction) {
    constexpr double testValue = 3.0;
    auto result = concat(1, "test", testValue);
    EXPECT_EQ(result, "1test3.000000");
}

TEST(ArgsViewTest, EqualityOperator) {
    ArgsView<int, double> view1(1, 2.0);
    ArgsView<int, double> view2(1, 2.0);
    EXPECT_TRUE(view1 == view2);
}

TEST(ArgsViewTest, InequalityOperator) {
    ArgsView<int, double> view1(1, 2.0);
    ArgsView<int, double> view2(1, 3.0);
    EXPECT_TRUE(view1 != view2);
}

TEST(ArgsViewTest, LessThanOperator) {
    ArgsView<int, double> view1(1, 2.0);
    ArgsView<int, double> view2(1, 3.0);
    EXPECT_TRUE(view1 < view2);
}

TEST(ArgsViewTest, LessThanOrEqualToOperator) {
    ArgsView<int, double> view1(1, 2.0);
    ArgsView<int, double> view2(1, 2.0);
    EXPECT_TRUE(view1 <= view2);
}

TEST(ArgsViewTest, GreaterThanOperator) {
    ArgsView<int, double> view1(1, 3.0);
    ArgsView<int, double> view2(1, 2.0);
    EXPECT_TRUE(view1 > view2);
}

TEST(ArgsViewTest, GreaterThanOrEqualToOperator) {
    ArgsView<int, double> view1(1, 2.0);
    ArgsView<int, double> view2(1, 2.0);
    EXPECT_TRUE(view1 >= view2);
}

TEST(ArgsViewTest, Hash) {
    ArgsView<int, double> view(1, 2.0);
    std::hash<ArgsView<int, double>> hasher;
    EXPECT_NE(hasher(view), 0);
}
