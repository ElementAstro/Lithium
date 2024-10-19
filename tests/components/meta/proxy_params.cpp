/*!
 * \file test_proxy_params.hpp
 * \brief Unit tests for Proxy Function Params
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TEST_PROXY_PARAMS_HPP
#define ATOM_META_TEST_PROXY_PARAMS_HPP

#include <gtest/gtest.h>
#include <any>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/proxy_params.hpp"

using namespace atom::meta;

// 测试 Arg 类
TEST(ArgTest, ConstructorAndGetters) {
    Arg arg1("param1");
    EXPECT_EQ(arg1.getName(), "param1");
    EXPECT_FALSE(arg1.getDefaultValue().has_value());

    Arg arg2("param2", 42);
    EXPECT_EQ(arg2.getName(), "param2");
    EXPECT_TRUE(arg2.getDefaultValue().has_value());
    EXPECT_EQ(std::any_cast<int>(arg2.getDefaultValue().value()), 42);
}

// 测试 FunctionParams 类的构造函数
TEST(FunctionParamsTest, SingleElementConstructor) {
    Arg arg("param1", 42);
    FunctionParams params(arg);

    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(params[0].getDefaultValue().value()), 42);
}

TEST(FunctionParamsTest, RangeConstructor) {
    std::vector<Arg> vec = {Arg("param1", 1), Arg("param2", "test"),
                            Arg("param3", 3.14)};
    FunctionParams params(vec);

    ASSERT_EQ(params.size(), vec.size());
    EXPECT_EQ(params[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(params[0].getDefaultValue().value()), 1);
    EXPECT_EQ(params[1].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(params[1].getDefaultValue().value()),
              "test");
    EXPECT_EQ(params[2].getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(params[2].getDefaultValue().value()), 3.14);
}

TEST(FunctionParamsTest, InitializerListConstructor) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};

    ASSERT_EQ(params.size(), 3);
    EXPECT_EQ(params[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(params[0].getDefaultValue().value()), 1);
    EXPECT_EQ(params[1].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(params[1].getDefaultValue().value()),
              "test");
    EXPECT_EQ(params[2].getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(params[2].getDefaultValue().value()), 3.14);
}

// 测试 FunctionParams 类的其他方法
TEST(FunctionParamsTest, AccessOperator) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};

    EXPECT_EQ(params[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(params[0].getDefaultValue().value()), 1);
    EXPECT_EQ(params[1].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(params[1].getDefaultValue().value()),
              "test");
    EXPECT_EQ(params[2].getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(params[2].getDefaultValue().value()), 3.14);
    EXPECT_THROW(params[3], atom::error::OutOfRange);  // 越界访问
}

TEST(FunctionParamsTest, BeginEnd) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};
    std::vector<Arg> expected = {Arg("param1", 1), Arg("param2", "test"),
                                 Arg("param3", 3.14)};

    EXPECT_TRUE(std::equal(params.begin(), params.end(), expected.begin(),
                           [](const Arg& a, const Arg& b) {
                               return a.getName() == b.getName() &&
                                      a.getDefaultValue().has_value() ==
                                          b.getDefaultValue().has_value() &&
                                      (!a.getDefaultValue().has_value() ||
                                       a.getDefaultValue().value().type() ==
                                           b.getDefaultValue().value().type());
                           }));
}

TEST(FunctionParamsTest, FrontMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};

    EXPECT_EQ(params.front().getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(params.front().getDefaultValue().value()), 1);
}

TEST(FunctionParamsTest, SizeAndEmpty) {
    FunctionParams empty_params;
    EXPECT_EQ(empty_params.size(), 0);
    EXPECT_TRUE(empty_params.empty());

    FunctionParams params = {Arg("param1", 1), Arg("param2", "test")};
    EXPECT_EQ(params.size(), 2);
    EXPECT_FALSE(params.empty());
}

TEST(FunctionParamsTest, ToVectorMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};
    auto vec = params.toVector();

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(vec[0].getDefaultValue().value()), 1);
    EXPECT_EQ(vec[1].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(vec[1].getDefaultValue().value()),
              "test");
    EXPECT_EQ(vec[2].getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(vec[2].getDefaultValue().value()), 3.14);
}

TEST(FunctionParamsTest, ToAnyVectorMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};
    auto anyVec = params.toAnyVector();

    EXPECT_EQ(anyVec.size(), 3);
    EXPECT_EQ(std::any_cast<int>(anyVec[0]), 1);
    EXPECT_EQ(std::any_cast<std::string>(anyVec[1]), "test");
    EXPECT_EQ(std::any_cast<double>(anyVec[2]), 3.14);
}

TEST(FunctionParamsTest, GetByNameMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};

    auto param1 = params.getByName("param1");
    EXPECT_TRUE(param1.has_value());
    EXPECT_EQ(param1->getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(param1->getDefaultValue().value()), 1);

    auto param2 = params.getByName("param2");
    EXPECT_TRUE(param2.has_value());
    EXPECT_EQ(param2->getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(param2->getDefaultValue().value()),
              "test");

    auto param3 = params.getByName("param3");
    EXPECT_TRUE(param3.has_value());
    EXPECT_EQ(param3->getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(param3->getDefaultValue().value()), 3.14);

    auto invalidParam = params.getByName("invalid");
    EXPECT_FALSE(invalidParam.has_value());
}

TEST(FunctionParamsTest, SliceMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14), Arg("param4", 42)};

    auto slice = params.slice(1, 3);
    EXPECT_EQ(slice.size(), 2);
    EXPECT_EQ(slice[0].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(slice[0].getDefaultValue().value()),
              "test");
    EXPECT_EQ(slice[1].getName(), "param3");
    EXPECT_EQ(std::any_cast<double>(slice[1].getDefaultValue().value()), 3.14);

    EXPECT_THROW(params.slice(3, 2), atom::error::OutOfRange);  // Invalid range
    EXPECT_THROW(params.slice(1, 5), atom::error::OutOfRange);  // Out of bounds
}

TEST(FunctionParamsTest, FilterMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14), Arg("param4", 42)};

    auto filtered = params.filter([](const Arg& arg) -> bool {
        return arg.getDefaultValue().has_value() &&
               arg.getDefaultValue()->type() == typeid(int);
    });

    EXPECT_EQ(filtered.size(), 2);
    EXPECT_EQ(filtered[0].getName(), "param1");
    EXPECT_EQ(std::any_cast<int>(filtered[0].getDefaultValue().value()), 1);
    EXPECT_EQ(filtered[1].getName(), "param4");
    EXPECT_EQ(std::any_cast<int>(filtered[1].getDefaultValue().value()), 42);
}

TEST(FunctionParamsTest, SetMethod) {
    FunctionParams params = {Arg("param1", 1), Arg("param2", "test"),
                             Arg("param3", 3.14)};

    params.set(1, Arg("param2", "new_test"));
    EXPECT_EQ(params[1].getName(), "param2");
    EXPECT_EQ(std::any_cast<std::string>(params[1].getDefaultValue().value()),
              "new_test");

    EXPECT_THROW(params.set(3, Arg("param4", 42)),
                 atom::error::OutOfRange);  // 越界设置
}

#endif  // ATOM_META_TEST_PROXY_PARAMS_HPP