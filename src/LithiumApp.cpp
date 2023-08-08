/*
 * LithiumApp.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#include "LithiumApp.hpp"

#include "loguru/loguru.hpp"

namespace Lithium
{
    std::shared_ptr<LithiumApp> MyApp = nullptr;
    LithiumApp::LithiumApp()
    {
        LOG_F(INFO, "Loading Lithium App and preparing ...");
        try
        {
            m_ConfigManager = Config::ConfigManager::createShared();
            m_MessageBus = std::make_shared<MessageBus>();
            m_MessageQueue = std::make_shared<QueueWrapper>();
            m_DeviceManager = DeviceManager::createShared(m_MessageBus, m_ConfigManager);

            auto max_thread = GetConfig("config/server").value<int>("maxthread", 10);
            m_ThreadManager = Thread::ThreadManager::createShared(max_thread);

            auto max_process = GetConfig("config/server").value<int>("maxprocess", 10);

            m_ProcessManager = Process::ProcessManager::createShared(max_process);

            m_PluginManager = PluginManager::createShared(m_ProcessManager);
            m_TaskManager = std::make_shared<Task::TaskManager>("tasks.json");
            m_TaskGenerator = std::make_shared<TaskGenerator>(m_DeviceManager);

            m_cModuleLoader = ModuleLoader::createShared("controllers");

            m_MessageBus->StartProcessingThread<IMessage>();
            LOG_F(INFO, "Lithium App Loaded.");
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load Lithium App , error : %s", e.what());
            throw std::runtime_error("Failed to load Lithium App");
        }
    }

    LithiumApp::~LithiumApp()
    {
    }

    nlohmann::json LithiumApp::GetConfig(const std::string &key_path) const
    {
        LOG_F(INFO, "Get value : %s", key_path.c_str());
        return m_ConfigManager->getValue(key_path);
    }

    void LithiumApp::SetConfig(const std::string &key_path, const nlohmann::json &value)
    {
        LOG_F(INFO, "Set %s to %s", key_path.c_str(), value.dump().c_str());
        m_ConfigManager->setValue(key_path, value);
    }

    std::vector<std::string> LithiumApp::getDeviceList(DeviceType type)
    {
        return m_DeviceManager->getDeviceList(type);
    }

    bool LithiumApp::addDevice(DeviceType type, const std::string &name, const std::string &lib_name)
    {
        return m_DeviceManager->addDevice(type, name, lib_name);
    }

    bool LithiumApp::addDeviceLibrary(const std::string &lib_path, const std::string &lib_name)
    {
        return m_DeviceManager->addDeviceLibrary(lib_path, lib_name);
    }

    void LithiumApp::addDeviceObserver(DeviceType type, const std::string &name)
    {
        m_DeviceManager->AddDeviceObserver(type, name);
    }

    bool LithiumApp::removeDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager->removeDevice(type, name);
    }

    bool LithiumApp::removeDevicesByName(const std::string &name)
    {
        return m_DeviceManager->removeDevicesByName(name);
    }

    bool LithiumApp::removeDeviceLibrary(const std::string &lib_name)
    {
        return m_DeviceManager->removeDeviceLibrary(lib_name);
    }

    std::shared_ptr<Device> LithiumApp::getDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager->getDevice(type, name);
    }

    size_t LithiumApp::findDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager->findDevice(type, name);
    }

    std::shared_ptr<Device> LithiumApp::findDeviceByName(const std::string &name) const
    {
        return m_DeviceManager->findDeviceByName(name);
    }

    std::shared_ptr<SimpleTask> LithiumApp::getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        return m_DeviceManager->getTask(type, device_name, task_name, params);
    }

    bool LithiumApp::createProcess(const std::string &command, const std::string &identifier)
    {
        return m_ProcessManager->createProcess(command, identifier);
    }

    bool LithiumApp::runScript(const std::string &script, const std::string &identifier)
    {
        return m_ProcessManager->runScript(script, identifier);
    }

    bool LithiumApp::terminateProcess(pid_t pid, int signal)
    {
        return m_ProcessManager->terminateProcess(pid, signal);
    }

    bool LithiumApp::terminateProcessByName(const std::string &name, int signal)
    {
        return m_ProcessManager->terminateProcessByName(name, signal);
    }

    std::vector<Process::Process> LithiumApp::getRunningProcesses()
    {
        return m_ProcessManager->getRunningProcesses();
    }

    std::vector<std::string> LithiumApp::getProcessOutput(const std::string &identifier)
    {
        return m_ProcessManager->getProcessOutput(identifier);
    }

    bool LithiumApp::addTask(const std::shared_ptr<BasicTask> &task)
    {
        return m_TaskManager->addTask(task);
    }

    bool LithiumApp::insertTask(const std::shared_ptr<BasicTask> &task, int position)
    {
        return m_TaskManager->insertTask(task, position);
    }

    bool LithiumApp::executeAllTasks()
    {
        return m_TaskManager->executeAllTasks();
    }

    bool LithiumApp::stopTask()
    {
        m_TaskManager->stopTask();
        return true;
    }

    bool LithiumApp::executeTaskByName(const std::string &name)
    {
        return m_TaskManager->executeTaskByName(name);
    }

    bool LithiumApp::modifyTask(int index, const std::shared_ptr<BasicTask> &task)
    {
        return m_TaskManager->modifyTask(index, task);
    }

    bool LithiumApp::modifyTaskByName(const std::string &name, const std::shared_ptr<BasicTask> &task)
    {
        return m_TaskManager->modifyTaskByName(name, task);
    }

    bool LithiumApp::deleteTask(int index)
    {
        return m_TaskManager->deleteTask(index);
    }

    bool LithiumApp::deleteTaskByName(const std::string &name)
    {
        return m_TaskManager->deleteTaskByName(name);
    }

    bool LithiumApp::queryTaskByName(const std::string &name)
    {
        return m_TaskManager->queryTaskByName(name);
    }

    const std::vector<std::shared_ptr<BasicTask>> &LithiumApp::getTaskList() const
    {
        return m_TaskManager->getTaskList();
    }

    bool LithiumApp::saveTasksToJson() const
    {
        return m_TaskManager->saveTasksToJson();
    }

    void LithiumApp::addThread(std::function<void()> func, const std::string &name)
    {
        m_ThreadManager->addThread(func, name);
    }

    void LithiumApp::joinAllThreads()
    {
        m_ThreadManager->joinAllThreads();
    }

    void LithiumApp::joinThreadByName(const std::string &name)
    {
        m_ThreadManager->joinThreadByName(name);
    }

    bool LithiumApp::sleepThreadByName(const std::string &name, int seconds)
    {
        return m_ThreadManager->sleepThreadByName(name, seconds);
    }

    bool LithiumApp::isThreadRunning(const std::string &name)
    {
        return m_ThreadManager->isThreadRunning(name);
    }

}