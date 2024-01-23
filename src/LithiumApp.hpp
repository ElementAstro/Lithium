/*
 * LithiumApp.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#pragma once

#define LITHIUM_APP_MAIN

#include <memory>

#include "atom/async/thread.hpp"
#include "config/configor.hpp"
#include "device/device_manager.hpp"
#include "atom/system/process.hpp"
#include "task/task_manager.hpp"
#include "task/task_generator.hpp"
#include "task/task_stack.hpp"
#include "atom/type/message.hpp"
#include "script/script_manager.hpp"
#include "atom/error/error_stack.hpp"
#include "atom/server/message_bus.hpp"
#include "device/device_manager.hpp"
#include "atom/server/commander.hpp"
#include "atom/type/iparams.hpp"

namespace Lithium
{
    class LithiumApp
    {
    public:
        LithiumApp();
        ~LithiumApp();

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        static std::shared_ptr<LithiumApp> createShared();

        static std::unique_ptr<LithiumApp> createUnique();

        // -------------------------------------------------------------------
        // Config methods
        // -------------------------------------------------------------------

        json GetConfig(const std::string &key_path) const;
        void SetConfig(const std::string &key_path, const json &value);

        // -------------------------------------------------------------------
        // Wrappered Config methods
        // -------------------------------------------------------------------

        ReturnMessage GetConfigW(const json &params);
        ReturnMessage SetConfigW(const json &params);

        // -------------------------------------------------------------------
        // Device methods
        // -------------------------------------------------------------------

        std::vector<std::string> getDeviceList(DeviceType type);
        bool addDevice(DeviceType type, const std::string &name, const std::string &lib_name = "");
        bool addDeviceLibrary(const std::string &lib_path, const std::string &lib_name);
        void addDeviceObserver(DeviceType type, const std::string &name);
        bool removeDevice(DeviceType type, const std::string &name);
        bool removeDeviceByName(const std::string &name);
        bool removeDeviceLibrary(const std::string &lib_name);
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);
        size_t findDevice(DeviceType type, const std::string &name);
        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;
        std::shared_ptr<SimpleTask> getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const json &params);
        bool getProperty(const std::string &name, const std::string &property_name);
        bool setProperty(const std::string &name, const std::string &property_name, const std::string &property_value);

        // -------------------------------------------------------------------
        // Process methods
        // -------------------------------------------------------------------

        bool createProcess(const std::string &command, const std::string &identifier);
        bool runScript(const std::string &script, const std::string &identifier);
        bool terminateProcess(pid_t pid, int signal = 15);
        bool terminateProcessByName(const std::string &name, int signal = 15);
        std::vector<Process::Process> getRunningProcesses();
        std::vector<std::string> getProcessOutput(const std::string &identifier);

        // -------------------------------------------------------------------
        // Task methods
        // -------------------------------------------------------------------

        bool addTask(const std::shared_ptr<BasicTask> &task);
        bool insertTask(const std::shared_ptr<BasicTask> &task, int position);
        bool executeAllTasks();
        bool stopTask();
        bool executeTaskByName(const std::string &name);
        bool modifyTask(int index, const std::shared_ptr<BasicTask> &task);
        bool modifyTaskByName(const std::string &name, const std::shared_ptr<BasicTask> &task);
        bool deleteTask(int index);
        bool deleteTaskByName(const std::string &name);
        bool queryTaskByName(const std::string &name);
        const std::vector<std::shared_ptr<BasicTask>> &getTaskList() const;
        bool saveTasksToJson() const;

        bool checkTaskExecutable(const std::string &name);

        // -------------------------------------------------------------------
        // Module methods
        // -------------------------------------------------------------------

        bool loadModule(const std::string &path, const std::string &name);
        bool unloadModule(const std::string &name);
        bool reloadModule(const std::string &name);
        bool reloadAllModules();
        bool checkModuleLoaded(const std::string &name);
        bool enableModule(const std::string &name);
        bool disableModule(const std::string &name);
        bool getModuleStatus(const std::string &name);
        json getModuleConfig(const std::string &name);
        std::vector<std::string> getModuleList();
        
        // -------------------------------------------------------------------
        // Message methods
        // -------------------------------------------------------------------

        template <typename T>
        void MSSubscribe(const std::string &topic, std::function<void(const T &)> callback, int priority = 0)
        {
            m_MessageBus->Subscribe(topic, callback, priority);
        }

        template <typename T>
        void MSUnsubscribe(const std::string &topic, std::function<void(const T &)> callback)
        {
            m_MessageBus->Unsubscribe(topic, callback);
        }

        void sendStringMessage(const std::string &topic, const std::string &message)
        {
            m_MessageBus->Publish<std::string>(topic, message);
        }

        void sendJsonMessage(const std::string &topic, const json &message)
        {
            m_MessageBus->Publish<json>(topic, message);
        }

        ReturnMessage returnMessage(const std::string &message);

    public:
        void addThread(std::function<void()> func, const std::string &name);
        void joinAllThreads();
        void joinThreadByName(const std::string &name);
        bool isThreadRunning(const std::string &name);

    public:
        bool loadChaiScriptFile(const std::string &filename);
        bool unloadChaiScriptFile(const std::string &filename);
        bool runChaiCommand(const std::string &command);
        bool runChaiMultiCommand(const std::vector<std::string> &command);
        bool runChaiScript(const std::string &filename);
        void initMyAppChai();

        void LiRegisterFunc(const std::string &name, std::function<void(const json &)> handler)
        {
            m_CommandDispatcher->RegisterHandler(name, handler);
        }

        template <typename T>
        void LiRegisterMemberFunc(const std::string &name, void (T::*memberFunc)(const json &))
        {
            if (!m_CommandDispatcher)
                m_CommandDispatcher = std::make_unique<CommandDispatcher<void,json>>();
            m_CommandDispatcher->RegisterMemberHandler(name, this, memberFunc);
        }

        // Max: The async func will be executed in a separate thread, and the return value will be ignored.
        //      So must use MessageBus to send the return value.
        template <typename T>
        void LiRegisterAsyncMemberFunc(const std::string &name, void (T::*memberFunc)(const json &), bool async = false)
        {
            if (!m_CommandDispatcher)
                m_CommandDispatcher = std::make_unique<CommandDispatcher<void,json>();
            m_CommandDispatcher->RegisterMemberHandler(name + "_async", this, memberFunc);
        }

    private:

        std::unique_ptr<CommandDispatcher<void,json> m_CommandDispatcher;

    private:
        std::shared_ptr<Atom::Async::ThreadManager> m_ThreadManager;
        std::shared_ptr<ConfigManager> m_ConfigManager;
        std::shared_ptr<DeviceManager> m_DeviceManager;
        std::shared_ptr<Atom::System::ProcessManager> m_ProcessManager;
        std::shared_ptr<Task::TaskManager> m_TaskManager;
        std::shared_ptr<Task::TaskGenerator> m_TaskGenerator;
        std::shared_ptr<Task::TaskStack> m_TaskStack;
        std::shared_ptr<Atom::Server::MessageBus> m_MessageBus;
        std::shared_ptr<ScriptManager> m_ScriptManager;
        std::shared_ptr<Atom::ModuleLoader> m_ModuleLoader;
        std::shared_ptr<ErrorStack> m_ErrorStack;
    };
    extern std::shared_ptr<LithiumApp> MyApp;

    void InitLithiumApp();
} // namespace Lithium
