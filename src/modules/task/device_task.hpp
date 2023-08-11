#pragma once

#include "task.hpp"

class DeviceTask : public Lithium::SimpleTask
{
public:
    /**
     * @brief DeviceTask类构造函数
     * @param func 要执行的函数
     * @param params_template 参数模板，用于参数验证
     * @param device_name 设备名称
     * @param stop_fn 一个可选的停止函数，默认为nullptr
     * @param can_stop 指示任务是否可以停止，默认为false
     */
    DeviceTask(const std::function<nlohmann::json(const nlohmann::json &)> &func,
               const nlohmann::json &params_template,
               const std::string &device_name,
               const std::string &device_uuid,
               const std::string &device_device_name,
               const std::function<nlohmann::json(const nlohmann::json &)> &stop_fn,
               bool can_stop = false)
        : SimpleTask(func, params_template, stop_fn, can_stop), device_name_(device_name), device_uuid_(device_uuid), device_device_name_(device_device_name)
    {
    }

    /**
     * @brief 获取设备名称
     * @return 设备名称
     */
    const std::string &get_device_name() const
    {
        return device_name_;
    }

    /**
     * @brief 设置设备名称
     * @param device_name 要设置的设备名称
     */
    void set_device_name(const std::string &device_name)
    {
        device_name_ = device_name;
    }

    /**
     * @brief 获取设备UUID
     * @return 设备UUID
     */
    const std::string &get_device_uuid() const
    {
        return device_uuid_;
    }

    /**
     * @brief 获取设备真实名称
     * @return 设备真实名称
     */
    const std::string &get_device_device_name() const
    {
        return device_device_name_;
    }

    /**
     * @brief 设置设备名称
     * @param device_name 要设置的设备名称
     */
    void set_device_device_name(const std::string &device_device_name)
    {
        device_device_name_ = device_device_name;
    }

private:
    // 设备名称
    std::string device_name_;
    std::string device_uuid_;
    std::string device_device_name_;
};
