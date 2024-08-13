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
// #include "pool.hpp"
#include "task.hpp"
#include "eval.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <shared_mutex>
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

#define ENABLE_DEBUG 1

using namespace std::literals;
using json = nlohmann::json;

auto operator<<(std::ostream& os, const std::error_code& ec) -> std::ostream& {
    os << "Error Code: " << ec.value() << ", Category: " << ec.category().name()
       << ", Message: " << ec.message();
    return os;
}

namespace lithium {

auto determineType(const json& value) -> VariableType {
    if (value.is_number_integer()) {
        return VariableType::INTEGER;
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
    // std::shared_ptr<TaskPool> taskPool_;
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
    LOG_F(INFO, "Loading script: {} with {}", name, script.dump());
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
        LOG_F(ERROR, "Error during step execution: {}", e.what());
        handleException(script["name"], e);
        return false;
    }
}

void TaskInterpreter::executeCondition(const json& step, size_t& idx,
                                       const json& script) {
    json condition = evaluate(step["condition"]);
    if (condition.get<bool>()) {
        executeStep(step["true"], idx, script);
    } else if (step.contains("false")) {
        executeStep(step["false"], idx, script);
    }
}

auto TaskInterpreter::executeLoop(const json& step, size_t& idx,
                                  const json& script) -> bool {
    int count = evaluate(step["loop_iterations"]).get<int>();
    size_t startIdx = idx;
    for (int i = 0; i < count && !impl_->stopRequested_; i++) {
        for (const auto& nestedStep : step["steps"]) {
            if (!executeStep(nestedStep, idx, script)) {
                return false;  // Allows breaking the loop from inside
            }
        }
    }
    idx = startIdx;
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
    if (!step.contains("variable")) {
        THROW_MISSING_ARGUMENT("Missing 'variable' parameter.");
    }
    std::string variable = step["variable"];
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

    if (!caseFound && step.contains("default")) {
        for (const auto& nestedStep : step["default"]["steps"]) {
            executeStep(nestedStep, idx, script);
        }
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
    std::vector<std::future<void>> futures;

    for (const auto& nestedStep : step["steps"]) {
        futures.emplace_back(
            impl_->threadPool_->enqueue([this, nestedStep, &script]() {
                size_t nestedIdx = 0;
                executeStep(nestedStep, nestedIdx, script);
            }));
    }

    for (auto& future : futures) {
        future.get();
    }
}

void TaskInterpreter::executeCall(const json& step) {
    // 获取函数名
    try {
        std::string functionName = step["function"];

        // 获取参数
        json params = step.contains("params") ? step["params"] : json::object();

        // 评估参数的值
        for (const auto& [key, value] : params.items()) {
            params[key] = evaluate(value)[1];
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
    } catch (const std::system_error& e) {
        LOG_F(ERROR, "Error during step execution: {} with code: {}", e.what(),
              e.code().message());
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
    std::string variable = step["variable"];
    json value = evaluate(step["value"]);
#if ENABLE_DEBUG
    std::cout << "Assigning value: " << value.dump()
              << " to variable: " << variable << std::endl;
#endif
    std::unique_lock lock(impl_->mtx_);
    impl_->variables_[variable] = {determineType(value), value};
}

void TaskInterpreter::executeImport(const json& step) {
    std::string scriptName = step["script"];
    std::shared_lock lock(impl_->mtx_);
    if (impl_->scripts_.find(scriptName) == impl_->scripts_.end()) {
        THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
    }
    loadScript(scriptName, impl_->scripts_[scriptName]);
}

void TaskInterpreter::executeWaitEvent(const json& step) {
    std::string eventName = step["event"];
    std::unique_lock lock(impl_->mtx_);
    impl_->cv_.wait(lock, [this, &eventName]() {
        return !impl_->eventQueue_.empty() &&
               impl_->eventQueue_.front().first == eventName;
    });
    impl_->eventQueue_.pop();
}

void TaskInterpreter::executePrint(const json& step) {
    std::string message = evaluate(step["message"]).get<std::string>();
    std::cout << message << std::endl;
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
    std::cout << message << std::endl;
}

auto TaskInterpreter::evaluate(const json& value) -> json {
    if (value.is_string() &&
        impl_->variables_.contains(value.get<std::string>())) {
        std::shared_lock lock(impl_->mtx_);
        return impl_->variables_.at(value.get<std::string>());
    }

    if (value.is_number() || value.is_boolean()) {
        return value;
    }

    if (value.is_object()) {
        if (value.contains("$")) {  // similar to python's eval()

        } else if (value.contains("$eq")) {  // 等于比较
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
                if (determineType(left[1]) != determineType(right)) {
                    THROW_RUNTIME_ERROR(
                        "Type mismatch in equality comparison: " +
                        value.dump());
                }
                return left[1] == right;
            }
        } else if (value.contains("$gt")) {  // 大于比较
            auto operands = value["$gt"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0]);
                auto right = evaluate(operands[1]);
                return left[1] > right;
            }
        } else if (value.contains("$lt")) {  // 小于比较
            auto operands = value["$lt"];
            if (operands.is_array() && operands.size() == 2) {
                auto left = evaluate(operands[0]);
                auto right = evaluate(operands[1]);
                return left[1] < right;
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
