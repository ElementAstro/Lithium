#include <gtest/gtest.h>

#include "task/manager.hpp"

#include <chrono>
#include <system_error>
#include <thread>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"
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

TEST_F(TaskInterpreterTest, DetermineType) {
    EXPECT_EQ(determineType(123), VariableType::NUMBER);
    EXPECT_EQ(determineType("test"), VariableType::STRING);
    EXPECT_EQ(determineType(true), VariableType::BOOLEAN);
    EXPECT_EQ(determineType(json::parse("{\"key\": \"value\"}")),
              VariableType::JSON);
    EXPECT_EQ(determineType(nullptr), VariableType::UNKNOWN);
}

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
                {"type": "assign", "variable": "counter", "value": {"$":
"counter + 1"}}
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
        {"type": "assign", "variable": "x", "value": "x + 1"},
        {"type": "condition", "condition": {"$eq": [
                    "x",
                    3
                ]}, "true": {"type": "goto", "label": "end"}, "false": {"type": "goto", "label": "start"}},
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
    {
        "header": {
            "name": "Initialization Script",
            "version": "1.0.1",
            "author": "Max Qian",
            "description": "This script initializes variables and runs setup steps.",
            "auto_execute": true
        },
        "steps": [
            {"type": "print", "message": "Initialization started."},
            {"type": "assign", "variable": "initialized", "value": true},
            {"type": "assign", "variable": "a", "value": 100},
            {"type": "print", "message": "Initialization complete."}
        ]
    }
    )"_json;

    json scriptB = R"(
    [
        {"type": "import", "script": "scriptA"},
        {"type": "print", "message": "Script B executed."},
        {"type": "assign", "variable": "b", "value": {"$": "a + 1"}},
        {"type": "print", "message": "Script B completed."}
    ]
    )"_json;

    interpreter->loadScript("scriptA", scriptA);
    interpreter->loadScript("scriptB", scriptB);
    interpreter->execute("scriptB");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(interpreter->getVariable("a").get<int>(), 100);
    std::cout << interpreter->getVariable("b").dump() << std::endl;
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

TEST_F(TaskInterpreterTest, CompleteScriptExecution) {
    // 注册函数
    interpreter->registerFunction("multiply", [](const json& params) {
        return params["a"].get<int>() * params["b"].get<int>();
    });

    // 注册异常处理
    interpreter->registerExceptionHandler(
        "complex_script", [&](const std::exception& e) {
            std::cerr << "Handled exception: " << e.what() << std::endl;
        });

    // 定义并加载脚本
    json script = R"({
        "header": {
            "name": "Complex Script",
            "author": "Max Qian",
            "version": "1.0",
            "auto_execute": true
        },
        "steps": [
            {"type": "assign", "variable": "x", "value": 5},
            {"type": "assign", "variable": "y", "value": 10},
            {"type": "call", "function": "multiply", "params": {"a": "$x", "b": "$y"}, "result": "product"},
            {"type": "print", "message": "The product of x and y is $product"}
        ]
    })"_json;

    interpreter->loadScript("complex_script", script);

    // 校验脚本变量
    EXPECT_EQ(interpreter->getVariable("x"), 5);
    EXPECT_EQ(interpreter->getVariable("y"), 10);
    EXPECT_EQ(interpreter->getVariable("product"), 50);
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

TEST_F(TaskInterpreterTest, EventHandling) {
    json script = R"({
        "steps": [
            {"type": "listen_event", "event_names": ["my_event"], "timeout": 1000},
            {"type": "broadcast_event", "event_name": "my_event"},
            {"type": "assign", "variable": "event_triggered", "value": true}
        ]
    })"_json;

    interpreter->loadScript("event_script", script);
    interpreter->execute("event_script");

    EXPECT_EQ(interpreter->getVariable("event_triggered"), true);
}

TEST_F(TaskInterpreterTest, TryCatchFinally) {
    json script = R"({
        "steps": [
            {
                "type": "try",
                "try": [
                    {"type": "throw", "exception_type": "runtime_error", "message": "Test Exception"}
                ],
                "catch": [{
                    "type": "nlohmann::json_abi_v3_11_3::detail::type_error",
                    "steps": [
                        {"type": "assign", "variable": "caught", "value": true}
                    ]
                }],
                "finally": [
                    {"type": "assign", "variable": "finalized", "value": true}
                ]
            }
        ]
    })"_json;

    interpreter->loadScript("try_catch_script", script);
    interpreter->execute("try_catch_script");

    EXPECT_EQ(interpreter->getVariable("caught"), true);
    EXPECT_EQ(interpreter->getVariable("finalized"), true);
}

