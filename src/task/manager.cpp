#include "manager.hpp"
#include "generator.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

using namespace std::literals;
using json = nlohmann::json;

namespace lithium {
TaskInterpretor::TaskInterpretor()
    : taskGenerator(std::make_shared<TaskGenerator>()) {}

TaskInterpretor::~TaskInterpretor() {
    if (executionThread.joinable()) {
        stop();
        executionThread.join();
    }
}

void TaskInterpretor::loadScript(const std::string& name, const json& script) {
    scripts[name] = script;
    if (prepareScript(scripts[name])) {
        parseLabels(script);
    } else {
        THROW_RUNTIME_ERROR("Failed to prepare script: " + name);
    }
}

void TaskInterpretor::unloadScript(const std::string& name) {
    scripts.erase(name);
}

bool TaskInterpretor::hasScript(const std::string& name) const {
    return scripts.find(name) != scripts.end();
}

std::optional<json> TaskInterpretor::getScript(const std::string& name) const {
    if (auto it = scripts.find(name); it != scripts.end()) {
        return scripts.at(name);
    }
    return std::nullopt;
}

bool TaskInterpretor::prepareScript(json& script) {
    try {
        taskGenerator->process_json(script);
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

void TaskInterpretor::registerFunction(const std::string& name,
                                       std::function<json(const json&)> func) {
    if (functions.find(name) != functions.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is already registered.");
    }
    functions[name] = func;
}

bool TaskInterpretor::hasFunction(const std::string& name) const {
    if (functions.find(name) == functions.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is not registered.");
    }
    return true;
}

void TaskInterpretor::registerExceptionHandler(
    const std::string& name,
    std::function<void(const std::exception&)> handler) {
    exceptionHandlers[name] = handler;
}

void TaskInterpretor::setVariable(const std::string& name, const json& value) {
    variables[name] = value;
}

json TaskInterpretor::getVariable(const std::string& name) {
    if (variables.find(name) == variables.end()) {
        THROW_RUNTIME_ERROR("Variable '" + name + "' is not defined.");
    }
    return variables[name];
}

void TaskInterpretor::parseLabels(const json& script) {
    for (size_t i = 0; i < script.size(); ++i) {
        if (script[i].contains("label")) {
            labels[script[i]["label"]] = i;
        }
    }
}

void TaskInterpretor::execute(const std::string& scriptName) {
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

void TaskInterpretor::stop() {
    stopRequested = true;
    if (executionThread.joinable()) {
        executionThread.join();
    }
}

bool TaskInterpretor::executeStep(const json& step, size_t& idx,
                                  const json& script) {
    if (stopRequested)
        return false;
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

void TaskInterpretor::executeCall(const json& step) {
    std::string functionName = step["function"];
    json params = step["params"];
    for (auto& [key, value] : params.items()) {
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

void TaskInterpretor::executeCondition(const json& step, size_t& idx,
                                       const json& script) {
    json condition = evaluate(step["condition"]);
    if (condition.get<bool>()) {
        executeStep(step["true"], idx, script);
    } else if (step.contains("false")) {
        executeStep(step["false"], idx, script);
    }
}

bool TaskInterpretor::executeLoop(const json& step, size_t& idx,
                                  const json& script) {
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

void TaskInterpretor::executeGoto(const json& step, size_t& idx,
                                  [[maybe_unused]] const json& script) {
    std::string label = step["label"];
    if (labels.find(label) != labels.end()) {
        idx = labels[label] - 1;  // Adjust for subsequent increment in the loop
    } else {
        THROW_RUNTIME_ERROR("Label '" + label + "' not found.");
    }
}

void TaskInterpretor::executeSwitch(const json& step, size_t& idx,
                                    const json& script) {
    std::string variable = step["variable"];
    json value = evaluate(
        variables[variable]);  // Evaluate the variable to get its value

    bool caseFound = false;  // Flag to check if a case has been matched

    if (step.contains("cases")) {
        for (const auto& case_block : step["cases"]) {
            if (case_block["case"] == value) {
                for (const auto& nestedStep : case_block["steps"]) {
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

void TaskInterpretor::executeDelay(const json& step) {
    int milliseconds = evaluate(step["milliseconds"]).get<int>();
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void TaskInterpretor::executeParallel(const json& step,
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

json TaskInterpretor::evaluate(const json& value) {
    if (value.is_primitive()) {
        return value;
    } else if (value.is_string() &&
               variables.contains(value.get<std::string>())) {
        return variables[value.get<std::string>()];
    }
    return value;
}

void TaskInterpretor::handleException(const std::string& scriptName,
                                      const std::exception& e) {
    if (exceptionHandlers.find(scriptName) != exceptionHandlers.end()) {
        exceptionHandlers[scriptName](e);
    } else {
        LOG_F(ERROR, "Unhandled exception in script '{}': {}", scriptName,
              e.what());
    }
}

}  // namespace lithium
