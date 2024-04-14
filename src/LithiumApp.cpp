/*
 * LithiumApp.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#include "lithiumapp.hpp"

#include "config.h"

#include "addon/addons.hpp"
#include "addon/manager.hpp"

#include "config/configor.hpp"

#include "device/manager.hpp"
#include "device/server/ascom.hpp"
#include "device/server/hydrogen.hpp"

#include "task/manager.hpp"

#include "script/manager.hpp"

#include "atom/error/error_stack.hpp"
#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "atom/system/process.hpp"
#include "atom/utils/time.hpp"
#include "utils/marco.hpp"

#include "magic_enum/magic_enum.hpp"

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
    type key = (params.contains(#key) ? params[#key].get<type>() : defaultValue)

namespace Lithium {
std::shared_ptr<LithiumApp> MyApp = nullptr;

LithiumApp::LithiumApp() {
    DLOG_SCOPE_FUNCTION(INFO);
    DLOG_F(INFO, "LithiumApp Constructor");
    try {
        // Specialized Managers and Threads
        m_ConfigManager = GetWeakPtr<ConfigManager>("lithium.config");
        CHECK_WEAK_PTR_EXPIRED(m_ConfigManager,
                               "load config manager from gpm: lithium.config");
        m_DeviceManager = GetWeakPtr<DeviceManager>("lithium.device");
        CHECK_WEAK_PTR_EXPIRED(m_DeviceManager,
                               "load device manager from gpm: lithium.device");
        m_ProcessManager =
            GetWeakPtr<Atom::System::ProcessManager>("lithium.system.process");
        CHECK_WEAK_PTR_EXPIRED(
            m_ProcessManager,
            "load process manager from gpm: lithium.system.process");
        m_MessageBus = GetWeakPtr<Atom::Server::MessageBus>("lithium.bus");
        CHECK_WEAK_PTR_EXPIRED(m_MessageBus,
                               "load message bus from gpm: lithium.bus");

        m_TaskManager = GetWeakPtr<TaskManager>("lithium.task.manager");
        CHECK_WEAK_PTR_EXPIRED(
            m_TaskManager, "load task manager from gpm: lithium.task.manager");

        // Common Message Processing Threads
        // Max : Maybe we only need one thread for Message, and dynamically cast
        // message
        //       to the right type to process.
        //       All of the messages are based on the Message class.
        DLOG_SCOPE_F(INFO, "Start Message Processing Thread");
        m_MessageBus.lock()->StartProcessingThread<Message>();

        DLOG_SCOPE_F(INFO, "Register LithiumApp Member Functions");

        LiRegisterMemberFunc("GetConfig", &LithiumApp::GetConfig);
        LiRegisterMemberFunc("SetConfig", &LithiumApp::SetConfig);
        LiRegisterMemberFunc("DeleteConfig", &LithiumApp::DeleteConfig);
        LiRegisterMemberFunc("SaveConfig", &LithiumApp::SaveConfig);
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to load Lithium App , error : {}", e.what());
        throw std::runtime_error("Failed to load Lithium App");
    }
    DLOG_SCOPE_F(INFO, "Lithium App Initialized");
}

LithiumApp::~LithiumApp() {
    m_MessageBus.lock()->UnsubscribeAll();
    m_MessageBus.lock()->StopAllProcessingThreads();
}

std::shared_ptr<LithiumApp> LithiumApp::createShared() {
    return std::make_shared<LithiumApp>();
}

void InitLithiumApp(int argc, char **argv) {
    LOG_F(INFO, "Init Lithium App");
    // Config
    AddPtr("lithium.config", ConfigManager::createShared());
    // Message Bus
    AddPtr("lithium.bus", Atom::Server::MessageBus::createShared());
    // AddPtr("ModuleLoader", ModuleLoader::createShared());
    // AddPtr("lithium.async.thread",
    // Atom::Async::ThreadManager::createShared(GetIntConfig("config/server/maxthread")));
    AddPtr("lithium.system.process",
           Atom::System::ProcessManager::createShared(
               GetIntConfig("config/server/maxprocess")));
    // AddPtr("PluginManager",
    // PluginManager::createShared(GetPtr<Process::ProcessManager>("ProcessManager")));
    // AddPtr("TaskManager", std::make_shared<Task::TaskManager>("tasks.json"));
    // AddPtr("TaskGenerator",
    // std::make_shared<Task::TaskGenerator>(GetPtr<DeviceManager>("DeviceManager")));
    // AddPtr("TaskStack", std::make_shared<Task::TaskStack>());
    // AddPtr("ScriptManager",
    // ScriptManager::createShared(GetPtr<MessageBus>("MessageBus")));
    AddPtr("lithium.device",
           DeviceManager::createShared(
               GetPtr<Atom::Server::MessageBus>("lithium.bus").value(),
               GetPtr<ConfigManager>("lithium.config").value()));
    AddPtr("lithium.device.hydrogen", HydrogenManager::createShared());

    AddPtr("lithium.error.stack", std::make_shared<Atom::Error::ErrorStack>());

    AddPtr("lithium.task.container", TaskContainer::createShared());
    AddPtr("lithiun.task.generator", TaskGenerator::createShared());
    AddPtr("lithium.task.loader", TaskLoader::createShared());
    AddPtr("lithium.task.pool",
           TaskPool::createShared(std::thread::hardware_concurrency()));
    AddPtr("lithium.task.tick",
           TickScheduler::createShared(std::thread::hardware_concurrency()));
    AddPtr("lithium.task.manager", TaskManager::createShared());

    AddPtr("lithium.cmd.global", CommandDispatcher<json, json>::createShared());

    AddPtr("lithium.utils.env", Atom::Utils::Env::createShared(argc, argv));

    // TODO: Addons path need to be configurable
    AddPtr("lithium.addon.loader", ModuleLoader::createShared("./modules"));
    AddPtr("lithium.addon.addon", AddonManager::createShared());
    AddPtr("lithium.addon.manager", ComponentManager::createShared());
}

json createSuccessResponse(const std::string &command, const json &value) {
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

json createErrorResponse(const std::string &command, const json &error,
                         const std::string &message = "") {
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

// ----------------------------------------------------------------
// Config
// ----------------------------------------------------------------

json LithiumApp::GetConfig(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("key");
    std::string key_path = params["key"].get<std::string>();
    json res;
    if (auto value = m_ConfigManager.lock()->getValue(key_path);
        value.has_value()) {
        return createSuccessResponse(__func__, value.value());
    }
    return createErrorResponse(__func__, json(),
                               std::format("Key {} not found", key_path));
}

json LithiumApp::SetConfig(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("key");
    CHECK_PARAM("value");
    std::string key_path = params["key"].get<std::string>();
    json value = params["value"];
    if (value.is_null()) {
        return createErrorResponse(__func__, json(), "Value is null");
    }
    if (m_ConfigManager.lock()->setValue(key_path, value)) {
        return createSuccessResponse(__func__, value);
    }
    return createErrorResponse(
        __func__, json(),
        std::format("Failed to set value for key {}", key_path));
}

json LithiumApp::DeleteConfig(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("key");
    std::string key_path = params["key"].get<std::string>();
    if (m_ConfigManager.lock()->deleteValue(key_path)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(
        __func__, json(), std::format("Failed to delete key {}", key_path));
}

json LithiumApp::SaveConfig(const json &params) {
    INIT_FUNC();
    GET_VALUE_D(std::string, path, "config/config.json");
    if (m_ConfigManager.lock()->saveToFile(path)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(__func__, json(), "Failed to save config");
}

// -----------------------------------------------------------------
// Device
// -----------------------------------------------------------------

json LithiumApp::getDeviceList(const json &params) {
    INIT_FUNC();
    GET_VALUE_D(std::string, type, "all");
    json device_list;
    if (type == "all") {
        for (const auto &device : m_DeviceManager.lock()->getDeviceList()) {
            device_list.push_back(device);
        }
    } else {
        DeviceType d_type = StringToDeviceType(type);
        if (d_type == DeviceType::NumDeviceTypes) {
            return createErrorResponse(
                __func__, json(), std::format("Unknown device type {}", type));
        }
        for (const auto &device :
             m_DeviceManager.lock()->getDeviceListByType(d_type)) {
            device_list.push_back(device);
        }
        if (device_list.empty()) {
            return createErrorResponse(
                __func__, json(),
                std::format("No device found for type {}", type));
        }
    }
    return createSuccessResponse(__func__, device_list);
}

json LithiumApp::addDevice(const json &params) {
    INIT_FUNC();
    CHECK_PARAMS("type", "name", "lib_name");
    std::string type = params["type"].get<std::string>();
    std::string name = params["name"].get<std::string>();
    std::string lib_name = params["lib_name"].get<std::string>();
    DeviceType d_type = StringToDeviceType(type);
    if (d_type == DeviceType::NumDeviceTypes) {
        return createErrorResponse(__func__, json(),
                                   std::format("Unknown device type {}", type));
    }
    if (m_DeviceManager.lock()->findDevice(d_type, name) != 0) {
        return createErrorResponse(
            __func__, json(), std::format("Device {} already exists", name));
    }
    if (m_DeviceManager.lock()->addDevice(d_type, name, lib_name)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(__func__, json(),
                               std::format("Failed to add device {}", name));
}

json LithiumApp::addDeviceLibrary(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("lib_path");
    CHECK_PARAM("lib_name");
    std::string lib_path = params["lib_path"].get<std::string>();
    std::string lib_name = params["lib_name"].get<std::string>();
    if (m_DeviceManager.lock()->addDeviceLibrary(lib_path, lib_name)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(
        __func__, json(),
        std::format("Failed to add device library {}", lib_name));
}

json LithiumApp::removeDevice(const json &params) {
    INIT_FUNC();
    CHECK_PARAMS("type", "name");
    std::string type = params["type"].get<std::string>();
    std::string name = params["name"].get<std::string>();
    DeviceType d_type = StringToDeviceType(type);
    if (d_type == DeviceType::NumDeviceTypes) {
        return createErrorResponse(__func__, json(),
                                   std::format("Unknown device type {}", type));
    }
    if (m_DeviceManager.lock()->removeDevice(d_type, name)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(__func__, json(),
                               std::format("Failed to remove device {}", name));
}

json LithiumApp::removeDeviceByName(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("name");
    std::string name = params["name"].get<std::string>();
    if (m_DeviceManager.lock()->removeDeviceByName(name)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(__func__, json(),
                               std::format("Failed to remove device {}", name));
}

json LithiumApp::removeDeviceLibrary(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("lib_name");
    std::string lib_name = params["lib_name"].get<std::string>();
    if (m_DeviceManager.lock()->removeDeviceLibrary(lib_name)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(
        __func__, json(),
        std::format("Failed to remove device library {}", lib_name));
}

/*
    void LithiumApp::addDeviceObserver(DeviceType type, const std::string &name)
    {
        m_DeviceManager.lock()->addDeviceObserver(type, name);
    }

    bool LithiumApp::removeDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager.lock()->removeDevice(type, name);
    }

    bool LithiumApp::removeDeviceByName(const std::string &name)
    {
        return m_DeviceManager.lock()->removeDeviceByName(name);
    }

    bool LithiumApp::removeDeviceLibrary(const std::string &lib_name)
    {
        return m_DeviceManager.lock()->removeDeviceLibrary(lib_name);
    }

    std::shared_ptr<Device> LithiumApp::getDevice(DeviceType type, const
   std::string &name)
    {
        return m_DeviceManager.lock()->getDevice(type, name);
    }

    size_t LithiumApp::findDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager.lock()->findDevice(type, name);
    }

    std::shared_ptr<Device> LithiumApp::findDeviceByName(const std::string
   &name) const
    {
        return m_DeviceManager.lock()->findDeviceByName(name);
    }

    std::shared_ptr<SimpleTask> LithiumApp::getTask(DeviceType type, const
   std::string &device_name, const std::string &task_name, const json &params)
    {
        return m_DeviceManager.lock()->getTask(type, device_name, task_name,
   params);
    }

    bool LithiumApp::getProperty(const std::string &name, const std::string
   &property_name)
    {
        m_DeviceManager.lock()->findDeviceByName(name)->getStringProperty(property_name);
        return true;
    }

    bool LithiumApp::setProperty(const std::string &name, const std::string
   &property_name, const std::string &property_value)
    {
        return true;
    }
*/

