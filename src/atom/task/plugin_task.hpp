/*
 * plugin_task.hpp
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

Date: 2023-7-19

Description: Plugin Task Definition

**************************************************/

#pragma once

#include "task.hpp"

class PluginTask : public Atom::Task::SimpleTask
{
public:
    /**
     * @brief PluginTask类构造函数
     * @param func 要执行的函数
     * @param params_template 参数模板，用于参数验证
     * @param plugin_name 插件名称
     * @param stop_fn 一个可选的停止函数，默认为nullptr
     * @param can_stop 指示任务是否可以停止，默认为false
     */
    PluginTask(const std::function<json(const json &)> &func,
               const json &params_template,
               const std::string &plugin_name,
               const std::function<json(const json &)>&stop_fn = nullptr,
               bool can_stop = false);

    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    const std::string &get_plugin_name() const;

    /**
     * @brief 设置插件名称
     * @param plugin_name 要设置的插件名称
     */
    void set_plugin_name(const std::string &plugin_name);

private:
    // 插件名称
    std::string plugin_name_;
};
