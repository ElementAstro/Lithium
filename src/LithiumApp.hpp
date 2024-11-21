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

#include <filesystem>
#include <memory>

#include "atom/async/message_bus.hpp"
#include "atom/components/component.hpp"
#include "atom/type/json_fwd.hpp"

#include "atom/macro.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

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

struct Program;
class Interpreter;

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
    // Config methods
    // -------------------------------------------------------------------

    ATOM_NODISCARD auto getValue(const std::string& key_path) const
        -> std::optional<nlohmann::json>;
    auto setValue(const std::string& key_path,
                  const nlohmann::json& value) -> bool;

    auto appendValue(const std::string& key_path, const nlohmann::json& value) -> bool;
    auto deleteValue(const std::string& key_path) -> bool;
    ATOM_NODISCARD auto hasValue(const std::string& key_path) const -> bool;
    auto loadFromFile(const fs::path& path) -> bool;
    auto loadFromDir(const fs::path& dir_path, bool recursive = false) -> bool;
    ATOM_NODISCARD auto saveToFile(const fs::path& file_path) const -> bool;
    [[nodiscard]] auto getKeys() const -> std::vector<std::string>;
    [[nodiscard]] auto listPaths() const -> std::vector<std::string>;
    void tidyConfig();
    void clearConfig();
    void mergeConfig(const nlohmann::json& src);

    // -------------------------------------------------------------------
    // Task methods
    // -------------------------------------------------------------------

    void loadScript(const std::string& filename);

    void interpretScript(const std::string& filename);

    void interpret(const std::shared_ptr<Program>& ast);

private:
    std::weak_ptr<TaskPool> m_taskpool_;
    std::weak_ptr<atom::async::MessageBus> m_messagebus_;
    std::weak_ptr<atom::error::ErrorStack> m_errorstack_;
    std::weak_ptr<ComponentManager> m_component_manager_;
    std::weak_ptr<Interpreter> m_task_interpreter_;
    std::weak_ptr<ConfigManager> m_config_manager_;

    std::weak_ptr<PyScriptManager> m_py_script_manager_;
};
extern std::shared_ptr<LithiumApp> myApp;

void initLithiumApp(int argc, char** argv);
}  // namespace lithium
