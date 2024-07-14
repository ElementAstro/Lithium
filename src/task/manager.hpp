#ifndef LITHIUM_TASK_INTERPRETER_HPP
#define LITHIUM_TASK_INTERPRETER_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {

class TaskInterpreterImpl;

class TaskInterpreter {
public:
    TaskInterpreter();
    ~TaskInterpreter();

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

    void setVariable(const std::string& name, const json& value);
    auto getVariable(const std::string& name) -> json;

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
    void executeCondition(const json& step, size_t& idx, const json& script);
    auto executeLoop(const json& step, size_t& idx, const json& script) -> bool;
    void executeGoto(const json& step, size_t& idx, const json& script);
    void executeSwitch(const json& step, size_t& idx, const json& script);
    void executeDelay(const json& step);
    void executeParallel(const json& step, size_t& idx, const json& script);
    void executeNestedScript(const json& step);
    void executeAssign(const json& step);
    void executeImport(const json& step);
    void executeWaitEvent(const json& step);
    void executePrint(const json& step);
    void executeAsync(const json& step);
    void executeSteps(const json& steps, size_t& idx, const json& script);

    void executeTryCatch(const json& step, size_t& idx, const json& script);
    void executeFunction(const json& step);
    void executeReturn(const json& step, size_t& idx);
    void executeBreak(const json& step, size_t& idx);
    void executeContinue(const json& step, size_t& idx);

    auto evaluate(const json& value) -> json;
    void handleException(const std::string& scriptName,
                         const std::exception& e);

    std::unique_ptr<TaskInterpreterImpl> impl_;
};

}  // namespace lithium

#endif
