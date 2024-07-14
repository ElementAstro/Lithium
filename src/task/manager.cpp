#include "manager.hpp"
#include "generator.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
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
    std::jthread executionThread_;
    std::vector<std::string> callStack_;
    std::mutex mtx_;
    std::condition_variable_any cv_;
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
    if (impl_->functions_.find(name) != impl_->functions_.end()) {
        THROW_RUNTIME_ERROR("Function '" + name + "' is already registered.");
    }
    impl_->functions_[name] = std::move(func);
}

void TaskInterpreter::registerExceptionHandler(
    const std::string& name,
    std::function<void(const std::exception&)> handler) {
    impl_->exceptionHandlers_[name] = std::move(handler);
}

void TaskInterpreter::setVariable(const std::string& name, const json& value) {
    impl_->variables_[name] = value;
}

auto TaskInterpreter::getVariable(const std::string& name) -> json {
    if (impl_->variables_.find(name) == impl_->variables_.end()) {
        THROW_RUNTIME_ERROR("Variable '" + name + "' is not defined.");
    }
    return impl_->variables_[name];
}

void TaskInterpreter::parseLabels(const json& script) {
    for (size_t i = 0; i < script.size(); ++i) {
        if (script[i].contains("label")) {
            impl_->labels_[script[i]["label"]] = i;
        }
    }
}

void TaskInterpreter::execute(const std::string& scriptName) {
    impl_->stopRequested_ = false;
    if (impl_->executionThread_.joinable()) {
        impl_->executionThread_.join();
    }

    impl_->executionThread_ = std::jthread([this, scriptName]() {
        try {
            if (impl_->scripts_.find(scriptName) == impl_->scripts_.end()) {
                THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
            }
            const json& script = impl_->scripts_[scriptName];
            size_t i = 0;
            while (i < script.size() && !impl_->stopRequested_) {
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
    impl_->stopRequested_ = true;
    if (impl_->executionThread_.joinable()) {
        impl_->executionThread_.join();
    }
}

void TaskInterpreter::pause() { impl_->pauseRequested_ = true; }

void TaskInterpreter::resume() {
    impl_->pauseRequested_ = false;
    impl_->cv_.notify_all();
}

void TaskInterpreter::queueEvent(const std::string& eventName,
                                 const json& eventData) {
    std::lock_guard<std::mutex> lock(impl_->mtx_);
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
                                  const json& /*script*/) {
    std::string label = step["label"];
    if (impl_->labels_.find(label) != impl_->labels_.end()) {
        idx = impl_->labels_[label] -
              1;  // Adjust for subsequent increment in the loop
    } else {
        THROW_RUNTIME_ERROR("Label '" + label + "' not found.");
    }
}

void TaskInterpreter::executeSwitch(const json& step, size_t& idx,
                                    const json& script) {
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
    int milliseconds = evaluate(step["milliseconds"]).get<int>();
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void TaskInterpreter::executeParallel(const json& step, size_t& idx,
                                      const json& script) {
    std::vector<std::jthread> threads;

    for (const auto& nestedStep : step["steps"]) {
        threads.emplace_back([this, &nestedStep, &script]() {
            size_t nestedIdx = 0;
            executeStep(nestedStep, nestedIdx, script);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

void TaskInterpreter::executeCall(const json& step) {
    std::string functionName = step["function"];
    json params = step["params"];
    for (const auto& [key, value] : params.items()) {
        params[key] = evaluate(value);
    }
    if (impl_->functions_.find(functionName) != impl_->functions_.end()) {
        auto task = std::make_shared<Task>(
            functionName, params, impl_->functions_[functionName],
            [](const std::exception& e) {
                LOG_F(ERROR, "Task failed: {}", e.what());
            });
        task->run();
        LOG_F(INFO, "Task {} executed", functionName);
    } else {
        LOG_F(ERROR, "Function {} not found.", functionName);
    }
}

void TaskInterpreter::executeNestedScript(const json& step) {
    std::string scriptName = step["script"];
    if (impl_->scripts_.find(scriptName) != impl_->scripts_.end()) {
        execute(scriptName);
    } else {
        THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
    }
}

void TaskInterpreter::executeAssign(const json& step) {
    std::string variable = step["variable"];
    json value = evaluate(step["value"]);
    impl_->variables_[variable] = value;
}

void TaskInterpreter::executeImport(const json& step) {
    std::string scriptName = step["script"];
    if (impl_->scripts_.find(scriptName) == impl_->scripts_.end()) {
        THROW_RUNTIME_ERROR("Script '" + scriptName + "' not found.");
    }
    loadScript(scriptName, impl_->scripts_[scriptName]);
}

void TaskInterpreter::executeWaitEvent(const json& step) {
    std::string eventName = step["event"];
    std::unique_lock lk(impl_->mtx_);
    impl_->cv_.wait(lk, [this, &eventName]() {
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
    std::jthread([this, step]() {
        size_t idx = 0;
        executeStep(step, idx, step);
    }).detach();
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

void TaskInterpreter::executeFunction(const json& step) {
    std::string functionName = step["name"];
    json params = step.contains("params") ? step["params"] : json::object();
    if (impl_->functions_.find(functionName) != impl_->functions_.end()) {
        impl_->functions_[functionName](params);
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

auto TaskInterpreter::evaluate(const json& value) -> json {
    if (value.is_primitive()) {
        return value;
    }
    if (value.is_string() &&
        impl_->variables_.contains(value.get<std::string>())) {
        return impl_->variables_[value.get<std::string>()];
    }
    return value;
}

void TaskInterpreter::handleException(const std::string& scriptName,
                                      const std::exception& e) {
    if (impl_->exceptionHandlers_.find(scriptName) !=
        impl_->exceptionHandlers_.end()) {
        impl_->exceptionHandlers_[scriptName](e);
    } else {
        LOG_F(ERROR, "Unhandled exception in script '{}': {}", scriptName,
              e.what());
    }
}

}  // namespace lithium
