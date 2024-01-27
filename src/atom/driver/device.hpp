/*
 * device.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#pragma once

#define ATOM_DRIVER_DEFINITION

#include <any>
#include <functional>
#include <memory>
#include <thread>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/task/device_task.hpp"

#include "atom/components/component.hpp"
#include "atom/type/args.hpp"

#include "device_exception.hpp"

#include "iproperty.hpp"


class Device : public Component
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

public:
    virtual bool connect(const json &params) { return true; };

    virtual bool disconnect(const json &params) { return true; };

    virtual bool reconnect(const json &params) { return true; };

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

    void insertTask(const std::string &name, std::any defaultValue, json params_template,
                    const std::function<json(const json &)> &func,
                    const std::function<json(const json &)> &stop_func,
                    bool isBlock = false);

    bool removeTask(const std::string &name);

    std::shared_ptr<DeviceTask> getTask(const std::string &name, const json &params);

private:
    // Why we use emhash8 : because it is so fast!
    // Map of the properties
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_properties;
    emhash8::HashMap<std::string, std::shared_ptr<DeviceTask>> m_task_map;
#else
    std::unordered_map<std::string, std::any> m_properties;
    std::unordered_map<std::string, std::shared_ptr<DeviceTask>> m_task_map;
#endif

    std::unique_ptr<CommandDispatcher<json,json>> m_commander;

    // Basic Device name and UUID
    std::string _name;
    std::string _uuid;

public:
    typedef bool (*ConnectFunc)(const json &params);
    typedef bool (*DisconnectFunc)(const json &params);
    typedef bool (*ReconnectFunc)(const json &params);
    typedef void (*InitFunc)();
    typedef void (*InsertPropertyFunc)(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check);
    typedef void (*SetPropertyFunc)(const std::string &name, const std::any &value);
    typedef std::any (*GetPropertyFunc)(const std::string &name);
    typedef void (*RemovePropertyFunc)(const std::string &name);
    typedef void (*InsertTaskFunc)(const std::string &name, std::any defaultValue, json params_template, const std::function<json(const json &)> &func, const std::function<json(const json &)> &stop_func, bool isBlock);
    typedef bool (*RemoveTaskFunc)(const std::string &name);
    typedef std::shared_ptr<DeviceTask> (*GetTaskFunc)(const std::string &name, const json &params);
    typedef void (*AddObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef void (*RemoveObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef const json (*ExportDeviceInfoTojsonFunc)();
};
