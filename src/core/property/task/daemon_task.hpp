/*
 * daemon_task.hpp
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

Date: 2023-7-19

Description: Daemon Task Definition

**************************************************/

#pragma once

#include "task.hpp"

namespace Lithium
{
    class DaemonTask : public BasicTask
    {
    public:
        /**
         * @brief 构造函数
         * @param task_fn 任务函数
         * @param stop_fn 任务停止函数
         */
        DaemonTask(const std::function<void()> &task_fn,
                   std::function<json(const json &)> &stop_fn);

        /**
         * @brief 执行任务
         * @return JSON对象表示的任务状态
         */
        const json execute() override;

        /**
         * @brief 将任务序列化为JSON对象
         * @return JSON对象表示的任务
         */
        const json toJson() const override;

    private:
        std::function<void()> task_fn_; // 任务函数

        /**
         * @brief 在循环中运行任务的线程
         */
        void runTask();
    };
}