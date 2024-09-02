/**
 * @file task_interpreter.hpp
 * @brief Task Interpreter for managing and executing scripts.
 *
 * This file defines the `TaskInterpreter` class, which is responsible for
 * loading, managing, and executing tasks represented as JSON scripts. The
 * `TaskInterpreter` class provides functionality to register functions and
 * exception handlers, set and retrieve variables, and control script execution
 * flow (e.g., pause, resume, stop). It supports various script operations such
 * as parsing labels, executing steps, handling exceptions, and evaluating
 * expressions.
 *
 * The class also supports asynchronous operations and event handling, making it
 * suitable for dynamic and complex scripting environments.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_INTERPRETER_HPP
#define LITHIUM_TASK_INTERPRETER_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {

enum class VariableType { NUMBER, STRING, BOOLEAN, JSON, UNKNOWN };

auto determineType(const json& value) -> VariableType;

class TaskInterpreterImpl;

class TaskInterpreter {
public:
    TaskInterpreter();
    ~TaskInterpreter();

    static auto createShared() -> std::shared_ptr<TaskInterpreter>;

    void loadScript(const std::string& name, const json& script);
    void unloadScript(const std::string& name);

    [[nodiscard]] auto hasScript(const std::string& name) const -> bool;
    [[nodiscard]] auto getScript(const std::string& name) const
        -> std::optional<json>;

    void registerFunction(const std::string& name,
                          std::function<json(const json&)> func);
    void registerExceptionHandler(
        const std::string& name,
        std::function<void(const std::exception&)> handler);
    void registerCustomError(const std::string& name,
                             const std::error_code& errorCode);

    void setVariable(const std::string& name, const json& value,
                     VariableType type);
    [[nodiscard]] auto getVariable(const std::string& name) const -> json;
    [[nodiscard]] auto getVariableImmediate(const std::string& name) const
        -> json;

    void parseLabels(const json& script);
    void execute(const std::string& scriptName);
    void stop();
    void pause();
    void resume();
    void queueEvent(const std::string& eventName, const json& eventData);

private:
    auto prepareScript(json& script) -> bool;
    void executeScript(const std::string& scriptName);
    void checkPause();

    auto executeStep(const json& step, size_t& idx, const json& script) -> bool;
    void executeCall(const json& step);
    void executeFunctionDef(const json& step);
    [[nodiscard]] auto captureClosureVariables() const -> json;
    void restoreClosureVariables(const json& closure);

    void executeCondition(const json& step, size_t& idx, const json& script);
    auto executeLoop(const json& step, size_t& idx, const json& script) -> bool;
    void executeWhileLoop(const json& step, size_t& idx, const json& script);
    void executeGoto(const json& step, size_t& idx, const json& script);
    void executeSwitch(const json& step, size_t& idx, const json& script);

    void executeScope(const json& step, size_t& idx, const json& script);
    void executeNestedScript(const json& step);
    void executeAssign(const json& step);
    void executeImport(const json& step);
    void executeWaitEvent(const json& step);
    void executePrint(const json& step);

    void executeAsync(const json& step);
    void executeSchedule(const json& step, size_t& idx, const json& script);
    void executeDelay(const json& step);
    void executeParallel(const json& step, size_t& idx, const json& script);
    void executeRetry(const json& step, size_t& idx, const json& script);

    void executeSteps(const json& steps, size_t& idx, const json& script);

    void executeThrow(const json& step);
    void executeTryCatch(const json& step, size_t& idx, const json& script);
    void executeFunction(const json& step);
    void executeReturn(const json& step, size_t& idx);
    void executeBreak(const json& step, size_t& idx);
    void executeContinue(const json& step, size_t& idx);

    void executeMessage(const json& step);
    void executeBroadcastEvent(const json& step);
    void executeListenEvent(const json& step, size_t& idx);

    auto evaluate(const json& value) -> json;
    auto evaluateExpression(const std::string& expr) -> json;
    auto precedence(char op) -> int;

    void throwCustomError(const std::string& name);
    void handleException(const std::string& scriptName,
                         const std::exception& e);

    std::unique_ptr<TaskInterpreterImpl> impl_;
};

}  // namespace lithium

#endif
