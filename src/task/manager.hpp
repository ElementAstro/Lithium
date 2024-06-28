#ifndef LITHIUM_TASK_INTERPRETOR_HPP
#define LITHIUM_TASK_INTERPRETOR_HPP

#include <atomic>
#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

#include "task.hpp"
#include "atom/type/json.hpp"

class Component; // Forward declaration

using json = nlohmann::json;

namespace lithium {
class TaskGenerator;

class TaskInterpretor {
public:
    TaskInterpretor();

    ~TaskInterpretor();

    void loadScript(const std::string& name, const json& script);

    void unloadScript(const std::string& name);

    auto hasScript(const std::string& name) const -> bool;

    auto getScript(const std::string& name) const -> std::optional<json>;

    void registerFunction(const std::string& name,
                          std::function<json(const json&)> func);

    auto hasFunction(const std::string& name) const -> bool;

    void registerExceptionHandler(
        const std::string& name,
        std::function<void(const std::exception&)> handler);

    void setVariable(const std::string& name, const json& value);

    auto getVariable(const std::string& name) -> json;

    void parseLabels(const json& script);

    void execute(const std::string& scriptName);

    void stop();

private:
    auto prepareScript(json& script) -> bool;

    auto executeStep(const json& step, size_t& idx, const json& script) -> bool;

    void executeCall(const json& step);

    void executeCondition(const json& step, size_t& idx, const json& script);

    auto executeLoop(const json& step, size_t& idx, const json& script) -> bool;

    void executeGoto(const json& step, size_t& idx,
                     [[maybe_unused]] const json& script);

    void executeSwitch(const json& step, size_t& idx, const json& script);

    void executeDelay(const json& step);

    void executeParallel(const json& step, size_t& idx, const json& script);

    auto evaluate(const json& value) -> json;

    void handleException(const std::string& scriptName,
                         const std::exception& e);

    std::shared_ptr<TaskGenerator> taskGenerator;
    std::weak_ptr<Component> component;
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