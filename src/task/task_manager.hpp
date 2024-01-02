/*
 * task_manager.hpp
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

Date: 2023-7-21

Description: Task Manager

**************************************************/

#pragma once

#include <vector>
#include <memory>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <stdexcept>
#include <fstream>

#include "atom/property/task/task.hpp"
#include "atom/type/json.hpp"

namespace Lithium::Task
{
    /**
     * @brief 任务管理器类，用于管理任务列表和相关操作。
     */
    class TaskManager
    {
    public:
        /**
         * @brief 构造函数，用于创建任务管理器对象并加载任务列表。
         * @param fileName 任务列表的文件名。
         */
        TaskManager(const std::string &fileName);

        /**
         * @brief 添加任务到任务列表末尾。
         * @param task 要添加的任务指针。
         * @return 添加成功返回 true，否则返回 false。
         */
        bool addTask(const std::shared_ptr<Atom::Task::SimpleTask> &task);

        /**
         * @brief 在指定位置插入任务到任务列表。
         * @param task 要插入的任务指针。
         * @param position 要插入的位置索引。
         * @return 插入成功返回 true，否则返回 false。
         */
        bool insertTask(const std::shared_ptr<Atom::Task::SimpleTask> &task, int position);

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
        std::string m_FileName;                                                /**< 任务列表的文件名 */
        bool m_StopFlag;                                                       /**< 停止标志，用于中止当前正在执行的任务 */

        /**
         * @brief 根据任务名称查找任务。
         * @param name 任务名称。
         * @return 找到的任务指针的迭代器，如果未找到则返回 m_TaskMap.end()。
         */
        std::unordered_map<std::string, std::shared_ptr<Atom::Task::SimpleTask>>::iterator findTaskByName(const std::string &name);
    };

} // namespace Lithium
