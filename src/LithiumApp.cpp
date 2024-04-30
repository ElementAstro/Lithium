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
#include "addon/loader.hpp"
#include "addon/manager.hpp"

#include "config/configor.hpp"

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

namespace lithium {
std::shared_ptr<LithiumApp> MyApp = nullptr;

LithiumApp::LithiumApp() {
    DLOG_F(INFO, "LithiumApp Constructor");
    try {
        m_MessageBus = GetWeakPtr<atom::server::MessageBus>("lithium.bus");
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
        DLOG_F(INFO, "Start Message Processing Thread");
        m_MessageBus.lock()->StartProcessingThread<Message>();

        DLOG_F(INFO, "Register LithiumApp Member Functions");
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to load Lithium App , error : {}", e.what());
        throw std::runtime_error("Failed to load Lithium App");
    }
    DLOG_F(INFO, "Lithium App Initialized");
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
    // Message Bus
    AddPtr("lithium.bus", atom::server::MessageBus::createShared());
    // AddPtr("ModuleLoader", ModuleLoader::createShared());
    // AddPtr("lithium.async.thread",
    // Atom::Async::ThreadManager::createShared(GetIntConfig("config/server/maxthread")));
    // AddPtr("PluginManager",
    // PluginManager::createShared(GetPtr<Process::ProcessManager>("ProcessManager")));
    // AddPtr("TaskManager", std::make_shared<Task::TaskManager>("tasks.json"));
    // AddPtr("TaskGenerator",
    // std::make_shared<Task::TaskGenerator>(GetPtr<DeviceManager>("DeviceManager")));
    // AddPtr("TaskStack", std::make_shared<Task::TaskStack>());
    // AddPtr("ScriptManager",
    // ScriptManager::createShared(GetPtr<MessageBus>("MessageBus")));

    AddPtr("lithium.error.stack", std::make_shared<atom::error::ErrorStack>());

    AddPtr("lithium.task.container", TaskContainer::createShared());
    AddPtr("lithiun.task.generator", TaskGenerator::createShared());
    AddPtr("lithium.task.loader", TaskLoader::createShared());
    AddPtr("lithium.task.pool",
           TaskPool::createShared(std::thread::hardware_concurrency()));
    AddPtr("lithium.task.tick",
           TickScheduler::createShared(std::thread::hardware_concurrency()));
    AddPtr("lithium.task.manager", TaskManager::createShared());

    AddPtr("lithium.utils.env", atom::utils::Env::createShared(argc, argv));

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
}  // namespace lithium