/*
 * device_manager.cpp
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

Date: 2023-3-29

Description: Device Manager

**************************************************/

#include "device_manager.hpp"

#include "core/device_exception.hpp"

#include "nlohmann/json.hpp"

#include "core/camera.hpp"
#include "core/telescope.hpp"
#include "core/focuser.hpp"
#include "core/filterwheel.hpp"
#include "core/solver.hpp"
#include "core/guider.hpp"

#include "core/camera_utils.hpp"

#define LOGURU_USE_FMTLIB
#include "loguru/loguru.hpp"

#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#include <typeinfo>

#include "indi_device.hpp"
#include "indidevice_manager.hpp"

// For DEVICE_FUNC

#define CHECK_MAIN_CAMERA                                                  \
    if (!m_main_camera)                                                    \
    {                                                                      \
        LOG_F(ERROR, "Main camera not specified on calling {}", __func__); \
        return DeviceError::NotSpecific;                                   \
    }

#define CHECK_TELESCOPE                                                       \
    if (!m_telescope)                                                         \
    {                                                                         \
        LOG_F(ERROR, "Main telescope not specified on calling {}", __func__); \
        return DeviceError::NotSpecific;                                      \
    }

#define CHECK_FOCUSER                                                       \
    if (!m_focuser)                                                         \
    {                                                                       \
        LOG_F(ERROR, "Main focuser not specified on calling {}", __func__); \
        return DeviceError::NotSpecific;                                    \
    }

#define CHECK_FILTERWHEEL                                                       \
    if (!m_filterwheel)                                                         \
    {                                                                           \
        LOG_F(ERROR, "Main filterwheel not specified on calling {}", __func__); \
        return DeviceError::NotSpecific;                                        \
    }

#define CHECK_GUIDER                                                       \
    if (!m_guider)                                                         \
    {                                                                      \
        LOG_F(ERROR, "Main guider not specified on calling {}", __func__); \
        return DeviceError::NotSpecific;                                   \
    }

// For DEVICE_FUNC_J

#define CHECK_MAIN_CAMERA_J                                                \
    if (!m_main_camera)                                                    \
    {                                                                      \
        LOG_F(ERROR, "Main camera not specified on calling {}", __func__); \
        return {{"error", "no camera specified"}};                         \
    }

#define CHECK_TELESCOPE_J                                                     \
    if (!m_telescope)                                                         \
    {                                                                         \
        LOG_F(ERROR, "Main telescope not specified on calling {}", __func__); \
        return {{"error", "no telescope specified"}};                         \
    }

#define CHECK_FOCUSER_J                                                     \
    if (!m_focuser)                                                         \
    {                                                                       \
        LOG_F(ERROR, "Main focuser not specified on calling {}", __func__); \
        return {{"error", "no focuser specified"}};                         \
    }

#define CHECK_FILTERWHEEL_J                                                     \
    if (!m_filterwheel)                                                         \
    {                                                                           \
        LOG_F(ERROR, "Main filterwheel not specified on calling {}", __func__); \
        return {{"error", "no filterwheel specified"}};                         \
    }

#define CHECK_GUIDER_J                                                     \
    if (!m_guider)                                                         \
    {                                                                      \
        LOG_F(ERROR, "Main guider not specified on calling {}", __func__); \
        return {{"error", "no guider specified"}};                         \
    }

namespace Lithium
{

    // Constructor
    DeviceManager::DeviceManager(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager)
    {
        m_ModuleLoader = ModuleLoader::createShared("drivers");
        m_ConfigManager = configManager;
        m_MessageBus = messageBus;
        for (auto &devices : m_devices)
        {
            devices.emplace_back();
        }

        m_indimanager = std::make_shared<INDIManager>();
    }

    DeviceManager::~DeviceManager()
    {
        for (auto &devices : m_devices)
        {
            for (auto &device : devices)
            {
                if (device)
                {
                    // device->disconnect();
                }
            }
        }
    }

    std::shared_ptr<DeviceManager> DeviceManager::createShared(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager)
    {
        return std::make_shared<DeviceManager>(messageBus, configManager);
    }

