#pragma once

#include "task.hpp"

class PluginTask : public Lithium::SimpleTask
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
    PluginTask(const std::function<nlohmann::json(const nlohmann::json &)> &func,
               const nlohmann::json &params_template,
               const std::string &plugin_name,
               std::function<nlohmann::json(const nlohmann::json &)>&stop_fn = nullptr,
               bool can_stop = false)
        : SimpleTask(func, params_template, stop_fn, can_stop), plugin_name_(plugin_name)
    {
    }

    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    const std::string &get_plugin_name() const
    {
        return plugin_name_;
    }

    /**
     * @brief 设置插件名称
     * @param plugin_name 要设置的插件名称
     */
    void set_plugin_name(const std::string &plugin_name)
    {
        plugin_name_ = plugin_name;
    }

private:
    // 插件名称
    std::string plugin_name_;
};
