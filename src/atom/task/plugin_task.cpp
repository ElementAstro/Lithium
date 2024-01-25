/*
 * plugin_task.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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