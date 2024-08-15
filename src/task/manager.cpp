/**
 * @file task_interpreter.cpp
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

#include "manager.hpp"
#include "generator.hpp"
#include "task.hpp"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <stack>
#include <string>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "atom/async/pool.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "task/loader.hpp"

#include "utils/constant.hpp"

// #define ENABLE_DEBUG 1

#if ENABLE_DEBUG
#include <iostream>
#endif

using namespace std::literals;
using json = nlohmann::json;

auto operator<<(std::ostream& os, const std::error_code& ec) -> std::ostream& {
    os << "Error Code: " << ec.value() << ", Category: " << ec.category().name()
       << ", Message: " << ec.message();
    return os;
}

namespace lithium {

auto determineType(const json& value) -> VariableType {
    if (value.is_number()) {
        return VariableType::NUMBER;
    }
    if (value.is_string()) {
        return VariableType::STRING;
    }
    if (value.is_boolean()) {
        return VariableType::BOOLEAN;
    }
    if (value.is_object() || value.is_array()) {
        return VariableType::JSON;
    }
    return VariableType::UNKNOWN;
}

class TaskInterpreterImpl {
public:
    std::unordered_map<std::string, json> scripts_;
    std::unordered_map<std::string, std::pair<VariableType, json>> variables_;
    std::unordered_map<std::string, std::function<json(const json&)>>
        functions_;
    std::unordered_map<std::string, size_t> labels_;
    std::unordered_map<std::string, std::function<void(const std::exception&)>>
        exceptionHandlers_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> pauseRequested_{false};
    std::atomic<bool> isRunning_{false};
    std::jthread executionThread_;
    std::vector<std::string> callStack_;
    mutable std::shared_mutex mtx_;
    std::condition_variable_any cv_;
    std::queue<std::pair<std::string, json>> eventQueue_;

    std::shared_ptr<TaskGenerator> taskGenerator_;
    std::shared_ptr<atom::async::ThreadPool<>> threadPool_;
};

TaskInterpreter::TaskInterpreter()
    : impl_(std::make_unique<TaskInterpreterImpl>()) {
    if (auto ptr = GetPtrOrCreate<atom::async::ThreadPool<>>(
            "lithium.task.pool",
            [] { return std::make_shared<atom::async::ThreadPool<>>(); });
        ptr) {
        impl_->threadPool_ = ptr;
    } else {
        THROW_RUNTIME_ERROR("Failed to create task pool.");
    }
    if (auto ptr = GetPtrOrCreate<TaskGenerator>("lithium.task.generator", [] {
            return std::make_shared<TaskGenerator>();
        })) {
        impl_->taskGenerator_ = ptr;
    } else {
        THROW_RUNTIME_ERROR("Failed to create task generator.");
    }
}

TaskInterpreter::~TaskInterpreter() {
    if (impl_->executionThread_.joinable()) {
        stop();
        // impl_->executionThread_.join();
    }
}

auto TaskInterpreter::createShared() -> std::shared_ptr<TaskInterpreter> {
    return std::make_shared<TaskInterpreter>();
}

void TaskInterpreter::loadScript(const std::string& name, const json& script) {
#if ENABLE_DEBUG
    LOG_F(INFO, "Loading script: {} with {}", name, script.dump());
#endif
    std::unique_lock lock(impl_->mtx_);
    impl_->scripts_[name] = script;
    lock.unlock();
    if (prepareScript(impl_->scripts_[name])) {
        parseLabels(script);
    } else {
        THROW_RUNTIME_ERROR("Failed to prepare script: " + name);
    }
}

void TaskInterpreter::unloadScript(const std::string& name) {
    std::unique_lock lock(impl_->mtx_);
    impl_->scripts_.erase(name);
}

auto TaskInterpreter::hasScript(const std::string& name) const -> bool {
    std::shared_lock lock(impl_->mtx_);
    return impl_->scripts_.contains(name);
}

auto TaskInterpreter::getScript(const std::string& name) const
    -> std::optional<json> {
    std::shared_lock lock(impl_->mtx_);
    if (impl_->scripts_.contains(name)) {
        return impl_->scripts_.at(name);
    }
    return std::nullopt;
}

auto TaskInterpreter::prepareScript(json& script) -> bool {
    try {
        impl_->taskGenerator_->processJson(script);
    } catch (const json::parse_error& e) {
        LOG_F(ERROR, "Failed to parse script: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to process script: {}", e.what());
        return false;
    }
    return true;
}

void TaskInterpreter::registerFunction(const std::string& name,
                                       std::function<json(const json&)> func) {
    std::unique_lock lock(impl_->mtx_);
    if (impl_->functions_.find(name) != impl_->functions_.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is already registered.");
    }
    impl_->functions_[name] = std::move(func);
}

void TaskInterpreter::registerExceptionHandler(
    const std::string& name,
    std::function<void(const std::exception&)> handler) {
    std::unique_lock lock(impl_->mtx_);
    impl_->exceptionHandlers_[name] = std::move(handler);
}

void TaskInterpreter::setVariable(const std::string& name, const json& value,
                                  VariableType type) {
    std::unique_lock lock(impl_->mtx_);
    impl_->cv_.wait(lock, [this]() { return !impl_->isRunning_; });

    VariableType currentType = determineType(value);
    if (currentType != type) {
        THROW_RUNTIME_ERROR(
            "Type mismatch when setting variable '" + name + "'. Expected " +
            std::to_string(static_cast<int>(type)) + ", got " +
            std::to_string(static_cast<int>(currentType)) + ".");
    }

    if (impl_->variables_.find(name) != impl_->variables_.end()) {
        if (impl_->variables_[name].first != type) {
            THROW_RUNTIME_ERROR("Type mismatch: Variable '" + name +
                                "' already exists with a different type.");
        }
    }

    impl_->variables_[name] = {type, value};
}

auto TaskInterpreter::getVariable(const std::string& name) const -> json {
    std::shared_lock lock(impl_->mtx_);
    impl_->cv_.wait(lock, [this]() { return !impl_->isRunning_; });
    if (impl_->variables_.find(name) == impl_->variables_.end()) {
        THROW_RUNTIME_ERROR("Variable '" + name + "' is not defined.");
    }
    return impl_->variables_.at(name).second;
}

void TaskInterpreter::parseLabels(const json& script) {
    std::unique_lock lock(impl_->mtx_);
    LOG_F(INFO, "Parsing labels...");
    for (size_t i = 0; i < script.size(); ++i) {
        if (script[i].contains("label")) {
            impl_->labels_[script[i]["label"]] = i;
        }
    }
}

void TaskInterpreter::execute(const std::string& scriptName) {
    impl_->stopRequested_ = false;
    impl_->isRunning_ = true;
    if (impl_->executionThread_.joinable()) {
        impl_->executionThread_.join();
    }

    if (!impl_->scripts_.contains(scriptName)) {
        THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
    }

    impl_->executionThread_ = std::jthread([this, scriptName]() {
        std::exception_ptr exPtr = nullptr;
        try {
            std::shared_lock lock(impl_->mtx_);
            const json& script = impl_->scripts_.at(scriptName);
            lock.unlock();

            size_t i = 0;
            while (i < script.size() && !impl_->stopRequested_) {
                if (!executeStep(script[i], i, script)) {
                    break;
                }
                i++;
            }
        } catch (...) {
            exPtr = std::current_exception();
        }

        impl_->isRunning_ = false;
        impl_->cv_.notify_all();

        if (exPtr) {
            try {
                std::rethrow_exception(exPtr);
            } catch (const std::exception& e) {
                handleException(scriptName, e);
            }
        }
    });
}

void TaskInterpreter::stop() {
    impl_->stopRequested_ = true;
    if (impl_->executionThread_.joinable()) {
        impl_->executionThread_.join();
    }
}

void TaskInterpreter::pause() {
    LOG_F(INFO, "Pausing task interpreter...");
    impl_->pauseRequested_ = true;
}

void TaskInterpreter::resume() {
    LOG_F(INFO, "Resuming task interpreter...");
    impl_->pauseRequested_ = false;
    impl_->cv_.notify_all();
}

void TaskInterpreter::queueEvent(const std::string& eventName,
                                 const json& eventData) {
    std::unique_lock lock(impl_->mtx_);
    impl_->eventQueue_.emplace(eventName, eventData);
    impl_->cv_.notify_all();
}

auto TaskInterpreter::executeStep(const json& step, size_t& idx,
                                  const json& script) -> bool {
    if (impl_->stopRequested_) {
        return false;
    }

    try {
        std::string type = step["type"];
        if (type == "call") {
            executeCall(step);
        } else if (type == "condition") {
            executeCondition(step, idx, script);
        } else if (type == "loop") {
            return executeLoop(step, idx, script);
        } else if (type == "goto") {
            executeGoto(step, idx, script);
        } else if (type == "switch") {
            executeSwitch(step, idx, script);
        } else if (type == "delay") {
            executeDelay(step);
        } else if (type == "parallel") {
            executeParallel(step, idx, script);
        } else if (type == "nested_script") {
            executeNestedScript(step);
        } else if (type == "assign") {
            executeAssign(step);
        } else if (type == "import") {
            executeImport(step);
        } else if (type == "wait_event") {
            executeWaitEvent(step);
        } else if (type == "print") {
            executePrint(step);
        } else if (type == "async") {
            executeAsync(step);
        } else if (type == "try") {
            executeTryCatch(step, idx, script);
        } else if (type == "function") {
            executeFunction(step);
        } else if (type == "return") {
            executeReturn(step, idx);
        } else if (type == "break") {
            executeBreak(step, idx);
        } else if (type == "continue") {
            executeContinue(step, idx);
        } else if (type == "message") {
            executeMessage(step);
        } else {
            LOG_F(ERROR, "Unknown step type: {}", type);
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during step {} execution: {}",
              step["type"].get<std::string>(), e.what());
        handleException(script["name"], e);
        return false;
    }
}

void TaskInterpreter::executeCondition(const json& step, size_t& idx,
                                       const json& script) {
    try {
        if (!step.contains("condition")) {
            THROW_INVALID_ARGUMENT(
                "Condition step is missing 'condition' field.");
        }

        json conditionResult = evaluate(step["condition"]);

        if (!conditionResult.is_boolean()) {
            THROW_INVALID_ARGUMENT("Condition result must be boolean.");
        }

        // 根据条件执行分支
        if (conditionResult.get<bool>()) {
            executeStep(step["true"], idx, script);
        } else if (step.contains("false")) {
            executeStep(step["false"], idx, script);
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeCondition: {}", e.what());
        throw;
    }
}

auto TaskInterpreter::executeLoop(const json& step, size_t& idx,
                                  const json& script) -> bool {
    try {
        if (!step.contains("loop_iterations")) {
            THROW_INVALID_ARGUMENT(
                "Loop step is missing 'loop_iterations' field.");
        }

        int iterations = evaluate(step["loop_iterations"]).get<int>();

        for (int i = 0; i < iterations && !impl_->stopRequested_; i++) {
            for (const auto& nestedStep : step["steps"]) {
                if (!executeStep(nestedStep, idx, script)) {
                    return false;
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeLoop: {}", e.what());
        throw;
    }

    return true;
}

void TaskInterpreter::executeGoto(const json& step, size_t& idx,
                                  const json& script) {
    static const int MAX_GOTO_DEPTH = 100;  // 设置最大跳转深度
    static std::unordered_map<std::string, int>
        gotoDepthCounter;  // 跳转深度计数器
    static std::unordered_map<std::string, size_t> labelCache;  // 标签位置缓存

    // 标签字段验证
    if (!step.contains("label") || !step["label"].is_string()) {
        THROW_INVALID_ARGUMENT("Goto step is missing a valid 'label' field.");
    }

    // 获取标签和当前上下文
    std::string label = step["label"];
    std::string currentContext =
        script.contains("context") ? script["context"].get<std::string>() : "";
    std::string fullLabel =
        currentContext.empty() ? label : currentContext + "::" + label;

    // 检查缓存
    if (labelCache.find(fullLabel) != labelCache.end()) {
        idx = labelCache[fullLabel];
        gotoDepthCounter[fullLabel]++;
        if (gotoDepthCounter[fullLabel] > MAX_GOTO_DEPTH) {
            THROW_RUNTIME_ERROR("Exceeded maximum GOTO depth for label '" +
                                fullLabel + "'. Possible infinite loop.");
        }
        return;
    }

    // 查找标签并验证存在性
    if (impl_->labels_.find(fullLabel) == impl_->labels_.end()) {
        THROW_RUNTIME_ERROR("Label '" + fullLabel +
                            "' not found in the script.");
    }

    // 更新索引并缓存结果
    idx = impl_->labels_.at(fullLabel);
    labelCache[fullLabel] = idx;

    // 更新跳转深度计数器
    gotoDepthCounter[fullLabel] = 1;
}

void TaskInterpreter::executeSwitch(const json& step, size_t& idx,
                                    const json& script) {
    try {
        if (!step.contains("variable") || !step["variable"].is_string()) {
            THROW_MISSING_ARGUMENT("Missing 'variable' parameter.");
        }
        std::string variable = step["variable"];
        if (!impl_->variables_.contains(variable)) {
            THROW_OBJ_NOT_EXIST("Variable '" + variable + "' not found.");
        }

        json value = evaluate(impl_->variables_[variable]);

        bool caseFound = false;

        if (step.contains("cases")) {
            for (const auto& caseBlock : step["cases"]) {
                if (caseBlock["case"] == value) {
                    for (const auto& nestedStep : caseBlock["steps"]) {
                        executeStep(nestedStep, idx, script);
                    }
                    caseFound = true;
                    break;
                }
            }
        }

        if (!caseFound && step.contains("default") &&
            step["default"].contains("steps")) {
            for (const auto& nestedStep : step["default"]["steps"]) {
                executeStep(nestedStep, idx, script);
            }
        } else if (!caseFound) {
            LOG_F(WARNING, "No matching case found for variable '{}'",
                  variable);
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeSwitch: {}", e.what());
        throw;
    }
}

void TaskInterpreter::executeDelay(const json& step) {
    if (!step.contains("milliseconds")) {
        THROW_MISSING_ARGUMENT("Missing 'milliseconds' parameter.");
    }
    if (!step["milliseconds"].is_number()) {
        THROW_INVALID_ARGUMENT("'milliseconds' must be a number.");
    }
    auto milliseconds =
        std::chrono::milliseconds(evaluate(step["milliseconds"]).get<int>());
    std::this_thread::sleep_for(milliseconds);
}

void TaskInterpreter::executeParallel(const json& step,
                                      [[maybe_unused]] size_t& idx,
                                      const json& script) {
    try {
        if (!step.contains("steps") || !step["steps"].is_array()) {
            THROW_INVALID_ARGUMENT(
                "Parallel step is missing a valid 'steps' array.");
        }
        std::vector<std::future<void>> futures;

        for (const auto& nestedStep : step["steps"]) {
            futures.emplace_back(
                impl_->threadPool_->enqueue([this, nestedStep, &script]() {
                    try {
                        size_t nestedIdx = 0;
                        executeStep(nestedStep, nestedIdx, script);
                    } catch (const std::exception& e) {
                        LOG_F(ERROR, "Error during parallel task execution: {}",
                              e.what());
                        throw;
                    }
                }));
        }

        for (auto& future : futures) {
            future.get();
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeParallel: {}", e.what());
        std::throw_with_nested(e);
    }
}

void TaskInterpreter::executeCall(const json& step) {
    // 获取函数名
    try {
        if (!step.contains("function") || !step["function"].is_string()) {
            THROW_MISSING_ARGUMENT(
                "Call step is missing a valid 'function' field.");
        }
        std::string functionName = step["function"];

        // 获取参数
        json params = step.contains("params") ? step["params"] : json::object();

        // 评估参数的值
        for (const auto& [key, value] : params.items()) {
            params[key] = evaluate(value);
#if ENABLE_DEBUG
            std::cout << "Pure value: " << key << " = " << value.dump()
                      << std::endl;
            std::cout << "Evaluated parameter: " << key << " = "
                      << params[key].dump() << std::endl;
#endif
        }

        // 获取目标变量名（可选）
        std::string targetVariable =
            step.contains("result") ? step["result"].get<std::string>() : "";

        json returnValue;
        {
            std::shared_lock lock(impl_->mtx_);
            // 检查函数是否已注册
            if (impl_->functions_.contains(functionName)) {
                // 调用函数并获取返回值
                returnValue = impl_->functions_[functionName](params);
            } else {
                THROW_RUNTIME_ERROR("Function '" + functionName +
                                    "' not found.");
            }
        }

        // 如果指定了目标变量名，则将返回值存储到该变量中
        if (!targetVariable.empty()) {
            std::unique_lock ulock(impl_->mtx_);
            impl_->variables_[targetVariable] = {determineType(returnValue),
                                                 returnValue};
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeCall: {}", e.what());
    }
}

void TaskInterpreter::executeNestedScript(const json& step) {
    std::string scriptName = step["script"];
    std::shared_lock lock(impl_->mtx_);
    if (impl_->scripts_.find(scriptName) != impl_->scripts_.end()) {
        execute(scriptName);
    } else {
        THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
    }
}

void TaskInterpreter::executeAssign(const json& step) {
    try {
        if (!step.contains("variable") || !step["variable"].is_string()) {
            THROW_INVALID_ARGUMENT(
                "Assign step is missing a valid 'variable' field.");
        }

        if (!step.contains("value")) {
            THROW_INVALID_ARGUMENT("Assign step is missing 'value' field.");
        }

        std::string variableName = step["variable"];
        json value = evaluate(step["value"]);

        // 设置变量值
        setVariable(variableName, value, determineType(value));
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeAssign: {}", e.what());
        throw;
    }
}

void TaskInterpreter::executeImport(const json& step) {
    if (!step.contains("script") || !step["script"].is_string()) {
        THROW_INVALID_ARGUMENT(
            "Import step is missing a valid 'script' field.");
    }

    std::string scriptName = step["script"];

    if (impl_->scripts_.find(scriptName) != impl_->scripts_.end()) {
        LOG_F(WARNING, "Script '{}' already imported. Skipping import.",
              scriptName);
        return;
    }

    std::string namespaceName;
    if (step.contains("namespace") && step["namespace"].is_string()) {
        namespaceName = step["namespace"];
    }

    if (hasScript(scriptName)) {
        LOG_F(INFO, "Script '{}' already imported. Skipping import.",
              scriptName);
        return;
    }

    try {
        bool callbackCalled = false;
        json scriptToImport;
        TaskLoader::asyncReadJsonFile(
            constants::TASK_FOLDER, [&](std::optional<json> data) {
                if (!data) {
                    THROW_FILE_NOT_FOUND("Script '" + scriptName +
                                         "' not found.");
                }
                if (data->is_null() || data->empty()) {
                    THROW_JSON_VALUE_ERROR("Script '" + scriptName +
                                           " is empty or null.");
                }
                scriptToImport = std::move(*data);
                callbackCalled = true;
            });

        if (!namespaceName.empty()) {
            json namespacedScript;
            for (const auto& [key, value] : scriptToImport.items()) {
                namespacedScript[namespaceName + "::" + key] = value;
            }
            loadScript(scriptName, namespacedScript);
        } else {
            loadScript(scriptName, scriptToImport);
        }

        LOG_F(INFO, "Successfully imported script '{}'.", scriptName);

        if (scriptToImport.contains("imports")) {
            for (const auto& nestedImport : scriptToImport["imports"]) {
                executeImport(nestedImport);
            }
        }

    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to import script '{}': {}", scriptName, e.what());
        THROW_RUNTIME_ERROR("Error importing script '" + scriptName +
                            "': " + e.what());
    }
}

void TaskInterpreter::executeWaitEvent(const json& step) {
    try {
        if (!step.contains("event") || !step["event"].is_string()) {
            THROW_INVALID_ARGUMENT(
                "WaitEvent step is missing a valid 'event' field.");
        }
        std::string eventName = step["event"];
        std::unique_lock lock(impl_->mtx_);
        impl_->cv_.wait(lock, [this, &eventName]() {
            return !impl_->eventQueue_.empty() &&
                   impl_->eventQueue_.front().first == eventName;
        });
        impl_->eventQueue_.pop();
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during executeWaitEvent: {}", e.what());
        std::throw_with_nested(e);
    }
}

void TaskInterpreter::executePrint(const json& step) {
    std::string message = evaluate(step["message"]).get<std::string>();
    LOG_F(INFO, "{}", message);
}

void TaskInterpreter::executeAsync(const json& step) {
    impl_->threadPool_->enqueueDetach([this, step]() {
        size_t idx = 0;
        executeStep(step, idx, step);
    });
}

void TaskInterpreter::executeTryCatch(const json& step, size_t& idx,
                                      const json& script) {
    try {
        executeSteps(step["try"], idx, script);
    } catch (const std::exception& e) {
        handleException(script["name"], e);
        if (step.contains("catch")) {
            executeSteps(step["catch"], idx, script);
        }
    }
}

// TODO: Switch to self implementation of CommandDispatcher
void TaskInterpreter::executeFunction(const json& step) {
    LOG_F(INFO, "Executing step {}", step.dump());
    std::string functionName = step["name"];
    json params = step.contains("params") ? step["params"] : json::object();
    // 用于处理返回值
    std::string targetVariable =
        step.contains("result") ? step["result"].get<std::string>() : "";
    std::shared_lock lock(impl_->mtx_);
    if (impl_->functions_.contains(functionName)) {
        json returnValue = impl_->functions_[functionName](params);
        // 如果指定了目标变量名，则将返回值存储到该变量中
        if (!targetVariable.empty()) {
            std::unique_lock ulock(impl_->mtx_);
            impl_->variables_[targetVariable] = returnValue;
        }
    } else {
        THROW_RUNTIME_ERROR("Function '" + functionName + "' not found.");
    }
}

void TaskInterpreter::executeReturn(const json& /*step*/, size_t& idx) {
    idx = std::numeric_limits<size_t>::max();  // Terminate the script execution
}

