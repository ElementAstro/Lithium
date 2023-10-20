/*
 * device.hpp
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

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#ifndef DEVICE_H
#define DEVICE_H

#include <any>
#include <functional>
#include <memory>
#include <thread>

#include "nlohmann/json.hpp"
#include "emhash/hash_table8.hpp"

#include "property/iproperty.hpp"
#include "property/task/device_task.hpp"

#include "device_exception.hpp"
#include "deviceio.hpp"

typedef emhash8::HashMap<std::string, std::any> IParams;

typedef emhash8::HashMap<std::string, std::any> IReturns;

class Device
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

public:
    virtual bool connect(const IParams &params) { return true; };

    virtual bool disconnect(const IParams &params) { return true; };

    virtual bool reconnect(const IParams &params) { return true; };

    virtual bool isConnected() { return true; }

    virtual void init();

    const std::string getDeviceName();

    void insertProperty(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check = false);

    void setProperty(const std::string &name, const std::any &value);

    std::any getProperty(const std::string &name, bool need_refresh = true);

    void removeProperty(const std::string &name);

    std::shared_ptr<INumberProperty> getNumberProperty(const std::string &name);

    std::shared_ptr<IStringProperty> getStringProperty(const std::string &name);

    std::shared_ptr<IBoolProperty> getBoolProperty(const std::string &name);

    void insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                    const std::function<nlohmann::json(const nlohmann::json &)> &func,
                    const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                    bool isBlock = false);

    bool removeTask(const std::string &name);

    std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params);

    void addObserver(const std::function<void(const std::any &message)> &observer);

    void removeObserver(const std::function<void(const std::any &message)> &observer);

    const nlohmann::json exportDeviceInfoToJson();

private:
    // Why we use emhash8 : because it is so fast!
    // Map of the properties
    emhash8::HashMap<std::string, std::any> m_properties;
    // Vector of the observers, though one driver will only have one observer one time, but this is necessary
    std::vector<std::function<void(const std::any &)>> m_observers;
    // Map of the task
    emhash8::HashMap<std::string, std::shared_ptr<DeviceTask>> task_map;
    // Basic Device name and UUID
    std::string _name;
    std::string _uuid;

public:
    EventLoop eventLoop;

    std::jthread loopThread;

    std::shared_ptr<SocketServer> deviceIOServer;

public:
    typedef bool (*ConnectFunc)(const IParams &params);
    typedef bool (*DisconnectFunc)(const IParams &params);
    typedef bool (*ReconnectFunc)(const IParams &params);
    typedef void (*InitFunc)();
    typedef void (*InsertPropertyFunc)(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check);
    typedef void (*SetPropertyFunc)(const std::string &name, const std::any &value);
    typedef std::any (*GetPropertyFunc)(const std::string &name);
    typedef void (*RemovePropertyFunc)(const std::string &name);
    typedef void (*InsertTaskFunc)(const std::string &name, std::any defaultValue, nlohmann::json params_template, const std::function<nlohmann::json(const nlohmann::json &)> &func, const std::function<nlohmann::json(const nlohmann::json &)> &stop_func, bool isBlock);
    typedef bool (*RemoveTaskFunc)(const std::string &name);
    typedef std::shared_ptr<Lithium::SimpleTask> (*GetTaskFunc)(const std::string &name, const nlohmann::json &params);
    typedef void (*AddObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef void (*RemoveObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef const nlohmann::json (*ExportDeviceInfoToJsonFunc)();

    /**
     * @brief HandlerFunc 是用于处理命令的函数类型。
     *
     * 该函数应该接受一个 `json` 类型的参数，表示命令所携带的数据。
     */
    using HandlerFunc = std::function<const IReturns(const IParams &)>;

    /**
     * @brief RegisterHandler 函数用于将一个命令处理程序注册到 `CommandDispatcher` 中。
     *
     * @tparam ClassType 命令处理程序所属的类类型。
     * @param name 命令的名称。
     * @param handler 处理命令的成员函数指针。
     * @param instance 处理命令的对象指针。
     */
    template <typename ClassType>
    void RegisterHandler(const std::string &name, const IReturns (ClassType::*handler)(const IParams &), ClassType *instance)
    {
        auto hash_value = Djb2Hash(name.c_str());
        command_handlers_[hash_value] = std::bind(handler, instance, std::placeholders::_1);
    }

    /**
     * @brief HasHandler 函数用于检查是否有名为 `name` 的命令处理程序。
     *
     * @param name 要检查的命令名称。
     * @return 如果存在名为 `name` 的命令处理程序，则返回 `true`；否则返回 `false`。
     */
    bool HasHandler(const std::string &name);

    /**
     * @brief Dispatch 函数用于派发一个命令，并将它交给相应的处理程序处理。
     *
     * @param name 要派发的命令的名称。
     * @param data 命令所携带的数据。
     */
    IReturns Dispatch(const std::string &name, const IParams &data);

private:
    /**
     * @brief command_handlers_ 是一个哈希表，存储了所有已注册的命令处理程序。
     *
     * 键值为哈希值，值为命令处理程序本身。
     */
    std::unordered_map<std::size_t, HandlerFunc> command_handlers_;

    /**
     * @brief Djb2Hash 函数是一个字符串哈希函数，用于将字符串转换成哈希值。
     *
     * @param str 要转换的字符串。
     * @return 转换后的哈希值。
     */
    static std::size_t Djb2Hash(const char *str);
};

#endif // DEVICE_H
