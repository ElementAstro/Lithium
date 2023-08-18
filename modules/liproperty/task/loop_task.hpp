/*
 * loop_task.hpp
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

Description: Loop Task Definition

**************************************************/

#pragma once

#include "task.hpp"

namespace Lithium
{
    class LoopTask : public BasicTask
    {
    public:
        /**
         * @brief LoopTask类构造函数
         * @param item_fn 单项任务函数，对每个参数执行一次
         * @param params 任务参数
         * @param stop_fn 一个可选的停止函数，默认为nullptr
         */
        LoopTask(const std::function<void(const nlohmann::json &)> &item_fn,
                 const nlohmann::json &params,
                 std::function<nlohmann::json(const nlohmann::json &)> &stop_fn);

        /**
         * @brief 执行任务的虚函数，由子类实现具体逻辑
         * @return 以json格式返回任务执行结果
         */
        virtual nlohmann::json Execute() override;

        /**
         * @brief 将任务序列化为JSON对象
         * @return 表示任务的JSON对象
         */
        virtual const nlohmann::json ToJson() const override;

    private:
        // 单项任务函数，对每个参数执行一次
        std::function<void(const nlohmann::json &)> item_fn_;

        // 任务参数
        nlohmann::json params_;
    };
}