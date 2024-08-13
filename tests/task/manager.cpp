#include <gtest/gtest.h>

#include "task/manager.hpp"

#include <chrono>
#include <system_error>
#include <thread>

#include "atom/type/json.hpp"
#include "atom/error/exception.hpp"
using namespace lithium;
using json = nlohmann::json;

// Helper function to create a simple script
json createSimpleScript() {
    return R"(
    [
        {
            "type": "assign",
            "variable": "x",
            "value": 10
        },
        {
            "type": "condition",
            "condition": {
                "$eq": [
                    "x",
                    10
                ]
            },
            "true": {
                "type": "assign",
                "variable": "y",
                "value": 20
            },
            "false": {
                "type": "assign",
                "variable": "y",
                "value": 30
            }
        },
        {
            "type": "call",
            "function": "increment_x",
            "params": {
                "x": "x"
            },
            "result": "x"
        }
    ]
    )"_json;
}

class TaskInterpreterTest : public ::testing::Test {
protected:
    void SetUp() override { interpreter = TaskInterpreter::createShared(); }

    std::shared_ptr<TaskInterpreter> interpreter;
};

// Test loading and unloading scripts
TEST_F(TaskInterpreterTest, LoadAndUnloadScript) {
    json script = createSimpleScript();
    interpreter->loadScript("test_script", script);
    ASSERT_TRUE(interpreter->hasScript("test_script"));

    auto loadedScript = interpreter->getScript("test_script");
    ASSERT_TRUE(loadedScript.has_value());
    EXPECT_EQ(script, loadedScript.value());

    interpreter->unloadScript("test_script");
    EXPECT_FALSE(interpreter->hasScript("test_script"));
}

// Test executing a simple script
TEST_F(TaskInterpreterTest, ExecuteSimpleScript) {
    try {
        json script = createSimpleScript();
        interpreter->loadScript("test_script", script);

        interpreter->registerFunction("increment_x",
                                      [](const json& params) -> json {
                                          int x = params.at("x").get<int>();
                                          return x + 1;
                                      });

        interpreter->execute("test_script");

        EXPECT_EQ(interpreter->getVariable("x"), 11);
        EXPECT_EQ(interpreter->getVariable("y"), 20);
    } catch (const std::system_error& e) {
        std::cout << e.what() << " " << e.code() << std::endl;
    }
}

// Test exception handling for missing scripts
TEST_F(TaskInterpreterTest, MissingScriptThrowsException) {
    EXPECT_THROW(interpreter->execute("nonexistent_script"),
                 atom::error::RuntimeError);
}

// Test handling of script preparation failure
// TODO: Add a strong type for script preparation failure
/*
TEST_F(TaskInterpreterTest, PrepareScriptFailure) {
    json malformedScript = R"(
    [
        {"type": "assign", "variable": "x", "value": "not a number"}
    ]
    )"_json;

    EXPECT_THROW(interpreter->loadScript("bad_script", malformedScript),
                 std::runtime_error);
}
*/


// Test handling of a function that throws an exception
TEST_F(TaskInterpreterTest, FunctionExceptionHandling) {
    json script = R"(
    [
        {"type": "call", "function": "throw_error", "params": {}}
    ]
    )"_json;

    interpreter->loadScript("exception_script", script);

    interpreter->registerFunction("throw_error", [](const json&) -> json {
        throw std::runtime_error("Test error");
    });

    interpreter->registerExceptionHandler(
        "exception_script",
        [](const std::exception& e) { EXPECT_STREQ(e.what(), "Test error"); });

    interpreter->execute("exception_script");
}

// Test multithreading: Pausing and resuming execution
/*
TEST_F(TaskInterpreterTest, PauseAndResume) {
    json script = R"(
    [
        {"type": "assign", "variable": "counter", "value": 0},
        {"type": "loop", "loop_iterations": 5, "steps": [
            {"type": "async", "steps": [
                {"type": "assign", "variable": "counter", "value": {"$": "counter + 1"}}
            ]}
        ]}
    ]
    )"_json;

    interpreter->loadScript("pause_resume_script", script);

    interpreter->execute("pause_resume_script");

    interpreter->pause();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    interpreter->resume();

    EXPECT_EQ(interpreter->getVariable("counter").get<int>(), 5);
}
*/


// Test handling of script labels and goto
TEST_F(TaskInterpreterTest, LabelAndGoto) {
    json script = R"(
    [
        {"type": "assign", "variable": "x", "value": 0},
        {"type": "message", "label": "start"},
        {"type": "assign", "variable": "x", "value": {"$": "x + 1"}},
        {"type": "condition", "condition": {"x": 3}, "true": {"type": "goto", "label": "end"}, "false": {"type": "goto", "label": "start"}},
        {"type": "message", "label": "end"}
    ]
    )"_json;

    interpreter->loadScript("label_goto_script", script);
    interpreter->execute("label_goto_script");

    EXPECT_EQ(interpreter->getVariable("x").get<int>(), 3);
}

// Test handling of script importing
TEST_F(TaskInterpreterTest, ScriptImport) {
    json scriptA = R"(
    [
        {"type": "assign", "variable": "a", "value": 100}
    ]
    )"_json;

    json scriptB = R"(
    [
        {"type": "import", "script": "scriptA"},
        {"type": "assign", "variable": "b", "value": {"$": "a + 1"}}
    ]
    )"_json;

    interpreter->loadScript("scriptA", scriptA);
    interpreter->loadScript("scriptB", scriptB);
    interpreter->execute("scriptB");

    EXPECT_EQ(interpreter->getVariable("a").get<int>(), 100);
    EXPECT_EQ(interpreter->getVariable("b").get<int>(), 101);
}

// Test edge case: Extremely large script
TEST_F(TaskInterpreterTest, LargeScriptExecution) {
    json script = R"([])"_json;
    for (int i = 0; i < 10000; ++i) {
        script.push_back({{"type", "assign"}, {"variable", "x"}, {"value", i}});
    }
    interpreter->loadScript("large_script", script);
    interpreter->execute("large_script");

    EXPECT_EQ(interpreter->getVariable("x").get<int>(), 9999);
}

// Test edge case: Nested parallel execution
TEST_F(TaskInterpreterTest, NestedParallelExecution) {
    json script = R"(
    [
        {"type": "parallel", "steps": [
            {"type": "assign", "variable": "a", "value": 1},
            {"type": "parallel", "steps": [
                {"type": "assign", "variable": "b", "value": 2},
                {"type": "assign", "variable": "c", "value": 3}
            ]}
        ]}
    ]
    )"_json;

    interpreter->loadScript("nested_parallel_script", script);
    interpreter->execute("nested_parallel_script");

    EXPECT_EQ(interpreter->getVariable("a").get<int>(), 1);
    EXPECT_EQ(interpreter->getVariable("b").get<int>(), 2);
    EXPECT_EQ(interpreter->getVariable("c").get<int>(), 3);
}
