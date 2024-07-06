#include "manager.hpp"
#include "generator.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "task.hpp"

using namespace std::literals;
using json = nlohmann::json;

namespace lithium {

class TaskInterpreterImpl {
public:
    std::unordered_map<std::string, json> scripts_;
    json variables_;
    std::unordered_map<std::string, std::function<json(const json&)>>
        functions_;
    std::unordered_map<std::string, size_t> labels_;
    std::unordered_map<std::string, std::function<void(const std::exception&)>>
        exceptionHandlers_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> pauseRequested_{false};
    std::thread executionThread_;
    std::vector<std::string> callStack_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::pair<std::string, json>> eventQueue_;

    std::shared_ptr<TaskGenerator> taskGenerator_;
};

TaskInterpreter::TaskInterpreter()
    : impl_(std::make_unique<TaskInterpreterImpl>()) {}

TaskInterpreter::~TaskInterpreter() {
    if (impl_->executionThread_.joinable()) {
        stop();
        impl_->executionThread_.join();
    }
}

void TaskInterpreter::loadScript(const std::string& name, const json& script) {
    impl_->scripts_[name] = script;
    if (prepareScript(impl_->scripts_[name])) {
        parseLabels(script);
    } else {
        THROW_RUNTIME_ERROR("Failed to prepare script: " + name);
    }
}

void TaskInterpreter::unloadScript(const std::string& name) {
    if (hasScript(name)) {
        impl_->scripts_.erase(name);
    }
}

bool TaskInterpreter::hasScript(const std::string& name) const {
    return impl_->scripts_.contains(name);
}

auto TaskInterpreter::getScript(const std::string& name) const
    -> std::optional<json> {
    if (hasScript(name)) {
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
    if (impl_->functions_.find(name) != functions.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is already registered.");
    }
    functions[name] = std::move(func);
}

auto TaskInterpreter::hasFunction(const std::string& name) const -> bool {
    if (functions.find(name) == functions.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is not registered.");
    }
    return true;
}

void TaskInterpreter::registerExceptionHandler(
    const std::string& name,
    std::function<void(const std::exception&)> handler) {
    exceptionHandlers[name] = std::move(handler);
}

void TaskInterpreter::setVariable(const std::string& name, const json& value) {
    variables[name] = value;
}

auto TaskInterpreter::getVariable(const std::string& name) -> json {
    if (variables.find(name) == variables.end()) {
        THROW_RUNTIME_ERROR("Variable '" + name + "' is not defined.");
    }
    return variables[name];
}

void TaskInterpreter::parseLabels(const json& script) {
    for (size_t i = 0; i < script.size(); ++i) {
        if (script[i].contains("label")) {
            labels[script[i]["label"]] = i;
        }
    }
}

void TaskInterpreter::execute(const std::string& scriptName) {
    stopRequested = false;
    if (executionThread.joinable()) {
        executionThread.join();
    }

    executionThread = std::thread([this, scriptName]() {
        try {
            if (scripts.find(scriptName) == scripts.end()) {
                THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
            }
            const json& script = scripts[scriptName];
            size_t i = 0;
            while (i < script.size() && !stopRequested) {
                if (!executeStep(script[i], i, script)) {
                    break;
                }
                i++;
            }
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Error during script execution: {}", e.what());
            handleException(scriptName, e);
        }
    });
}

void TaskInterpreter::stop() {
    stopRequested = true;
    if (executionThread.joinable()) {
        executionThread.join();
    }
}

auto TaskInterpreter::executeStep(const json& step, size_t& idx,
                                  const json& script) -> bool {
    if (stopRequested) {
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
        }
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error during step execution: {}", e.what());
        return false;  // Optionally, stop execution or handle differently
    }
}

void TaskInterpreter::executeCall(const json& step) {
    std::string functionName = step["function"];
    json params = step["params"];
    for (const auto& [key, value] : params.items()) {
        params[key] = evaluate(value);
    }
    if (functions.find(functionName) != functions.end()) {
        auto task = std::make_shared<Task>(
            functionName, params, functions[functionName],
            [](const std::exception& e) {
                LOG_F(ERROR, "Task failed: {}", e.what());
            });
        task->run();
        LOG_F(INFO, "Task {} executed", functionName);
    } else {
        LOG_F(ERROR, "Function {} not found.", functionName);
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
    for (int i = 0; i < count && !stopRequested; i++) {
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
                                  [[maybe_unused]] const json& script) {
    std::string label = step["label"];
    if (labels.find(label) != labels.end()) {
        idx = labels[label] - 1;  // Adjust for subsequent increment in the loop
    } else {
        THROW_RUNTIME_ERROR("Label '" + label + "' not found.");
    }
}

void TaskInterpreter::executeSwitch(const json& step, size_t& idx,
                                    const json& script) {
    std::string variable = step["variable"];
    json value = evaluate(
        variables[variable]);  // Evaluate the variable to get its value

    bool caseFound = false;  // Flag to check if a case has been matched

    if (step.contains("cases")) {
        for (const auto& caseBlock : step["cases"]) {
            if (caseBlock["case"] == value) {
                for (const auto& nestedStep : caseBlock["steps"]) {
                    executeStep(nestedStep, idx, script);
                }
                caseFound = true;  // Mark that a case has been matched
                break;  // Break after matching case to avoid executing
                        // other cases
            }
        }
    }

    // Execute default block if no case has been matched
    if (!caseFound && step.contains("default")) {
        for (const auto& nestedStep : step["default"]["steps"]) {
            executeStep(nestedStep, idx, script);
        }
    }
}

void TaskInterpreter::executeDelay(const json& step) {
    int milliseconds = evaluate(step["milliseconds"]).get<int>();
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void TaskInterpreter::executeParallel(const json& step,
                                      [[maybe_unused]] size_t& idx,
                                      [[maybe_unused]] const json& script) {
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Task>> tasks;

    for (const auto& nestedStep : step["steps"]) {
        threads.emplace_back([this, &nestedStep, &tasks]() {
            auto task = std::make_shared<Task>(
                nestedStep["function"], nestedStep["params"],
                functions[nestedStep["function"]], [](const std::exception& e) {
                    LOG_F(ERROR, "Task failed: {}", e.what());
                });
            tasks.push_back(task);
            task->run();
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

json TaskInterpreter::evaluate(const json& value) {
    if (value.is_primitive()) {
        return value;
    }
    if (value.is_string() && variables.contains(value.get<std::string>())) {
        return variables[value.get<std::string>()];
    }
    return value;
}

void TaskInterpreter::handleException(const std::string& scriptName,
                                      const std::exception& e) {
    if (exceptionHandlers.find(scriptName) != exceptionHandlers.end()) {
        exceptionHandlers[scriptName](e);
    } else {
        LOG_F(ERROR, "Unhandled exception in script '{}': {}", scriptName,
              e.what());
    }
}

}  // namespace lithium