void TaskInterpreter::executeBreak(const json& /*step*/, size_t& idx) {
    idx = std::numeric_limits<size_t>::max();  // Terminate the loop
}

void TaskInterpreter::executeContinue(const json& /*step*/, size_t& idx) {
    idx = std::numeric_limits<size_t>::max() - 1;  // Skip to the next iteration
}

void TaskInterpreter::executeSteps(const json& steps, size_t& idx,
                                   const json& script) {
    for (const auto& step : steps) {
        if (!executeStep(step, idx, script)) {
            break;
        }
    }
}

void TaskInterpreter::executeMessage(const json& step) {
    std::string message = evaluate(step["label"]).get<std::string>();
    LOG_F(INFO, "{}", message);
#if ENABLE_DEBUG
    std::cout << message << std::endl;
#endif
}

auto TaskInterpreter::evaluate(const json& value) -> json {
    if (value.is_string()) {
        std::string valStr = value.get<std::string>();

        // 检查是否是变量
        if (impl_->variables_.contains(valStr)) {
            std::shared_lock lock(impl_->mtx_);
            return impl_->variables_.at(valStr).second;
        }

        // 检查是否是表达式
        if (valStr.find_first_of("+-*/%^!&|<=>") != std::string::npos) {
            return evaluateExpression(valStr);  // 解析并评估表达式
        }

        // 检查是否是 $ 开头的运算符表达式
        if (valStr.starts_with("$")) {
            return evaluateExpression(
                valStr.substr(1));  // 去掉 $ 前缀后评估表达式
        }
    }

    if (value.is_number() || value.is_boolean()) {
        return value;
    }

    if (value.is_object()) {
        if (value.contains("$eq")) {  // 等于比较
            auto operands = value["$eq"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0]);
                auto right = evaluate(operands[1]);
#if ENABLE_DEBUG
                std::cout << "Left type: " << left.type_name()
                          << ", Right type: " << right.type_name() << std::endl;
                std::cout << "Evaluating equality comparison: " << left.dump()
                          << " == " << right.dump() << std::endl;
#endif
                LOG_F(INFO, "{} == {}", left.dump(), right.dump());
                if (determineType(left) != determineType(right)) {
                    THROW_RUNTIME_ERROR(
                        "Type mismatch in equality comparison: " +
                        value.dump());
                }
                return left == right;
            }
            THROW_RUNTIME_ERROR("Invalid equality comparison: " + value.dump());
        }
        if (value.contains("$gt")) {  // 大于比较
            auto operands = value["$gt"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0]);
                auto right = evaluate(operands[1]);
                return left > right;
            }
        } else if (value.contains("$lt")) {  // 小于比较
            auto operands = value["$lt"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0]);
                auto right = evaluate(operands[1]);
                return left < right;
            }
        } else if (value.contains("$add")) {  // 加法运算
            auto operands = value["$add"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0])[1].get<int>();
                auto right = evaluate(operands[1]).get<int>();
                return left + right;
            }
        } else if (value.contains("$sub")) {  // 减法运算
            auto operands = value["$sub"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0])[1].get<int>();
                auto right = evaluate(operands[1]).get<int>();
                return left - right;
            }
        } else if (value.contains("$mul")) {  // 乘法运算
            auto operands = value["$mul"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0])[1].get<int>();
                auto right = evaluate(operands[1]).get<int>();
                return left * right;
            }
        } else if (value.contains("$div")) {  // 除法运算
            auto operands = value["$div"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0])[1].get<int>();
                auto right = evaluate(operands[1]).get<int>();
                if (right == 0) {
                    THROW_RUNTIME_ERROR("Division by zero");
                }
                return left / right;
            }
        } else if (value.contains("$and")) {  // 逻辑与
            auto operands = value["$and"];
            if (operands.is_array()) {
                for (const auto& operand : operands) {
                    if (!evaluate(operand).get<bool>()) {
                        return false;
                    }
                }
                return true;
            }
        } else if (value.contains("$or")) {  // 逻辑或
            auto operands = value["$or"];
            if (operands.is_array()) {
                for (const auto& operand : operands) {
                    if (evaluate(operand).get<bool>()) {
                        return true;
                    }
                }
                return false;
            }
        }
    }
    return value;
}

