/*
 * lithiumapp.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#pragma once

#define LITHIUM_APP_MAIN

#include <memory>

#include "atom/async/message_bus.hpp"
#include "atom/components/component.hpp"
#include "atom/type/json.hpp"
#include "atom/type/message.hpp"
using json = nlohmann::json;

// -------------------------------------------------------------------
// About the LithiumApp
// This is the main class of the Lithium App. All of the functions can be
// executed here. NOTE: No wrapper functions needed, just use the functions
// directly.
//       A json object is used to pass parameters to the functions.
//       And the return value is a json object.
//       Sometimes I think it is unnecessary to use json object to pass
//       parameters. However, It is more convenient to use json object.
// -------------------------------------------------------------------

namespace atom {
namespace error {
class ErrorStack;
}

namespace system {
class ProcessManager;
}
}  // namespace atom
namespace lithium {
class PyScriptManager;  // FWD

class ComponentManager;  // FWD

class ConfigManager;

class TaskPool;

class TaskInterpreter;

class LithiumApp : public Component {
public:
    explicit LithiumApp();
    ~LithiumApp() override;

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    static auto createShared() -> std::shared_ptr<LithiumApp>;
    auto initialize() -> bool override;
    auto destroy() -> bool override;

    // -------------------------------------------------------------------
    // Component methods
    // -------------------------------------------------------------------

    auto getComponentManager() -> std::weak_ptr<ComponentManager>;
    auto loadComponent(const std::string& name) -> bool;
    auto unloadComponent(const std::string& name) -> bool;
    auto unloadAllComponents() -> bool;
    auto reloadComponent(const std::string& name) -> bool;
    auto reloadAllComponents() -> bool;
    auto getComponent(const std::string& name) -> std::weak_ptr<Component>;
    auto getComponentInfo(const std::string& name) -> json;
    auto getComponentList() -> std::vector<std::string>;

    // -------------------------------------------------------------------
    // Task methods
    // -------------------------------------------------------------------

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
    std::weak_ptr<TaskPool> m_taskpool_;
    std::weak_ptr<atom::async::MessageBus> m_messagebus_;
    std::weak_ptr<atom::error::ErrorStack> m_errorstack_;
    std::weak_ptr<ComponentManager> m_component_manager_;
    std::weak_ptr<TaskInterpreter> m_task_interpreter_;

    std::weak_ptr<PyScriptManager> m_py_script_manager_;
};
extern std::shared_ptr<LithiumApp> myApp;

void initLithiumApp(int argc, char** argv);
}  // namespace lithium
