/*
 * plugin_task.cpp
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

Description: Plugin Task Definition

**************************************************/

#include "plugin_task.hpp"

PluginTask::PluginTask(const std::function<json(const json &)> &func,
                       const json &params_template,
                       const std::string &plugin_name,
                       const std::function<json(const json &)> &stop_fn,
                       bool can_stop)
    : SimpleTask(func, params_template, stop_fn, can_stop), plugin_name_(plugin_name)
{
}

/**
 * @brief 获取插件名称
 * @return 插件名称
 */
const std::string &PluginTask::get_plugin_name() const
{
    return plugin_name_;
}

/**
 * @brief 设置插件名称
 * @param plugin_name 要设置的插件名称
 */
void PluginTask::set_plugin_name(const std::string &plugin_name)
{
    plugin_name_ = plugin_name;
}