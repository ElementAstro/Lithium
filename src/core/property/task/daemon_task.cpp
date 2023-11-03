/*
 * daemon_task.cpp
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

#include "daemon_task.hpp"

namespace Lithium
{
    DaemonTask::DaemonTask(const std::function<void()> &task_fn,
                           std::function<json(const json &)> &stop_fn)
        : BasicTask(stop_fn, true), task_fn_(task_fn)
    {
    }

    const json DaemonTask::execute()
    {
        // 执行任务的具体逻辑实现
#if __cplusplus >= 202002L
        std::jthread task_thread([this](std::stop_token stoken)
                                 {
#else
        std::jthread task_thread([this](std::stop_token stoken)
                                 {
#endif
        while (!stoken.stop_requested())
        {
            if (task_fn_)
            {
                task_fn_();
            }
        }
        done_ = true; });
        task_thread.detach(); // 分离任务线程
        return {{"status", "running"}};
    }

    const json DaemonTask::toJson() const
    {
        auto json = BasicTask::toJson();
        json["type"] = "daemon";
        return json;
    }

    void DaemonTask::runTask()
    {
        while (!stop_flag_)
        {
            task_fn_();
        }
        done_ = true;
    }

}