// ------------------------------------------------------------------
// Process
// ------------------------------------------------------------------

json LithiumApp::createProcess(const json &params) {
    INIT_FUNC();
    CHECK_PARAMS("command", "identifier");
    std::string command = params["command"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    if (m_ProcessManager.lock()->hasProcess(identifier)) {
        return createErrorResponse(
            __func__, json(),
            std::format("Process {} already exists", identifier));
    }
    if (m_ProcessManager.lock()->createProcess(command, identifier)) {
        return createSuccessResponse(__func__, json());
    } else {
        return createErrorResponse(
            __func__, json(),
            std::format("Failed to create process {}", identifier));
    }
}

json LithiumApp::runScript(const json &params) {
    INIT_FUNC();
    CHECK_PARAMS("script", "identifier");
    std::string script = params["script"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    if (m_ProcessManager.lock()->runScript(script, identifier)) {
        return createSuccessResponse(__func__, json());
    } else {
        return createErrorResponse(
            __func__, json(),
            std::format("Failed to run script {}", identifier));
    }
}

json LithiumApp::terminateProcess(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("pid");
    int pid = params["pid"].get<int>();
    int signal = params.value("signal", 15);
    if (m_ProcessManager.lock()->terminateProcess(pid, signal)) {
        return createSuccessResponse(__func__, json());
    } else {
        return createErrorResponse(
            __func__, json(),
            std::format("Failed to terminate process {}", pid));
    }
}

json LithiumApp::terminateProcessByName(const json &params) {
    INIT_FUNC();
    CHECK_PARAMS("name");
    std::string name = params["name"].get<std::string>();
    int signal = params.value("signal", 15);
    if (m_ProcessManager.lock()->terminateProcessByName(name, signal)) {
        return createSuccessResponse(__func__, json());
    }
    return createErrorResponse(
        __func__, json(), std::format("Failed to terminate process {}", name));
}

json LithiumApp::getRunningProcesses(const json &params) {
    json running_process;
    for (auto &process : m_ProcessManager.lock()->getRunningProcesses()) {
        running_process["process"].push_back({{"pid", process.pid},
                                              {"name", process.name},
                                              {"status", process.status},
                                              {"output", process.output}});
    }
    return createSuccessResponse(__func__, running_process["process"]);
}

json LithiumApp::getProcessOutput(const json &params) {
    INIT_FUNC();
    CHECK_PARAM("identifier");
    std::string identifier = params["identifier"].get<std::string>();
    if (!m_ProcessManager.lock()->hasProcess(identifier)) {
        return createErrorResponse(
            __func__, json(),
            std::format("Process {} does not exist", identifier));
    }
    json output;
    for (auto &line : m_ProcessManager.lock()->getProcessOutput(identifier)) {
        output.push_back(line);
    }
    return createSuccessResponse(__func__, output);
}

json LithiumApp::DispatchCommand(const std::string &name, const json &params) {
    if (name.empty()) {
        DLOG_F(ERROR, "Invalid command name or params");
        return json();
    }
    if (m_CommandDispatcher) {
        if (m_CommandDispatcher->hasHandler(name)) {
            DLOG_F(INFO, "Dispatching command {}", name);
#if ENABLE_DEBUG
            json res = m_CommandDispatcher->Dispatch(name, params);
            DLOG_F(INFO, _("Dispatched command {} with result: {}"), name,
                   res.dump());
            return res;
#else
            return m_CommandDispatcher->dispatch(name, params);
#endif
        } else {
            LOG_F(ERROR, "Command {} not found", name);
            return json();
        }
    }
    LOG_F(ERROR, "Command dispatcher not found");
    return json();
}

bool LithiumApp::hasCommand(const std::string &name) {
    if (m_CommandDispatcher) {
        return m_CommandDispatcher->hasHandler(name);
    } else {
        return false;
    }
}
}  // namespace Lithium