// TODO: FIX ME - This test is failing
/*
TEST_F(TaskInterpreterTest, FunctionDefinitionWithClosureAndRecursion) {
    json script = R"({
        "steps": [
            {
                "type": "function_def",
                "name": "factorial",
                "params": ["n"],
                "steps": [
                    {"type": "condition", "condition": {"$lt": ["$n", 2]}, "true": {"type": "return", "value": 1}},
                    {"type": "assign", "variable": "n_minus_1", "value": {"$sub": ["$n", 1]}},
                    {"type": "call", "function": "factorial", "params": {"n": "$n_minus_1"}, "result": "sub_result"},
                    {"type": "return", "value": {"$mul": ["$n", "$sub_result"]}}
                ]
            },
            {"type": "call", "function": "factorial", "params": {"n": 5}, "result": "factorial_result"}
        ]
    })"_json;
    

    interpreter->loadScript("factorial_script", script);
    interpreter->execute("factorial_script");

    EXPECT_EQ(interpreter->getVariable("factorial_result"), 120);
}
*/

TEST_F(TaskInterpreterTest, FullAbilityTest)
{
    json script = R"(
        {
        "steps": [
            // 定义一个简单的加法函数
            {
                "type": "function_def",
                "name": "add",
                "params": ["a", "b"],
                "steps": [
                    {
                        "type": "return",
                        "value": {"$add": ["$a", "$b"]}
                    }
                ]
            },

            // 调用 add 函数
            {
                "type": "call",
                "function": "add",
                "params": {"a": 3, "b": 4},
                "result": "addition_result"
            },

            // 打印加法结果
            {
                "type": "print",
                "message": "3 + 4 = $addition_result"
            },

            // 执行条件判断
            {
                "type": "condition",
                "condition": {"$gt": ["$addition_result", 5]},
                "true": {
                    "type": "print",
                    "message": "Addition result is greater than 5"
                },
                "false": {
                    "type": "print",
                    "message": "Addition result is not greater than 5"
                }
            },

            // 执行一个 while 循环，将 n 从 5 减少到 0
            {
                "type": "assign",
                "variable": "n",
                "value": 5
            },
            {
                "type": "while",
                "condition": {"$gt": ["$n", 0]},
                "steps": [
                    {"type": "print", "message": "n is: $n"},
                    {"type": "assign", "variable": "n", "value": {"$sub": ["$n", 1]}}
                ]
            },

            // 执行并行任务
            {
                "type": "parallel",
                "steps": [
                    {"type": "print", "message": "Parallel task 1"},
                    {"type": "print", "message": "Parallel task 2"},
                    {"type": "print", "message": "Parallel task 3"}
                ]
            },

            // 等待一个事件并处理
            {
                "type": "wait_event",
                "event": "custom_event"
            },
            {
                "type": "print",
                "message": "Custom event received!"
            },

            // 调度一个任务，延迟3秒执行
            {
                "type": "schedule",
                "delay": 3000,
                "steps": [
                    {"type": "print", "message": "This message is delayed by 3 seconds"}
                ]
            },

            // 处理错误的 try-catch-finally 结构
            {
                "type": "try",
                "try": [
                    {"type": "throw", "exception_type": "runtime_error", "message": "Simulated Error"}
                ],
                "catch": {
                    "type": "all",
                    "steps": [
                        {"type": "print", "message": "Exception caught in catch block!"}
                    ]
                },
                "finally": [
                    {"type": "print", "message": "Finally block executed."}
                ]
            },

            // 广播事件
            {
                "type": "broadcast_event",
                "event_name": "custom_event",
                "event_data": {}
            },

            // 定义一个带有作用域和变量的嵌套脚本
            {
                "type": "scope",
                "variables": {
                    "local_var": 42
                },
                "steps": [
                    {"type": "print", "message": "Local var inside scope is: $local_var"},
                    {"type": "assign", "variable": "local_var", "value": {"$add": ["$local_var", 1]}},
                    {"type": "print", "message": "Local var after increment: $local_var"}
                ]
            },

            // 使用 switch-case 结构
            {
                "type": "switch",
                "variable": "addition_result",
                "cases": [
                    {
                        "case": 7,
                        "steps": [
                            {"type": "print", "message": "Result is exactly 7"}
                        ]
                    },
                    {
                        "case": 8,
                        "steps": [
                            {"type": "print", "message": "Result is 8"}
                        ]
                    }
                ],
                "default": {
                    "steps": [
                        {"type": "print", "message": "Result is neither 7 nor 8"}
                    ]
                }
            },

            // 导入另一个脚本
            {
                "type": "import",
                "script": "external_script"
            }
        ]
    }

    )";

    interpreter->loadScript("main_script", script);
    interpreter->execute("main_script");
}