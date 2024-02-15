/*
 * task_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Manager

**************************************************/

#include "manager.hpp"

#include "atom/server/global_ptr.hpp"
#include "atom/log/loguru.hpp"

namespace Lithium
{
    TaskManager::TaskManager()
        : m_StopFlag(false)
    {
        // Load Task Component from Global Ptr Manager
        m_TaskContainer = GetPtr<TaskContainer>("lithium.task.contianer");
        m_TaskPool = GetPtr<TaskPool>("lithium.task.pool");
        m_TaskList = GetPtr<TaskList>("lithium.task.list");
        m_TaskGenerator = GetPtr<TaskGenerator>("lithium.task.generator");
        m_TickScheduler = GetPtr<TickScheduler>("lithium.task.tick");
        m_TaskLoader = GetPtr<TaskLoader>("ltihium.task.loader");
    }

    TaskManager::~TaskManager()
    {
        saveTasksToJson();
    }

    std::shared_ptr<TaskManager> TaskManager::createShared()
    {
        return std::make_shared<TaskManager>();
    }

    bool TaskManager::addTask(const std::string name, const json &params)
    {
        // Task Check Needed
        m_TaskList->addOrUpdateTask(name, params);
        return true;
    }

    bool TaskManager::insertTask(const int &position, const std::string &name, const json &params)
    {
        if (m_TaskList->insertTask(name, params, position))
        {
            DLOG_F(INFO, "Insert {} task to {}", name, position);
        }
        else
        {
        }
        return true;
    }

    bool TaskManager::modifyTask(const std::string &name, const json &params)
    {
        m_TaskList->addOrUpdateTask(name, params);
        return true;
    }

    bool TaskManager::deleteTask(const std::string &name)
    {
        m_TaskList->removeTask(name);
        return true;
    }

    bool TaskManager::executeAllTasks()
    {
        for (const auto &[name, params] : m_TaskList->getTasks())
        {
            DLOG_F(INFO, "Run task {}", name);
            std::string task_type = params["type"].get<std::string>();
            if (auto task = m_TaskContainer->getTask(task_type); task.has_value())
            {
                json t_params = params["params"];
                auto handle = m_TickScheduler->scheduleTask(1, false, 1, std::chrono::milliseconds(0), std::nullopt, std::nullopt, std::nullopt, task.value()->m_function, t_params);

                if (params.contains("callbacks"))
                {
                    std::vector<std::string> callbacks = params["callbacks"];
                    for (auto callback : callbacks)
                    {
                        auto c_task = m_TaskContainer->getTask(task_type);
                        if (c_task.has_value())
                        {
                            m_TickScheduler->setCompletionCallback(handle, [c_task]()
                                                                   { c_task.value()->m_function({}); });
                        }
                    }
                }
                if (params.contains("timers"))
                {
                    std::vector<json> timers = params["timers"];
                    for (auto timer : timers)
                    {
                        if (!timer.contains("name") || !timer.contains("params") || !timer.contains("delay"))
                        {
                            continue;
                        }
                        else
                        {
                            std::string timer_name = timer["name"];
                            int tick = timer["delay"];
                            if (auto tt_task = m_TaskContainer->getTask(name); tt_task.has_value())
                            {
                                m_TickScheduler->scheduleTask(tick, false, 1, std::chrono::milliseconds(0), std::nullopt, std::nullopt, std::nullopt, tt_task.value()->m_function, timer["params"]);
                            }
                        }
                    }
                }
                m_Timer->start();
                while (!handle->completed.load() || !m_StopFlag.load())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (params.contains("timeout") && m_Timer->elapsedSeconds() > params["timeout"].get<double>())
                    {
                        LOG_F(ERROR, "Timeout");
                        break;
                    }
                }
                m_Timer->stop();
                m_Timer->reset();
            }
            else
            {
                LOG_F(ERROR, "Task {} contains a invalid function target");
            }
            if (m_StopFlag.load())
            {
                break;
            }
        }
        return true;
    }

    void TaskManager::stopTask()
    {
        m_StopFlag.store(true);
    }

    bool TaskManager::executeTaskByName(const std::string &name)
    {
        return false;
    }

    bool TaskManager::saveTasksToJson() const
    {
        return true;
    }
}