    std::vector<std::string> DeviceManager::getDeviceList(DeviceType type)
    {
        std::vector<std::string> deviceList;
        auto &devices = m_devices[static_cast<int>(type)];
        for (const auto &device : devices)
        {
            if (device)
            {
                deviceList.emplace_back(device->getDeviceName());
            }
        }
        return deviceList;
    }

    bool DeviceManager::addDevice(DeviceType type, const std::string &name, const std::string &lib_name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (type < DeviceType::Camera || type > DeviceType::Guider)
        {
            throw InvalidDeviceType("Invalid device type");
        }
        if (findDeviceByName(name))
        {
            LOG_F(ERROR, "A device with name {} already exists, please choose a different name", name.c_str());
            return false;
        }
        std::string newName = name;
        int index = 1;
        while (findDevice(type, newName) != -1)
        {
#if __cplusplus >= 202002L
#ifdef __cpp_lib_format
            newName = std::format("{}-{}", name, index++);
#else

#endif
#else
            std::stringstream ss;
            ss << name << "-" << index++;
            newName = ss.str();
#endif
        }
        try
        {
            if (lib_name.empty())
            {
                switch (type)
                {
                case DeviceType::Camera:
                {
                    LOG_F(INFO, "Trying to add a new camera instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Camera>(newName));
                    LOG_F(INFO, "Added new camera {} instance successfully", newName.c_str());
                    break;
                }
                case DeviceType::Telescope:
                {
                    LOG_F(INFO, "Trying to add a new telescope instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                    LOG_F(INFO, "Added new telescope instance successfully");
                    break;
                }
                case DeviceType::Focuser:
                {
                    LOG_F(INFO, "Trying to add a new Focuser instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                    LOG_F(INFO, "Added new focuser instance successfully");
                    break;
                }
                case DeviceType::FilterWheel:
                {
                    LOG_F(INFO, "Trying to add a new filterwheel instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Filterwheel>(newName));
                    LOG_F(INFO, "Added new filterwheel instance successfully");
                    break;
                }
                case DeviceType::Solver:
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
                    break;
                case DeviceType::Guider:
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
                    break;
                default:
                    LOG_F(ERROR, "Invalid device type");
                    throw InvalidDeviceType("Invalid device type");
                    break;
                }
            }
            else
            {
                nlohmann::json params;
                params["name"] = newName;
                switch (type)
                {
                case DeviceType::Camera:
                {
                    LOG_F(INFO, "Trying to add a new camera instance : {} from {}", newName.c_str(), lib_name.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(m_ModuleLoader->GetInstance<Camera>(lib_name, params, "GetInstance"));
                    LOG_F(INFO, "Added new camera {} instance successfully", newName.c_str());
                    break;
                }
                case DeviceType::Telescope:
                {
                    LOG_F(INFO, "Trying to add a new telescope instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                    LOG_F(INFO, "Added new telescope instance successfully");
                    break;
                }
                case DeviceType::Focuser:
                {
                    LOG_F(INFO, "Trying to add a new Focuser instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                    LOG_F(INFO, "Added new focuser instance successfully");
                    break;
                }
                case DeviceType::FilterWheel:
                {
                    LOG_F(INFO, "Trying to add a new filterwheel instance : {}", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Filterwheel>(newName));
                    LOG_F(INFO, "Added new filterwheel instance successfully");
                    break;
                }
                case DeviceType::Solver:
                {
                    LOG_F(INFO, "Trying to add a new solver instance : {} from {}", newName.c_str(), lib_name.c_str());
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
                    break;
                }
                case DeviceType::Guider:
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
                    break;
                default:
                    LOG_F(ERROR, "Invalid device type");
                    throw InvalidDeviceType("Invalid device type");
                    break;
                }
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to add device {} , error : {}", newName.c_str(), e.what());
            return false;
        }
        if (m_ConfigManager)
        {
#ifdef __cpp_lib_format
            m_ConfigManager->setValue(std::format("driver/{}/name", newName), newName);
#else
#endif
        }
        else
        {
            LOG_F(ERROR, "Config manager not initialized");
        }
        return true;
    }

    bool DeviceManager::addDeviceLibrary(const std::string &lib_path, const std::string &lib_name)
    {
        if (lib_path.empty() || lib_name.empty())
        {
            LOG_F(ERROR, "Library path and name is required!");
            return false;
        }
        if (!m_ModuleLoader->LoadModule(lib_path, lib_name))
        {
            LOG_F(ERROR, "Failed to load device library : {} in {}", lib_name.c_str(), lib_path.c_str());
            return false;
        }
        return true;
    }

    bool DeviceManager::AddDeviceObserver(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto &devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it)
        {
            if (*it && (*it)->getDeviceName() == name)
            {
                (*it)->addObserver([this](const std::any &message)
                                   { 
                                    if(message.has_value())
                                    {
                                        try
                                        {
                                            if (message.type() == typeid(std::shared_ptr<IStringProperty>))
                                            {
                                                messageBusPublishString(std::any_cast<std::shared_ptr<IStringProperty>>(message));
                                            }
                                            else if(message.type() == typeid(std::shared_ptr<INumberProperty>))
                                            {
                                                messageBusPublishNumber(std::any_cast<std::shared_ptr<INumberProperty>>(message));
                                            }
                                            else if (message.type() == typeid(std::shared_ptr<IBoolProperty>))
                                            {
                                                messageBusPublishBool(std::any_cast<std::shared_ptr<IBoolProperty>>(message));
                                            }
                                            else
                                            {
                                                LOG_F(ERROR,"Unknown property type!");
                                            }
                                        }
                                        catch(const std::bad_any_cast &e)
                                        {
                                            LOG_F(ERROR,"Failed to cast property {}",e.what());
                                        }
                                    } });
                LOG_F(INFO, "Add device {} observer successfully", name.c_str());

                return true;
            }
        }
        LOG_F(ERROR, "Could not find device {} of type %d", name.c_str(), static_cast<int>(type));
        return false;
    }

    bool DeviceManager::removeDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto &devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it)
        {
            if (*it && (*it)->getDeviceName() == name)
            {
                (*it)->getTask("disconnect", {});
                devices.erase(it);
                LOG_F(INFO, "Remove device {} successfully", name.c_str());
                if (m_ConfigManager)
                {
#ifdef __cpp_lib_format
                    m_ConfigManager->deleteValue(std::format("driver/{}", name));
#else
                    m_ConfigManager->deleteValue(fmt::format("driver/{}", name));
#endif
                }
                else
                {
                    LOG_F(ERROR, "Config manager not initialized");
                }
                return true;
            }
        }
        LOG_F(ERROR, "Could not find device {} of type %d", name.c_str(), static_cast<int>(type));
        return false;
    }

    bool DeviceManager::removeDevicesByName(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto &devices : m_devices)
        {
            devices.erase(std::remove_if(devices.begin(), devices.end(),
                                         [&](const std::shared_ptr<Device> &device)
                                         { return device && device->getDeviceName() == name; }),
                          devices.end());
        }
        if (m_ConfigManager)
        {
#ifdef __cpp_lib_format
            m_ConfigManager->deleteValue(std::format("driver/{}", name));
#else
            m_ConfigManager->deleteValue(fmt::format("driver/{}", name));
#endif
        }
        else
        {
            LOG_F(ERROR, "Config manager not initialized");
        }
        return true;
    }

    bool DeviceManager::removeDeviceLibrary(const std::string &lib_name)
    {
        if (lib_name.empty())
        {
            LOG_F(ERROR, "Library name is required");
            return false;
        }
        if (!m_ModuleLoader->UnloadModule(lib_name))
        {
            LOG_F(ERROR, "Failed to remove device library : {} with unload error", lib_name.c_str());
            return false;
        }
        return true;
    }

    std::shared_ptr<Device> DeviceManager::getDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t index = findDevice(type, name);
        if (index != -1)
        {
            return m_devices[static_cast<int>(type)][index];
        }
        else
        {
            LOG_F(WARNING, "Could not find device {} of type %d", name.c_str(), static_cast<int>(type));
            return nullptr;
        }
    }

    size_t DeviceManager::findDevice(DeviceType type, const std::string &name)
    {
        auto &devices = m_devices[static_cast<int>(type)];
        for (size_t i = 0; i < devices.size(); ++i)
        {
            if (devices[i] && devices[i]->getDeviceName() == name)
            {
                return i;
            }
        }
        return -1;
    }

    std::shared_ptr<Device> DeviceManager::findDeviceByName(const std::string &name) const
    {
        for (const auto &devices : m_devices)
        {
            for (const auto &device : devices)
            {
                if (device && device->getDeviceName() == name)
                {
                    return device;
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<SimpleTask> DeviceManager::getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        LOG_F(INFO, "Trying to find {} and get {} task", device_name.c_str(), task_name.c_str());
        auto device = findDeviceByName(device_name);

        if (device != nullptr)
        {
            switch (type)
            {
            case DeviceType::Camera:
            {
                LOG_F(INFO, "Found Camera device: {} with task: {}", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Camera>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Telescope:
            {
                LOG_F(INFO, "Found Telescope device: {} with driver: {}", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Telescope>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Focuser:
            {
                LOG_F(INFO, "Found Focuser device: {} with driver: {}", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Focuser>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::FilterWheel:
            {
                LOG_F(INFO, "Found FilterWheel device: {} with driver: {}", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Filterwheel>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Solver:
            {
                break;
            }
            case DeviceType::Guider:
            {
                break;
            }
            default:
                LOG_F(ERROR, "Invalid device type");
                break;
            }
        }
        else
        {
            LOG_F(INFO, "Device {} not found", device_name.c_str());
        }
        return nullptr;
    }

    void DeviceManager::messageBusPublishString(const std::shared_ptr<IStringProperty> &message)
    {
        if (m_MessageBus)
        {
            m_MessageBus->Publish<std::shared_ptr<IStringProperty>>("main", message);
        }
        if (!m_ConfigManager)
        {
            LOG_F(ERROR, "Config manager not initialized");
        }
        else
        {
            if (!message->value.empty())
            {
#ifdef __cpp_lib_format
                m_ConfigManager->setValue(std::format("driver/{}/{}", message->device_name, message->name), message->value);
#else
                m_ConfigManager->setValue(fmt::format("driver/{}/{}", message->device_name, message->name), message->value);
#endif
            }
        }
    }

    void DeviceManager::messageBusPublishNumber(const std::shared_ptr<INumberProperty> &message)
    {
        if (m_MessageBus)
        {
            m_MessageBus->Publish<std::shared_ptr<INumberProperty>>("main", message);
        }
        if (!m_ConfigManager)
        {
            LOG_F(ERROR, "Config manager not initialized");
        }
        else
        {
#ifdef __cpp_lib_format
            m_ConfigManager->setValue(std::format("driver/{}/{}", message->device_name, message->name), message->value);
#else
            m_ConfigManager->setValue(fmt::format("driver/{}/{}", message->device_name, message->name), message->value);
#endif
        }
    }

    void DeviceManager::messageBusPublishBool(const std::shared_ptr<IBoolProperty> &message)
    {
        if (m_MessageBus)
        {
            m_MessageBus->Publish<std::shared_ptr<IBoolProperty>>("main", message);
        }
        if (!m_ConfigManager)
        {
            LOG_F(ERROR, "Config manager not initialized");
        }
        else
        {
#ifdef __cpp_lib_format
            m_ConfigManager->setValue(std::format("driver/{}/{}", message->device_name, message->name), message->value);
#else
            m_ConfigManager->setValue(fmt::format("driver/{}/{}", message->device_name, message->name), message->value);
#endif
        }
    }

    bool DeviceManager::setDeviceProperty(DeviceType type, const std::string &name, const std::string &value_name, const std::any &value)
    {
        m_ThreadManager->addThread([this, &value, &type, &name, &value_name]()
                                   {
                                 auto device = getDevice(type, name);
                                 if (!device)
                                 {
                                     LOG_F(ERROR, "{} not found",name.c_str());
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name.c_str(), name.c_str(), e.what());
                                 } },
                                   m_ThreadManager->generateRandomString(16));
        return true;
    }

    bool DeviceManager::setDevicePropertyByName(const std::string &name, const std::string &value_name, const std::any &value)
    {
        m_ThreadManager->addThread([this, &value, &name, &value_name]()
                                   {
                                 auto device = findDeviceByName(name);
                                 if (!device)
                                 {
                                     LOG_F(ERROR, "{} not found",name.c_str());
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name.c_str(), name.c_str(), e.what());
                                 } },
                                   m_ThreadManager->generateRandomString(16));
        return true;
    }

    bool DeviceManager::setMainCamera(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_main_camera = std::dynamic_pointer_cast<Camera>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set main camera to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    bool DeviceManager::setGuidingCamera(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_guiding_camera = std::dynamic_pointer_cast<Camera>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set main camera to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    bool DeviceManager::setTelescope(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_telescope = std::dynamic_pointer_cast<Telescope>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set telescope to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    bool DeviceManager::setFocuser(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_focuser = std::dynamic_pointer_cast<Focuser>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set focuser to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    bool DeviceManager::setFilterwheel(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_filterwheel = std::dynamic_pointer_cast<Filterwheel>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set filterwheel to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    bool DeviceManager::setGuider(const std::string &name)
    {
        if (name.empty())
            return false;
        if (findDeviceByName(name))
        {
            try
            {
                m_guider = std::dynamic_pointer_cast<Guider>(findDeviceByName(name));
            }
            catch (const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set guider to: {} with {}", name.c_str(), e.what());
                return false;
            }
        }
        return true;
    }

    DEVICE_FUNC(DeviceManager::startExposure)
    {
        CHECK_MAIN_CAMERA;
        if (m_main_camera->getExposureStatus({}))
        {
            LOG_F(WARNING, "Main camera is exposed, please do not restart it again!");
            return DeviceError::Busy;
        }
        if (!m_params.contains("exposure"))
        {
            LOG_F(ERROR, "Missing exposure time.");
            return DeviceError::MissingValue;
        }
        // 必须先指定是提前设置才会触发，不然所有功能均在startExposure中完成
        if (m_params.contains("preset"))
        {
            if (m_params["preset"].get<bool>())
            {
                if (m_params.contains("gain"))
                {
                    setGain({"gain", m_params["gain"]});
                }
                if (m_params.contains("offset"))
                {
                    setOffset({"offset", m_params["offset"]});
                }
                if (m_params.contains("iso"))
                {
                    setISO({"iso", m_params["iso"]});
                }
            }
        }
        if (!m_main_camera->startExposure(m_params))
        {
            LOG_F(ERROR, "{} failed to start exposure", m_main_camera->getDeviceName());
            return DeviceError::ExposureError;
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopExposure)
    {
        CHECK_MAIN_CAMERA;
        if (!m_main_camera->getExposureStatus({}))
        {
            // TODO: 这里需要一个错误返回吗？
            LOG_F(WARNING, "{} is not exposed", m_main_camera->getDeviceName());
        }
        else
        {
            if (!m_main_camera->abortExposure(m_params))
            {
                LOG_F(ERROR, "{} failed to stop exposure", m_main_camera->getDeviceName());
                return DeviceError::ExposureError;
            }
            LOG_F(INFO, "{} is aborted successfully", m_main_camera->getDeviceName());
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::startCooling)
    {
        CHECK_MAIN_CAMERA;
        if (!m_main_camera->isCoolingAvailable())
        {
            LOG_F(ERROR, "{} did not support cooling mode", m_main_camera->getDeviceName());
            return DeviceError::NotSupported;
        }
        // TODO: 这里是否需要一个温度的检查，或者说是在启动制冷时是否需要指定问温度
        if (!m_main_camera->startCooling(m_params))
        {
            LOG_F(ERROR, "{} failed to start cooling mode", m_main_camera->getDeviceName());
            return DeviceError::CoolingError;
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopCooling)
    {
        CHECK_MAIN_CAMERA;
        if (!m_main_camera->isCoolingAvailable())
        {
            LOG_F(ERROR, "{} did not support cooling mode", m_main_camera->getDeviceName());
            return DeviceError::NotSupported;
        }
        if (!m_main_camera->stopCooling(m_params))
        {
            LOG_F(ERROR, "{} failed to stop cooling mode", m_main_camera->getDeviceName());
            return DeviceError::CoolingError;
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::setGain)
    {
        CHECK_MAIN_CAMERA;
        if (!m_params.contains("gain"))
        {
            LOG_F(ERROR, "Failed to set gain: No gain value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isGainAvailable())
            {
                LOG_F(WARNING, "{} did not support set gain", m_main_camera->getDeviceName());
                return DeviceError::NotSupported;
            }
            else
            {
                int value = m_params["gain"].get<int>();
                if (value < 0 || value > 100)
                {
                    LOG_F(ERROR, "Invalid gain value {}, would not set", value);
                    return DeviceError::InvalidValue;
                }
                else
                {
                    if (!m_main_camera->setGain({"gain", value}))
                    {
                        LOG_F(ERROR, "Failed to set gain of main camera {}", m_main_camera->getDeviceName());
                        return DeviceError::GainError;
                    }
                }
            }
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::setOffset)
    {
        CHECK_MAIN_CAMERA;
        if (!m_params.contains("offset"))
        {
            LOG_F(ERROR, "Failed to set offset: No offset value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isOffsetAvailable())
            {
                LOG_F(WARNING, "{} did not support set offset", m_main_camera->getDeviceName());
                return DeviceError::NotSupported;
            }
            else
            {
                int value = m_params["offset"].get<int>();
                if (value < 0 || value > 255)
                {
                    LOG_F(ERROR, "Invalid offset value {}, would not set", value);
                    return DeviceError::InvalidValue;
                }
                else
                {
                    if (!m_main_camera->setOffset({"offset", value}))
                    {
                        LOG_F(ERROR, "Failed to set offset of main camera {}", m_main_camera->getDeviceName());
                        return DeviceError::OffsetError;
                    }
                }
            }
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::setISO)
    {
        CHECK_MAIN_CAMERA;
        if (!m_params.contains("iso"))
        {
            LOG_F(ERROR, "Failed to set iso: No iso value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isISOAvailable())
            {
                LOG_F(WARNING, "{} did not support set iso", m_main_camera->getDeviceName());
                return DeviceError::NotSupported;
            }
            else
            {
                int value = m_params["iso"].get<int>();
                // TODO: There needs a ISO value check
                if (!m_main_camera->setISO({"iso", value}))
                {
                    LOG_F(ERROR, "Failed to set iso of main camera {}", m_main_camera->getDeviceName());
                    return DeviceError::ISOError;
                }
            }
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::setCamareParams)
    {
        CHECK_MAIN_CAMERA;
        /*
        [
            {
                "name": "gain",
                "value": 30
            },
            {
                "name": "offset",
                "age": 25
            }
        ]
        or
        {
            "gain" : 30,
            "offset" : 25
        }
        */
        if (m_params.is_array())
        {
            for (auto &params : m_params)
            {
                for (auto it = params.begin(); it != params.end(); ++it)
                {
                    m_main_camera->setProperty(it.key(), it.value());
                }
            }
        }
        else
        {
            for (auto it = m_params.begin(); it != m_params.end(); ++it)
            {
                m_main_camera->setProperty(it.key(), it.value());
            }
        }
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getCameraParams)
    {
        /*
        ["gain","offset","iso"]
        or
        "name" : "gain"
        */
        CHECK_MAIN_CAMERA_J;
        json res;
        if (m_params.is_array())
        {
            for (auto it = m_params.begin(); it != m_params.end(); ++it)
            {
                res[it.key()] = m_main_camera->getStringProperty(it.key())->value;
            }
        }
        else
        {
            res["value"] = m_main_camera->getStringProperty(m_params["name"])->value;
        }
        return res;
    }

    // For telescope
    DEVICE_FUNC(DeviceManager::gotoTarget)
    {
        CHECK_TELESCOPE;
        if (!m_params.contains("ra") || !m_params.contains("dec"))
        {
            LOG_F(ERROR, "{} failed to goto: Missing RA or DEC value", m_telescope->getDeviceName());
            return DeviceError::MissingValue;
        }
        std::string ra = m_params["ra"];
        std::string dec = m_params["dec"];
        if (ra.empty() || dec.empty())
        {
            LOG_F(ERROR, "RA or DEC value is missing");
            return DeviceError::MissingValue;
        }
        try
        {
            if (checkDigits(ra))
            {
                ra = convertToTimeFormat(std::stoi(ra));
            }
            if (!checkTimeFormat(ra))
            {
                LOG_F(ERROR, "Error Format of RA value {}", ra);
                return DeviceError::InvalidValue;
            }
            if (checkDigits(dec))
            {
                dec = convertToTimeFormat(std::stoi(dec));
            }
            if (!checkTimeFormat(dec))
            {
                LOG_F(ERROR, "Error Format of DEC value {}", ra);
                return DeviceError::InvalidValue;
            }
        }
        catch (const std::out_of_range &e)
        {
            LOG_F(ERROR, "Failed to check RA and DEC value: {}", e.what());
            return DeviceError::InvalidValue;
        }
        if (!m_telescope->SlewTo(m_params))
        {
            LOG_F(ERROR, "{} failed to slew to {} {}", m_telescope->getDeviceName(), ra, dec);
            return DeviceError::GotoError;
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::park)
    {
        CHECK_TELESCOPE;
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::unpark)
    {
        CHECK_TELESCOPE;
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::goHome)
    {
        CHECK_TELESCOPE;
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::sync)
    {
        CHECK_TELESCOPE;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getCroods)
    {
        CHECK_TELESCOPE_J;
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getObserver)
    {
        CHECK_TELESCOPE_J;
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getTime)
    {
        CHECK_TELESCOPE_J;
        return {};
    }

    DEVICE_FUNC(DeviceManager::setTelescopeParams)
    {
        CHECK_TELESCOPE;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getTelescopeParams)
    {
        CHECK_TELESCOPE_J;
        return {};
    }

    // For focuser
    DEVICE_FUNC(DeviceManager::moveStep)
    {
        CHECK_FOCUSER;
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::moveTo)
    {
        CHECK_FOCUSER;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getTemperatrue)
    {
        CHECK_FOCUSER_J;
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getFocuserPosition)
    {
        CHECK_FOCUSER_J;
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getBacklash)
    {
        CHECK_FOCUSER_J;
        return {};
    }

    DEVICE_FUNC(DeviceManager::setFocuserParams)
    {
        CHECK_FOCUSER;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFocuserParams)
    {
        CHECK_FOCUSER_J;
        return {};
    }

    // For filterwheel
    DEVICE_FUNC(DeviceManager::slewTo)
    {
        CHECK_FILTERWHEEL;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFilterwheelPosition)
    {
        CHECK_FILTERWHEEL_J;
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getFilters)
    {
        CHECK_FILTERWHEEL_J;
        return {};
    }
    DEVICE_FUNC_J(DeviceManager::getOffsets)
    {
        CHECK_FILTERWHEEL_J;
        return {};
    }

    DEVICE_FUNC(DeviceManager::setFilterwheelParams)
    {
        CHECK_FILTERWHEEL;
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFilterwheelParams)
    {
        CHECK_FILTERWHEEL_J;
        return {};
    }

    // For guider
    DEVICE_FUNC(DeviceManager::startGuiding)
    {
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopGuiding)
    {
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::startCalibration)
    {
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopCalibration)
    {
        return DeviceError::None;
    }

    // For astrometry and astap
    DEVICE_FUNC_J(DeviceManager::solveImage)
    {
        return DeviceError::None;
    }

    bool DeviceManager::startINDIServer()
    {
        if (!m_indimanager->is_running())
        {
            m_indimanager->start_server();
        }
        return true;
    }

    bool DeviceManager::stopINDIServer()
    {
        if (m_indimanager->is_running())
        {
            m_indimanager->stop_server();
        }
        return true;
    }

    bool DeviceManager::startINDIDevice()
    {
        if (!m_indimanager->is_running())
        {
            LOG_F(ERROR, "INDI server is not started(not by lithium server)");
            return false;
        }
        return true;
    }

    bool DeviceManager::stopINDIDevice()
    {
        return true;
    }

    bool DeviceManager::startASCOMServer()
    {
        return true;
    }

    bool DeviceManager::stopASCOMServer()
    {
        return true;
    }

    bool DeviceManager::startASCOMDevice()
    {
        return true;
    }

    bool DeviceManager::stopASCOMDevice()
    {
        return true;
    }
}
