/*
 * manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Manager

**************************************************/

#ifndef LITHIUM_TASK_MANAGER_HPP
#define LITHIUM_TASK_MANAGER_HPP

#include "config.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// #include "checker.hpp"
#include "container.hpp"
#include "generator.hpp"
#include "list.hpp"
#include "loader.hpp"
#include "pool.hpp"
#include "tick.hpp"

#include "atom/task/task.hpp"

#include "atom/utils/stopwatcher.hpp"

namespace Lithium {
/**
 * @brief 任务管理器类，用于管理任务列表和相关操作。
 */
class TaskManager {
public:
    // -------------------------------------------------------------------
    // Class methods
    // -------------------------------------------------------------------

    /**
     * @brief 构造函数
     * @brief Constructor
     */
    explicit TaskManager();

    /**
     * @brief 析构函数
     * @brief Destructor
     */
    ~TaskManager();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * @brief 创建任务管理器的共享实例。
     * @brief Create shared instance of TaskManager
     * @return 返回任务管理器的共享实例。
     * @return Return shared instance of TaskManager
     */
    static std::shared_ptr<TaskManager> createShared();

    // -------------------------------------------------------------------
    // Task methods (logic)
    // -------------------------------------------------------------------

    /**
     * @brief 添加任务到任务列表末尾。
     * @brief Add task to the end of task list.
     * @param name 任务名称。
     * @param name The name of the task
     * @param params 任务参数。
     * @return 添加成功返回 true，否则返回 false。
     * @note 在这里的名称对应的是任务的名称，实际存储为序号
     */
    bool addTask(const std::string name, const json &params);

    /**
     * @brief 根据任务名称删除任务。
     * @param name 任务名称。
     * @return 删除成功返回 true，否则返回 false。
     */
    bool deleteTask(const std::string &name);

    bool insertTask(const int &position, const std::string &name,
                    const json &params);

    /**
     * @brief 根据任务名称修改任务。
     * @param name 任务名称。
     * @param task 新的任务指针。
     * @return 修改成功返回 true，否则返回 false。
     */
    bool modifyTask(const std::string &name, const json &params);

    /**
     * @brief 根据任务名称执行任务。
     * @param name 任务名称。
     * @return 执行成功返回 true，否则返回 false。
     */
    bool executeTaskByName(const std::string &name);

    /**
     * @brief 执行所有任务。
     * @return 执行成功返回 true，否则返回 false。
     */
    bool executeAllTasks();

    /**
     * @brief 停止当前正在执行的任务。
     */
    void stopTask();

    /**
     * @brief 获取任务列表。
     * @return 任务列表的常量引用。
     */
    [[nodiscard]] std::vector<std::shared_ptr<SimpleTask>> &getTaskList() const;

    /**
     * @brief 将任务列表保存为 JSON 文件。
     * @return 保存成功返回 true，否则返回 false。
     */
    bool saveTasksToJson() const;

    // -------------------------------------------------------------------
    // Task methods (function)
    // -------------------------------------------------------------------

    void loadBuiltinTask();

private:
    std::weak_ptr<TaskContainer> m_TaskContainer;
    std::weak_ptr<TaskGenerator> m_TaskGenerator;
    std::weak_ptr<TaskList> m_TaskList;
    std::weak_ptr<TaskLoader> m_TaskLoader;
    std::weak_ptr<TaskPool> m_TaskPool;
    std::weak_ptr<TickScheduler> m_TickScheduler;

    std::unique_ptr<Atom::Utils::StopWatcher> m_Timer;

    std::atomic_bool m_StopFlag;
};

}  // namespace Lithium

#endif
