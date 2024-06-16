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
        throw std::runtime_error("Failed to prepare script: " + name);
    }
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
        throw std::runtime_error("Function '" + name +
                                 "' is already registered.");
    }
    functions[name] = func;
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
        throw std::runtime_error("Variable '" + name + "' is not defined.");
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
                throw std::runtime_error("Script '" + scriptName +
                                         "' not found.");
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
            std::cerr << "Error during script execution: " << e.what()
                      << std::endl;
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
        std::cerr << "Exception caught during step execution: " << e.what()
                  << std::endl;
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
        auto task = std::make_shared<SimpleTask>(
            functionName, params, functions[functionName],
            [this](const std::exception& e) {
                std::cerr << "Task failed: " << e.what() << std::endl;
            });
        task->execute();
        std::cout << "Function " << functionName
                  << " executed, result: " << task->getResult().dump()
                  << std::endl;
    } else {
        std::cout << "Function " << functionName << " not found." << std::endl;
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
        throw std::runtime_error("Label '" + label + "' not found.");
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

void TaskInterpretor::executeParallel(const json& step, size_t& idx,
                                      const json& script) {
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<SimpleTask>> tasks;

    for (const auto& nestedStep : step["steps"]) {
        threads.emplace_back([this, &nestedStep, &idx, &script, &tasks]() {
            auto task = std::make_shared<SimpleTask>(
                nestedStep["function"], nestedStep["params"],
                functions[nestedStep["function"]],
                [this](const std::exception& e) {
                    std::cerr << "Task failed: " << e.what() << std::endl;
                });
            tasks.push_back(task);
            task->execute();
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
        std::cerr << "Unhandled exception in script '" << scriptName
                  << "': " << e.what() << std::endl;
    }
}

}  // namespace lithium
