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

#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <fstream>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif


#include "atom/task/task.hpp"

#include "atom/type/expected.hpp"

namespace Lithium
{
    /**
     * @brief 任务管理器类，用于管理任务列表和相关操作。
     */
    class TaskManager
    {
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
        // Task methods
        // -------------------------------------------------------------------

        /**
         * @brief 添加任务到任务列表末尾。
         * @brief Add task to the end of task list.
         * @param nama 任务名称。
         * 
         * @param params 任务参数。
         * @return 添加成功返回 true，否则返回 false。
         */
        bool addTask(const std::string nama, const json& params);

        /**
         * @brief 在指定位置插入任务到任务列表。
         * @param task 要插入的任务指针。
         * @param position 要插入的位置索引。
         * @return 插入成功返回 true，否则返回 false。
         */
        bool insertTask(const std::string nama, const json& params, int position);

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
         * @brief 根据任务名称执行任务。
         * @param name 任务名称。
         * @return 执行成功返回 true，否则返回 false。
         */
        bool executeTaskByName(const std::string &name);

        /**
         * @brief 修改指定位置的任务。
         * @param index 任务在列表中的位置索引。
         * @param task 新的任务指针。
         * @return 修改成功返回 true，否则返回 false。
         */
        bool modifyTask(int index, const std::shared_ptr<Atom::Task::SimpleTask> &task);

        /**
         * @brief 根据任务名称修改任务。
         * @param name 任务名称。
         * @param task 新的任务指针。
         * @return 修改成功返回 true，否则返回 false。
         */
        bool modifyTaskByName(const std::string &name, const std::shared_ptr<Atom::Task::SimpleTask> &task);

        /**
         * @brief 删除指定位置的任务。
         * @param index 任务在列表中的位置索引。
         * @return 删除成功返回 true，否则返回 false。
         */
        bool deleteTask(int index);

        /**
         * @brief 根据任务名称删除任务。
         * @param name 任务名称。
         * @return 删除成功返回 true，否则返回 false。
         */
        bool deleteTaskByName(const std::string &name);

        /**
         * @brief 根据任务名称查询任务是否存在。
         * @param name 任务名称。
         * @return 如果任务存在，则返回 true，否则返回 false。
         */
        bool queryTaskByName(const std::string &name);

        /**
         * @brief 获取任务列表。
         * @return 任务列表的常量引用。
         */
        const std::vector<std::shared_ptr<Atom::Task::SimpleTask>> &getTaskList() const;

        /**
         * @brief 将任务列表保存为 JSON 文件。
         * @return 保存成功返回 true，否则返回 false。
         */
        bool saveTasksToJson() const;

    private:
        std::vector<std::shared_ptr<Atom::Task::SimpleTask>> m_TaskList;                    /**< 任务列表 */
        std::unordered_map<std::string, std::shared_ptr<Atom::Task::SimpleTask>> m_TaskMap; /**< 任务名称到任务指针的映射表 */
        std::string m_FileName;                                                             /**< 任务列表的文件名 */
        bool m_StopFlag;                                                                    /**< 停止标志，用于中止当前正在执行的任务 */

        /**
         * @brief 根据任务名称查找任务。
         * @param name 任务名称。
         * @return 找到的任务指针的迭代器，如果未找到则返回 m_TaskMap.end()。
         */
        std::unordered_map<std::string, std::shared_ptr<Atom::Task::SimpleTask>>::iterator findTaskByName(const std::string &name);
    };

} // namespace Lithium

#endif
