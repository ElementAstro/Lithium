/*
 * LithiumApp.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#include "LithiumApp.hpp"

#include "components/component.hpp"
#include "config.h"

#include "addon/addons.hpp"
#include "addon/loader.hpp"
#include "addon/manager.hpp"

#include "config/configor.hpp"

#include "device/manager.hpp"

#include "task/container.hpp"
#include "task/generator.hpp"
#include "task/loader.hpp"
#include "task/manager.hpp"

#include "script/manager.hpp"

#include "atom/components/dispatch.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"
#include "atom/system/process.hpp"
#include "atom/utils/time.hpp"

#include "utils/constant.hpp"
#include "utils/marco.hpp"

using json = nlohmann::json;

#if __cplusplus >= 202003L
#include <format>
#else
#include <fmt/format.h>
#endif

#define CHECK_PARAM(key)                                                    \
    if (!params.contains(key)) {                                            \
        LOG_F(ERROR, "Failed to execute {}: Invalid Parameters", __func__); \
        return {{"command", __func__},                                      \
                {"error", "Invalid Parameters"},                            \
                {"status", "error"},                                        \
                {"code", 1000},                                             \
                {"message", std::format("Invalid Parameters, {} need {}",   \
                                        __func__, key)}};                   \
    }

#define CHECK_PARAMS(...)                                                     \
    {                                                                         \
        std::vector<std::string> __params = {__VA_ARGS__};                    \
        for (const auto &key : __params) {                                    \
            if (!params.contains(key)) {                                      \
                LOG_F(ERROR, "Failed to execute {}: Invalid Parameters",      \
                      __func__);                                              \
                return {                                                      \
                    {"command", __func__},                                    \
                    {"error", "Invalid Parameters"},                          \
                    {"status", "error"},                                      \
                    {"code", 1000},                                           \
                    {"message", std::format("Invalid Parameters, {} need {}", \
                                            __func__, key)}};                 \
            }                                                                 \
        }                                                                     \
    }

#define INIT_FUNC() DLOG_F(INFO, "Call {} with: {}", __func__, params.dump());

#define GET_VALUE_D(type, key, defaultValue) \
    type key =                               \
        (params.contains(#key) ? params[#key].get<type>() : (defaultValue))

namespace lithium {
std::shared_ptr<LithiumApp> myApp = nullptr;

LithiumApp::LithiumApp() : Component("lithium.main") {
    DLOG_F(INFO, "LithiumApp Constructor");
    try {
        m_messagebus_ = GetWeakPtr<atom::async::MessageBus>("lithium.bus");
        CHECK_WEAK_PTR_EXPIRED(m_messagebus_,
                               "load message bus from gpm: lithium.bus");

        m_task_interpreter_ =
            GetWeakPtr<TaskInterpreter>("lithium.task.manager");
        CHECK_WEAK_PTR_EXPIRED(
            m_task_interpreter_,
            "load task manager from gpm: lithium.task.manager");

        // Common Message Processing Threads
        // Max : Maybe we only need one thread for Message, and dynamically cast
        // message
        //       to the right type to process.
        //       All of the messages are based on the Message class.
        DLOG_F(INFO, "Start Message Processing Thread");
        // m_messagebus_.lock()->StartProcessingThread<Message>();

        DLOG_F(INFO, "Register LithiumApp Member Functions");
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to load Lithium App , error : {}", e.what());
        THROW_OBJ_UNINITIALIZED("Failed to load Lithium App");
    }

    doc("Lithium App");
    def("load_component", &LithiumApp::loadComponent, "addon",
        "Load a component");
    def("unload_component", &LithiumApp::unloadComponent, "addon",
        "Unload a component");
    def("reload_component", &LithiumApp::reloadComponent, "addon",
        "Reload a component");
    def("reload_all_components", &LithiumApp::reloadAllComponents,
        "Reload all components");
    def("unload_all_components", &LithiumApp::unloadAllComponents,
        "Unload all components");
    def("get_component_info", &LithiumApp::getComponentInfo, "addon",
        "Get info about a component");
    def("get_component_list", &LithiumApp::getComponentList,
        "Get a list of all components");

    def("load_script", &LithiumApp::loadScript, "script", "Load a script");
    def("unload_script", &LithiumApp::unloadScript, "script",
        "Unload a script");
    def("has_script", &LithiumApp::hasScript, "script",
        "Check if a script is ");
    def("get_script", &LithiumApp::getScript, "script", "Get a script");

    DLOG_F(INFO, "Lithium App Initialized");
}

LithiumApp::~LithiumApp() {
}

auto LithiumApp::createShared() -> std::shared_ptr<LithiumApp> {
    return std::make_shared<LithiumApp>();
}

auto LithiumApp::initialize() -> bool { return true; }

auto LithiumApp::destroy() -> bool { return true; }

auto LithiumApp::getComponentManager() -> std::weak_ptr<ComponentManager> {
    return m_component_manager_;
}

auto LithiumApp::loadComponent(const std::string &name) -> bool {
    return m_component_manager_.lock()->loadComponent(name);
}

auto LithiumApp::unloadComponent(const std::string &name) -> bool {
    return m_component_manager_.lock()->unloadComponent(name);
}

auto LithiumApp::unloadAllComponents() -> bool {
    for (const auto &componentName :
         m_component_manager_.lock()->getComponentList()) {
        if (!unloadComponent(componentName)) {
            return false;
        }
    }
    return true;
}

auto LithiumApp::reloadComponent(const std::string &name) -> bool {
    return m_component_manager_.lock()->reloadComponent(name);
}

auto LithiumApp::reloadAllComponents() -> bool {
    return m_component_manager_.lock()->reloadAllComponents();
}
auto LithiumApp::getComponent(const std::string &name)
    -> std::weak_ptr<Component> {
    return m_component_manager_.lock()->getComponent(name).value();
}

auto LithiumApp::getComponentInfo(const std::string &name) -> json {
    return m_component_manager_.lock()->getComponentInfo(name).value();
}

auto LithiumApp::getComponentList() -> std::vector<std::string> {
    return m_component_manager_.lock()->getComponentList();
}

void LithiumApp::loadScript(const std::string &name, const json &script) {
    m_task_interpreter_.lock()->loadScript(name, script);
}

void LithiumApp::unloadScript(const std::string &name) {
    m_task_interpreter_.lock()->unloadScript(name);
}

auto LithiumApp::hasScript(const std::string &name) const -> bool {
    return m_task_interpreter_.lock()->hasScript(name);
}

auto LithiumApp::getScript(const std::string &name) const
    -> std::optional<json> {
    return m_task_interpreter_.lock()->getScript(name);
}

void LithiumApp::registerFunction(const std::string &name,
                                  std::function<json(const json &)> func) {
    m_task_interpreter_.lock()->registerFunction(name, func);
}

void LithiumApp::registerExceptionHandler(
    const std::string &name,
    std::function<void(const std::exception &)> handler) {
    m_task_interpreter_.lock()->registerExceptionHandler(name, handler);
}

void LithiumApp::setVariable(const std::string &name, const json &value) {
    m_task_interpreter_.lock()->setVariable(name, value, determineType(value));
}

auto LithiumApp::getVariable(const std::string &name) -> json {
    return m_task_interpreter_.lock()->getVariable(name);
}

void LithiumApp::parseLabels(const json &script) {
    m_task_interpreter_.lock()->parseLabels(script);
}

void LithiumApp::execute(const std::string &scriptName) {
    m_task_interpreter_.lock()->execute(scriptName);
}

void LithiumApp::stop() { m_task_interpreter_.lock()->stop(); }

void LithiumApp::pause() { m_task_interpreter_.lock()->pause(); }

void LithiumApp::resume() { m_task_interpreter_.lock()->resume(); }

void LithiumApp::queueEvent(const std::string &eventName,
                            const json &eventData) {
    m_task_interpreter_.lock()->queueEvent(eventName, eventData);
}

auto LithiumApp::getValue(const std::string &key_path) const
    -> std::optional<nlohmann::json> {}
auto LithiumApp::setValue(const std::string &key_path,
                          const nlohmann::json &value) -> bool {}

auto LithiumApp::appendValue(const std::string &key_path,
                             const nlohmann::json &value) -> bool {}
auto LithiumApp::deleteValue(const std::string &key_path) -> bool {}
auto LithiumApp::hasValue(const std::string &key_path) const -> bool {}
auto LithiumApp::loadFromFile(const fs::path &path) -> bool {}
auto LithiumApp::loadFromDir(const fs::path &dir_path,
                             bool recursive) -> bool {}
auto LithiumApp::saveToFile(const fs::path &file_path) const -> bool {}
void LithiumApp::tidyConfig() {}
void LithiumApp::clearConfig() {}
void LithiumApp::mergeConfig(const nlohmann::json &src) {}

void initLithiumApp(int argc, char **argv) {
    LOG_F(INFO, "Init Lithium App");
    // Message Bus
    AddPtr("lithium.bus", atom::async::MessageBus::createShared());
    // AddPtr("ModuleLoader", ModuleLoader::createShared());
    // AddPtr("lithium.async.thread",
    // Atom::Async::ThreadManager::createShared(GetIntConfig("config/server/maxthread")));
    // AddPtr("PluginManager",
    // PluginManager::createShared(GetPtr<Process::ProcessManager>("ProcessManager")));
    // AddPtr("TaskInterpreter",
    // std::make_shared<Task::TaskInterpreter>("tasks.json"));
    // AddPtr("TaskGenerator",
    // std::make_shared<Task::TaskGenerator>(GetPtr<DeviceManager>("DeviceManager")));
    // AddPtr("TaskStack", std::make_shared<Task::TaskStack>());
    // AddPtr("ScriptManager",
    // ScriptManager::createShared(GetPtr<MessageBus>("MessageBus")));

    AddPtr(Constants::LITHIUM_DEVICE_MANAGER, DeviceManager::createShared());
    AddPtr(Constants::LITHIUM_DEVICE_LOADER, ModuleLoader::createShared("./drivers"));

    AddPtr("lithium.error.stack", std::make_shared<atom::error::ErrorStack>());

    AddPtr("lithium.task.container", TaskContainer::createShared());
    AddPtr("lithiun.task.generator", TaskGenerator::createShared());
    AddPtr("lithium.task.loader", TaskLoader::createShared());
    AddPtr("lithium.task.manager", TaskInterpreter::createShared());

    AddPtr("lithium.utils.env", atom::utils::Env::createShared(argc, argv));

    // TODO: Addons path need to be configurable
    AddPtr("lithium.addon.loader", ModuleLoader::createShared("./modules"));
    AddPtr("lithium.addon.addon", AddonManager::createShared());
    AddPtr("lithium.addon.manager", ComponentManager::createShared());
}

auto createSuccessResponse(const std::string &command,
                           const json &value) -> json {
    json res;
    res["command"] = command;
    res["value"] = value;
    res["status"] = "ok";
    res["code"] = 200;
#if __cplusplus >= 202003L
    res["message"] = std::format("{} operated on success", command);
#else
    res["message"] = fmt::format("{} operated on success", command);
#endif
    return res;
}

auto createErrorResponse(const std::string &command, const json &error,
                         const std::string &message = "") -> json {
    json res;
    res["command"] = command;
    res["status"] = "error";
    res["code"] = 500;
#if __cplusplus >= 202003L
    res["message"] =
        std::format("{} operated on failure, message: {}", command, message);
#else
    res["message"] =
        std::format("{} operated on failure, message: {}", command, message);
#endif
    if (!error.empty()) {
        res["error"] = error;
    } else {
        res["error"] = "Unknown Error";
    }
    return res;
}

json createWarningResponse(const std::string &command, const json &warning,
                           const std::string &message = "") {
    json res;
    res["command"] = command;
    res["status"] = "warning";
    res["code"] = 400;
#if __cplusplus >= 202003L
    res["message"] =
        std::format("{} operated on warning, message: {}", command, message);
#else
    res["message"] =
        std::format("{} operated on warning, message: {}", command, message);
#endif
    if (!warning.empty()) {
        res["warning"] = warning;
    } else {
        res["warning"] = "Unknown Warning";
    }
    return res;
}
}  // namespace lithium