auto TaskInterpreter::evaluateExpression(const std::string& expr) -> json {
    std::istringstream iss(expr);
    std::stack<char> operators;
    std::stack<double> operands;
    std::string token;

    auto applyOperator = [](char op, double a, double b) -> double {
        switch (op) {
            case '+':
                return a + b;
            case '-':
                return a - b;
            case '*':
                return a * b;
            case '/':
                if (b != 0) {
                    return a / b;
                } else {
                    THROW_INVALID_ARGUMENT("Division by zero.");
                }
            case '%':
                if (b != 0) {
                    return std::fmod(a, b);
                } else {
                    THROW_INVALID_ARGUMENT("Modulo by zero.");
                }
            case '^':
                return std::pow(a, b);
            case '<':
                return static_cast<double>(a < b);
            case '>':
                return static_cast<double>(a > b);
            case '=':
                return static_cast<double>(a == b);
            case '!':
                return static_cast<double>(a != b);
            case '&':
                return static_cast<double>(static_cast<bool>(a) &&
                                           static_cast<bool>(b));
            case '|':
                return static_cast<double>(static_cast<bool>(a) ||
                                           static_cast<bool>(b));
            default:
                THROW_RUNTIME_ERROR("Unknown operator.");
        }
    };

    while (iss >> token) {
        // 处理数字和变量
        if ((std::isdigit(token[0]) != 0) || token[0] == '.') {
            operands.push(std::stod(token));
        } else if (impl_->variables_.contains(token)) {
            operands.push(impl_->variables_.at(token).second.get<double>());
        } else if (token == "+" || token == "-" || token == "*" ||
                   token == "/" || token == "%" || token == "^" ||
                   token == "<" || token == ">" || token == "==" ||
                   token == "!=" || token == "&&" || token == "||") {
            // 处理操作符
            while (!operators.empty() &&
                   precedence(operators.top()) >= precedence(token[0])) {
                double b = operands.top();
                operands.pop();
                double a = operands.top();
                operands.pop();
                char op = operators.top();
                operators.pop();
                operands.push(applyOperator(op, a, b));
            }
            operators.push(token[0]);
        } else if (token == "(") {
            operators.push('(');
        } else if (token == ")") {
            while (!operators.empty() && operators.top() != '(') {
                double b = operands.top();
                operands.pop();
                double a = operands.top();
                operands.pop();
                char op = operators.top();
                operators.pop();
                operands.push(applyOperator(op, a, b));
            }
            if (operators.empty() || operators.top() != '(') {
                THROW_INVALID_ARGUMENT("Mismatched parentheses.");
            }
            operators.pop();  // 移除左括号
        } else {
            THROW_INVALID_ARGUMENT("Invalid token in expression: " + token);
        }
    }

    // 处理剩余操作符
    while (!operators.empty()) {
        double b = operands.top();
        operands.pop();
        double a = operands.top();
        operands.pop();
        char op = operators.top();
        operators.pop();
        operands.push(applyOperator(op, a, b));
    }

    if (operands.size() != 1) {
        THROW_INVALID_ARGUMENT("Invalid expression: " + expr);
    }

    return operands.top();
}

auto TaskInterpreter::precedence(char op) -> int {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
        case '%':
            return 2;
        case '^':
            return 3;
        case '<':
        case '>':
        case '=':
        case '!':
            return 4;
        case '&':
            return 5;
        case '|':
            return 6;
        default:
            return 0;
    }
}

void TaskInterpreter::handleException(const std::string& scriptName,
                                      const std::exception& e) {
    std::shared_lock lock(impl_->mtx_);
    if (impl_->exceptionHandlers_.contains(scriptName)) {
        impl_->exceptionHandlers_.at(scriptName)(e);
    } else {
        LOG_F(ERROR, "Unhandled exception in script '{}': {}", scriptName,
              e.what());
        std::rethrow_if_nested(e);
    }
}

}  // namespace lithium
