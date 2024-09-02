#include "task/generator.hpp"
#include <gtest/gtest.h>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"

using namespace lithium;
using json = nlohmann::json;

// Utility function to create a shared TaskGenerator instance
std::shared_ptr<TaskGenerator> createTaskGenerator() {
    return TaskGenerator::createShared();
}

TEST(TaskGeneratorTest, BasicMacroUsage) {
    auto generator = createTaskGenerator();

    json j = {{"name", "${uppercase(hello)}"},
              {"concat", "${concat(Hello, ,World)}"},
              {"length", "${length(Hello)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["name"], "HELLO");
    EXPECT_EQ(j["concat"], "Hello World");
    EXPECT_EQ(j["length"], "5");
}

TEST(TaskGeneratorTest, NestedMacroUsage) {
    auto generator = createTaskGenerator();

    json j = {{"nested", "${concat(${uppercase(hello)}, ,${tolower(WORLD)})}"}};

    generator->processJson(j);

    EXPECT_EQ(j["nested"], "HELLO world");
}

TEST(TaskGeneratorTest, ConditionalMacroUsage) {
    auto generator = createTaskGenerator();

    json j = {{"conditionTrue", "${if(true,Yes,No)}"},
              {"conditionFalse", "${if(false,Yes,No)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["conditionTrue"], "Yes");
    EXPECT_EQ(j["conditionFalse"], "No");
}

TEST(TaskGeneratorTest, RepeatMacroUsage) {
    auto generator = createTaskGenerator();

    json j = {{"repeat", "${repeat(abc,3)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["repeat"], "abcabcabc");
}

TEST(TaskGeneratorTest, UndefinedMacroShouldThrow) {
    auto generator = createTaskGenerator();

    json j = {{"undefined", "${undefinedMacro()}"}};

    EXPECT_THROW(generator->processJson(j), atom::error::InvalidArgument);
}

TEST(TaskGeneratorTest, IncorrectArgumentCountShouldThrow) {
    auto generator = createTaskGenerator();

    json j = {
        {"concat", "${concat(Hello)}"},  // Not enough arguments
        {"if", "${if(true,Yes)}"}        // Missing third argument
    };

    EXPECT_THROW(generator->processJson(j), atom::error::InvalidArgument);
}

TEST(TaskGeneratorTest, ComplexMacroUsage) {
    auto generator = createTaskGenerator();

    json j = {{"complex", "${if(${equals(${length(hello)},5)},true,false)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["complex"], "true");
}

// TODO: FIX ME - Not working
/*
TEST(TaskGeneratorTest, ProcessJsonWithJsonMacros) {
    auto generator = TaskGenerator::createShared();

    json j = {{"macros",
               {{"uppercase_macro", "${uppercase(hello)}"},
                {"simple_macro", "simple_value"}}},
              {"use_macros",
               "${concat(${macros.uppercase_macro},${macros.simple_macro})}"}};

    generator->processJsonWithJsonMacros(j);

    EXPECT_EQ(j["use_macros"], "HELLO simple_value");
}
*/

TEST(TaskGeneratorTest, EmptyStringHandling) {
    auto generator = createTaskGenerator();

    json j = {{"empty", ""}, {"non_empty", "${concat(,Hello)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["empty"], "");
    EXPECT_EQ(j["non_empty"], "Hello");
}

TEST(TaskGeneratorTest, NoMacrosInString) {
    auto generator = createTaskGenerator();

    json j = {{"text", "No macros here!"}};

    generator->processJson(j);

    EXPECT_EQ(j["text"], "No macros here!");
}

TEST(TaskGeneratorTest, EmptyJsonObjectHandling) {
    auto generator = createTaskGenerator();

    json j = {};

    EXPECT_NO_THROW(generator->processJson(j));
}

TEST(TaskGeneratorTest, MalformedMacroShouldThrow) {
    auto generator = createTaskGenerator();

    json j = {{"malformed1", "${uppercase(Hello}"},
              {"malformed2", "${uppercaseHello)}"}};

    EXPECT_THROW(generator->processJson(j), std::exception);
}

// TODO: FIX ME - Not working
/*
TEST(TaskGeneratorTest, MalformedMacroInsideJsonShouldThrow) {
    auto generator = TaskGenerator::createShared();

    json j = {{"complex", "${concat(${uppercase(Hello)},${repeat(x,2)})"},
              {"text", "This is ${not_a_macro()"}};  // Incorrect macro

    EXPECT_THROW(generator->processJsonWithJsonMacros(j), std::exception);
}
*/

TEST(TaskGeneratorTest, DeeplyNestedMacros) {
    auto generator = createTaskGenerator();

    json j = {
        {"nested",
         "${concat(${concat(${uppercase(hello)}, ,${tolower(WORLD)})},!,)}"}};

    generator->processJson(j);

    EXPECT_EQ(j["nested"], "HELLO world!");
}
