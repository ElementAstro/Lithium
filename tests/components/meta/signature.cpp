#include "atom/function/signature.hpp"
#include <gtest/gtest.h>

TEST(FunctionSignatureTest,
     ParseFunctionDefinition_ValidDefinition_ReturnsSignature) {
    std::string_view definition = "def foo(a: int, b: float) -> float";
    std::optional<atom::meta::FunctionSignature> result =
        atom::meta::parseFunctionDefinition(definition);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "foo");
    EXPECT_EQ(result->parameters.size(), 2);
    EXPECT_EQ(result->parameters[0].first, "a");
    EXPECT_EQ(result->parameters[0].second, "int");
    EXPECT_EQ(result->parameters[1].first, "b");
    EXPECT_EQ(result->parameters[1].second, "float");
    EXPECT_EQ(result->returnType.value(), "float");
}

TEST(FunctionSignatureTest,
     ParseFunctionDefinition_MissingName_ReturnsNullopt) {
    std::string_view definition = "def (a: int, b: float) -> float";
    std::optional<atom::meta::FunctionSignature> result =
        atom::meta::parseFunctionDefinition(definition);

    EXPECT_FALSE(result.has_value());
}

TEST(FunctionSignatureTest,
     ParseFunctionDefinition_MissingParameters_ReturnsNullopt) {
    std::string_view definition = "def foo() -> float";
    std::optional<atom::meta::FunctionSignature> result =
        atom::meta::parseFunctionDefinition(definition);

    EXPECT_FALSE(result.has_value());
}

TEST(
    FunctionSignatureTest,
    ParseFunctionDefinition_MissingReturnType_ReturnsSignatureWithoutReturnType) {
    std::string_view definition = "def foo(a: int, b: float)";
    std::optional<atom::meta::FunctionSignature> result =
        atom::meta::parseFunctionDefinition(definition);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "foo");
    EXPECT_EQ(result->parameters.size(), 2);
    EXPECT_EQ(result->parameters[0].first, "a");
    EXPECT_EQ(result->parameters[0].second, "int");
    EXPECT_EQ(result->parameters[1].first, "b");
    EXPECT_EQ(result->parameters[1].second, "float");
    EXPECT_FALSE(result->returnType.has_value());
}

TEST(FunctionSignatureTest,
     ParseFunctionDefinition_InvalidDefinition_ReturnsNullopt) {
    std::string_view definition = "def foo(a, b) -> float";
    std::optional<atom::meta::FunctionSignature> result =
        atom::meta::parseFunctionDefinition(definition);

    EXPECT_FALSE(result.has_value());
}