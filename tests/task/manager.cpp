#include <gtest/gtest.h>
#include "task/manager.hpp"

using namespace lithium;

/*
// Test Lexer
TEST(LexerTest, TokenizeNumbers) {
    Lexer lexer("123 45.67");
    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::NUMBER);
    EXPECT_EQ(token1.value, "123");

    auto token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::NUMBER);
    EXPECT_EQ(token2.value, "45.67");
}

TEST(LexerTest, TokenizeStrings) {
    Lexer lexer("\"hello\" \"world\"");
    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::STRING);
    EXPECT_EQ(token1.value, "hello");

    auto token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::STRING);
    EXPECT_EQ(token2.value, "world");
}

TEST(LexerTest, TokenizeIdentifiers) {
    Lexer lexer("foo bar");
    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(token1.value, "foo");

    auto token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::IDENTIFIER);
    EXPECT_EQ(token2.value, "bar");
}

TEST(LexerTest, TokenizeOperators) {
    Lexer lexer("+ - * / = == != < > <= >=");
    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::PLUS);
    EXPECT_EQ(token1.value, "+");

    auto token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::MINUS);
    EXPECT_EQ(token2.value, "-");

    auto token3 = lexer.nextToken();
    EXPECT_EQ(token3.type, TokenType::MULTIPLY);
    EXPECT_EQ(token3.value, "*");

    auto token4 = lexer.nextToken();
    EXPECT_EQ(token4.type, TokenType::DIVIDE);
    EXPECT_EQ(token4.value, "/");

    auto token5 = lexer.nextToken();
    EXPECT_EQ(token5.type, TokenType::ASSIGNMENT);
    EXPECT_EQ(token5.value, "=");

    auto token6 = lexer.nextToken();
    EXPECT_EQ(token6.type, TokenType::EQUAL);
    EXPECT_EQ(token6.value, "==");

    auto token7 = lexer.nextToken();
    EXPECT_EQ(token7.type, TokenType::NOT_EQUAL);
    EXPECT_EQ(token7.value, "!=");

    auto token8 = lexer.nextToken();
    EXPECT_EQ(token8.type, TokenType::LESS);
    EXPECT_EQ(token8.value, "<");

    auto token9 = lexer.nextToken();
    EXPECT_EQ(token9.type, TokenType::GREATER);
    EXPECT_EQ(token9.value, ">");

    auto token10 = lexer.nextToken();
    EXPECT_EQ(token10.type, TokenType::LESS_EQUAL);
    EXPECT_EQ(token10.value, "<=");

    auto token11 = lexer.nextToken();
    EXPECT_EQ(token11.type, TokenType::GREATER_EQUAL);
    EXPECT_EQ(token11.value, ">=");
}

// Test Parser
TEST(ParserTest, ParseNumber) {
    Lexer lexer("123");
    Parser parser(lexer);
    auto program = parser.parse();
    auto stmt = program->getStatements()[0];
    auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt);
    auto number = std::dynamic_pointer_cast<Number>(exprStmt->getExpression());
    ASSERT_NE(number, nullptr);
    EXPECT_EQ(number->getValue(), "123");
}

TEST(ParserTest, ParseString) {
    Lexer lexer("\"hello\"");
    Parser parser(lexer);
    auto program = parser.parse();
    auto stmt = program->getStatements()[0];
    auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt);
    auto strLiteral = std::dynamic_pointer_cast<StringLiteral>(exprStmt->getExpression());
    ASSERT_NE(strLiteral, nullptr);
    EXPECT_EQ(strLiteral->getValue(), "hello");
}

TEST(ParserTest, ParseIdentifier) {
    Lexer lexer("foo");
    Parser parser(lexer);
    auto program = parser.parse();
    auto stmt = program->getStatements()[0];
    auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt);
    auto identifier = std::dynamic_pointer_cast<Identifier>(exprStmt->getExpression());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->getName(), "foo");
}

TEST(ParserTest, ParseBinaryOp) {
    Lexer lexer("1 + 2");
    Parser parser(lexer);
    auto program = parser.parse();
    auto stmt = program->getStatements()[0];
    auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt);
    auto binaryOp = std::dynamic_pointer_cast<BinaryOp>(exprStmt->getExpression());
    ASSERT_NE(binaryOp, nullptr);
    auto left = std::dynamic_pointer_cast<Number>(binaryOp->getLeft());
    auto right = std::dynamic_pointer_cast<Number>(binaryOp->getRight());
    ASSERT_NE(left, nullptr);
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(left->getValue(), "1");
    EXPECT_EQ(right->getValue(), "2");
    EXPECT_EQ(binaryOp->getOp().type, TokenType::PLUS);
}

// Test Interpreter
TEST(InterpreterTest, EvaluateNumber) {
    Lexer lexer("123");
    Parser parser(lexer);
    auto program = parser.parse();
    Interpreter interpreter;
    auto result = interpreter.interpret(program);
    EXPECT_EQ(std::get<int>(result), 123);
}

TEST(InterpreterTest, EvaluateString) {
    Lexer lexer("\"hello\"");
    Parser parser(lexer);
    auto program = parser.parse();
    Interpreter interpreter;
    auto result = interpreter.interpret(program);
    EXPECT_EQ(std::get<std::string>(result), "hello");
}

TEST(InterpreterTest, EvaluateBinaryOp) {
    Lexer lexer("1 + 2");
    Parser parser(lexer);
    auto program = parser.parse();
    Interpreter interpreter;
    auto result = interpreter.interpret(program);
    EXPECT_EQ(std::get<int>(result), 3);
}

TEST(InterpreterTest, EvaluateFunctionCall) {
    Lexer lexer("function foo() { return 42; } foo();");
    Parser parser(lexer);
    auto program = parser.parse();
    Interpreter interpreter;
    interpreter.interpret(program);
    auto result = interpreter.callFunction("foo", {});
    EXPECT_EQ(std::get<int>(result), 42);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
