#ifndef LITHIUM_TASK_INTERPRETOR_HPP
#define LITHIUM_TASK_INTERPRETOR_HPP

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>
#include "atom/task/task.hpp"

using json = nlohmann::json;

namespace lithium {
class TaskGenerator;

class TaskInterpretor {
public:
    TaskInterpretor();

    ~TaskInterpretor();

    void loadScript(const std::string& name, const json& script);

    void registerFunction(const std::string& name,
                          std::function<json(const json&)> func);

    void registerExceptionHandler(
        const std::string& name,
        std::function<void(const std::exception&)> handler);

    void setVariable(const std::string& name, const json& value);

    json getVariable(const std::string& name);

    void parseLabels(const json& script);

    void execute(const std::string& scriptName);

    void stop();

private:
    bool prepareScript(json& script);

    bool executeStep(const json& step, size_t& idx, const json& script);

    void executeCall(const json& step);

    void executeCondition(const json& step, size_t& idx, const json& script);

    bool executeLoop(const json& step, size_t& idx, const json& script);

    void executeGoto(const json& step, size_t& idx,
                     [[maybe_unused]] const json& script);

    void executeSwitch(const json& step, size_t& idx, const json& script);

    void executeDelay(const json& step);

    void executeParallel(const json& step, size_t& idx, const json& script);

    json evaluate(const json& value);

    void handleException(const std::string& scriptName,
                         const std::exception& e);

private:
    std::shared_ptr<TaskGenerator> taskGenerator;
    std::unordered_map<std::string, json> scripts;
    json variables;
    std::unordered_map<std::string, std::function<json(const json&)>> functions;
    std::unordered_map<std::string, size_t>
        labels;  // Label to script index mapping
    std::unordered_map<std::string, std::function<void(const std::exception&)>>
        exceptionHandlers;
    std::atomic<bool> stopRequested{false};
    std::thread executionThread;
};
}  // namespace lithium

#endif