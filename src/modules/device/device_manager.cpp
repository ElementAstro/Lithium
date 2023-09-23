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
        LOG_F(ERROR, "Main camera not specified on calling %s", __func__); \
        return DeviceError::NotSpecific;                                   \
    }

#define CHECK_TELESCOPE                                                       \
    if (!m_telescope)                                                         \
    {                                                                         \
        LOG_F(ERROR, "Main telescope not specified on calling %s", __func__); \
        return DeviceError::NotSpecific;                                      \
    }

#define CHECK_FOCUSER                                                       \
    if (!m_focuser)                                                         \
    {                                                                       \
        LOG_F(ERROR, "Main focuser not specified on calling %s", __func__); \
        return DeviceError::NotSpecific;                                    \
    }

#define CHECK_FILTERWHEEL                                                       \
    if (!m_filterwheel)                                                         \
    {                                                                           \
        LOG_F(ERROR, "Main filterwheel not specified on calling %s", __func__); \
        return DeviceError::NotSpecific;                                        \
    }

#define CHECK_GUIDER                                                       \
    if (!m_guider)                                                         \
    {                                                                      \
        LOG_F(ERROR, "Main guider not specified on calling %s", __func__); \
        return DeviceError::NotSpecific;                                   \
    }

// For DEVICE_FUNC_J

#define CHECK_MAIN_CAMERA_J                                                \
    if (!m_main_camera)                                                    \
    {                                                                      \
        LOG_F(ERROR, "Main camera not specified on calling %s", __func__); \
        return {{"error", "no camera specified"}};                         \
    }

#define CHECK_TELESCOPE_J                                                     \
    if (!m_telescope)                                                         \
    {                                                                         \
        LOG_F(ERROR, "Main telescope not specified on calling %s", __func__); \
        return {{"error", "no telescope specified"}};                         \
    }

#define CHECK_FOCUSER_J                                                     \
    if (!m_focuser)                                                         \
    {                                                                       \
        LOG_F(ERROR, "Main focuser not specified on calling %s", __func__); \
        return {{"error", "no focuser specified"}};                         \
    }

#define CHECK_FILTERWHEEL_J                                                     \
    if (!m_filterwheel)                                                         \
    {                                                                           \
        LOG_F(ERROR, "Main filterwheel not specified on calling %s", __func__); \
        return {{"error", "no filterwheel specified"}};                         \
    }

#define CHECK_GUIDER_J                                                     \
    if (!m_guider)                                                         \
    {                                                                      \
        LOG_F(ERROR, "Main guider not specified on calling %s", __func__); \
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
                deviceList.emplace_back(device->getStringProperty("name")->value);
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
            LOG_F(ERROR, "A device with name %s already exists, please choose a different name", name.c_str());
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
                    LOG_F(INFO, "Trying to add a new camera instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Camera>(newName));
                    LOG_F(INFO, "Added new camera %s instance successfully", newName.c_str());
                    break;
                }
                case DeviceType::Telescope:
                {
                    LOG_F(INFO, "Trying to add a new telescope instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                    LOG_F(INFO, "Added new telescope instance successfully");
                    break;
                }
                case DeviceType::Focuser:
                {
                    LOG_F(INFO, "Trying to add a new Focuser instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                    LOG_F(INFO, "Added new focuser instance successfully");
                    break;
                }
                case DeviceType::FilterWheel:
                {
                    LOG_F(INFO, "Trying to add a new filterwheel instance : %s", newName.c_str());
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
                    LOG_F(INFO, "Trying to add a new camera instance : %s from %s", newName.c_str(), lib_name.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(m_ModuleLoader->GetInstance<Camera>(lib_name, params, "GetInstance"));
                    LOG_F(INFO, "Added new camera %s instance successfully", newName.c_str());
                    break;
                }
                case DeviceType::Telescope:
                {
                    LOG_F(INFO, "Trying to add a new telescope instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                    LOG_F(INFO, "Added new telescope instance successfully");
                    break;
                }
                case DeviceType::Focuser:
                {
                    LOG_F(INFO, "Trying to add a new Focuser instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                    LOG_F(INFO, "Added new focuser instance successfully");
                    break;
                }
                case DeviceType::FilterWheel:
                {
                    LOG_F(INFO, "Trying to add a new filterwheel instance : %s", newName.c_str());
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Filterwheel>(newName));
                    LOG_F(INFO, "Added new filterwheel instance successfully");
                    break;
                }
                case DeviceType::Solver:
                {
                    LOG_F(INFO, "Trying to add a new solver instance : %s from %s", newName.c_str(), lib_name.c_str());
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
            LOG_F(ERROR, "Failed to add device %s , error : %s", newName.c_str(), e.what());
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
            LOG_F(ERROR, "Failed to load device library : %s in %s", lib_name.c_str(), lib_path.c_str());
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
            if (*it && (*it)->getStringProperty("name")->value == name)
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
                                            LOG_F(ERROR,"Failed to cast property %s",e.what());
                                        }
                                    } });
                LOG_F(INFO, "Add device %s observer successfully", name.c_str());

                return true;
            }
        }
        LOG_F(ERROR, "Could not find device %s of type %d", name.c_str(), static_cast<int>(type));
        return false;
    }

    bool DeviceManager::removeDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto &devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it)
        {
            if (*it && (*it)->getStringProperty("name")->value == name)
            {
                (*it)->getTask("disconnect", {});
                devices.erase(it);
                LOG_F(INFO, "Remove device %s successfully", name.c_str());
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
        LOG_F(ERROR, "Could not find device %s of type %d", name.c_str(), static_cast<int>(type));
        return false;
    }

    bool DeviceManager::removeDevicesByName(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto &devices : m_devices)
        {
            devices.erase(std::remove_if(devices.begin(), devices.end(),
                                         [&](const std::shared_ptr<Device> &device)
                                         { return device && device->getStringProperty("name")->value == name; }),
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
            LOG_F(ERROR, "Failed to remove device library : %s with unload error", lib_name.c_str());
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
            LOG_F(WARNING, "Could not find device %s of type %d", name.c_str(), static_cast<int>(type));
            return nullptr;
        }
    }

    size_t DeviceManager::findDevice(DeviceType type, const std::string &name)
    {
        auto &devices = m_devices[static_cast<int>(type)];
        for (size_t i = 0; i < devices.size(); ++i)
        {
            if (devices[i] && devices[i]->getStringProperty("name")->value == name)
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
                if (device && device->getStringProperty("name")->value == name)
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
        LOG_F(INFO, "Trying to find %s and get %s task", device_name.c_str(), task_name.c_str());
        auto device = findDeviceByName(device_name);

        if (device != nullptr)
        {
            switch (type)
            {
            case DeviceType::Camera:
            {
                LOG_F(INFO, "Found Camera device: %s with task: %s", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Camera>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Telescope:
            {
                LOG_F(INFO, "Found Telescope device: %s with driver: %s", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Telescope>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Focuser:
            {
                LOG_F(INFO, "Found Focuser device: %s with driver: %s", device_name.c_str(), task_name.c_str());
                return std::dynamic_pointer_cast<Focuser>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::FilterWheel:
            {
                LOG_F(INFO, "Found FilterWheel device: %s with driver: %s", device_name.c_str(), task_name.c_str());
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
            LOG_F(INFO, "Device %s not found", device_name.c_str());
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
                                     LOG_F(ERROR, "%s not found",name.c_str());
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert %s of %s with %s", value_name.c_str(), name.c_str(), e.what());
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
                                     LOG_F(ERROR, "%s not found",name.c_str());
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert %s of %s with %s", value_name.c_str(), name.c_str(), e.what());
                                 } },
                                   m_ThreadManager->generateRandomString(16));
        return true;
    }

    bool DeviceManager::setMainCamera(const std::string &name)
    {
        if (name.empty())
            return false;

        if (m_main_camera)
        {
            LOG_F(INFO, "Current main camera: %s", m_main_camera->getStringProperty("name")->value.c_str());
        }
        LOG_F(INFO, "Try to set main camera to: %s", name.c_str());
        if (findDeviceByName(name))
        {
            try
            {
                m_main_camera.reset(std::dynamic_pointer_cast<Camera>(findDeviceByName(name)));
            }
            catch(const std::bad_alloc &e)
            {
                LOG_F(ERROR, "Failed to set main camera to: %s with %s", name.c_str(),e.what());
                return false;
            }
        }
        return true;
    }
    bool DeviceManager::setGuidingCamera(const std::string &name)
    {
    }
    bool DeviceManager::setTelescope(const std::string &name)
    {
    }
    bool DeviceManager::setFocuser(const std::string &name)
    {
    }
    bool DeviceManager::setFilterwheel(const std::string &name)
    {
    }
    bool DeviceManager::setGuider(const std::string &name)
    {
    }

    DEVICE_FUNC(DeviceManager::startExposure)
    {
        CHECK_MAIN_CAMERA;
    }
    DEVICE_FUNC(DeviceManager::stopExposure)
    {
        CHECK_MAIN_CAMERA;
    }
    DEVICE_FUNC(DeviceManager::startCooling)
    {
        CHECK_MAIN_CAMERA;
    }
    DEVICE_FUNC(DeviceManager::stopCooling)
    {
        CHECK_MAIN_CAMERA;
    }
    DEVICE_FUNC(DeviceManager::setCamareParams)
    {
        CHECK_MAIN_CAMERA;
    }
    DEVICE_FUNC_J(DeviceManager::getCameraParams)
    {
        CHECK_MAIN_CAMERA_J;
    }

    // For telescope
    DEVICE_FUNC(DeviceManager::gotoTarget)
    {
        CHECK_TELESCOPE;
    }
    DEVICE_FUNC(DeviceManager::park)
    {
        CHECK_TELESCOPE;
    }
    DEVICE_FUNC(DeviceManager::unpark)
    {
        CHECK_TELESCOPE;
    }
    DEVICE_FUNC(DeviceManager::goHome)
    {
        CHECK_TELESCOPE;
    }
    DEVICE_FUNC(DeviceManager::sync)
    {
        CHECK_TELESCOPE;
    }
    DEVICE_FUNC_J(DeviceManager::getCroods)
    {
        CHECK_TELESCOPE_J;
    }
    DEVICE_FUNC_J(DeviceManager::getObserver) { CHECK_TELESCOPE_J; }
    DEVICE_FUNC_J(DeviceManager::getTime) { CHECK_TELESCOPE_J; }
    DEVICE_FUNC(DeviceManager::setTelescopeParams) { CHECK_TELESCOPE; }
    DEVICE_FUNC_J(DeviceManager::getTelescopeParams) { CHECK_TELESCOPE_J; }

    // For focuser
    DEVICE_FUNC(DeviceManager::moveStep)
    {
        CHECK_FOCUSER;
    }
    DEVICE_FUNC(DeviceManager::moveTo)
    {
        CHECK_FOCUSER;
    }
    DEVICE_FUNC_J(DeviceManager::getTemperatrue)
    {
        CHECK_FOCUSER_J;
    }
    DEVICE_FUNC_J(DeviceManager::getFocuserPosition)
    {
        CHECK_FOCUSER_J;
    }
    DEVICE_FUNC_J(DeviceManager::getBacklash)
    {
        CHECK_FOCUSER_J;
    }
    DEVICE_FUNC(DeviceManager::setFocuserParams)
    {
        CHECK_FOCUSER;
    }
    DEVICE_FUNC_J(DeviceManager::getFocuserParams)
    {
        CHECK_FOCUSER_J;
    }

    // For filterwheel
    DEVICE_FUNC(DeviceManager::slewTo)
    {
        CHECK_FILTERWHEEL;
    }
    DEVICE_FUNC_J(DeviceManager::getFilterwheelPosition)

    {
        CHECK_FILTERWHEEL_J;
    }
    DEVICE_FUNC_J(DeviceManager::getFilters)
    {
        CHECK_FILTERWHEEL_J;
    }
    DEVICE_FUNC_J(DeviceManager::getOffsets)
    {
        CHECK_FILTERWHEEL_J;
    }
    DEVICE_FUNC(DeviceManager::setFilterwheelParams)
    {
        CHECK_FILTERWHEEL;
    }
    DEVICE_FUNC_J(DeviceManager::getFilterwheelParams)
    {
        CHECK_FILTERWHEEL_J;
    }

    // For guider
    DEVICE_FUNC(DeviceManager::startGuiding)
    {

    }
    DEVICE_FUNC(DeviceManager::stopGuiding)
    {

    }
    DEVICE_FUNC(DeviceManager::startCalibration)
    {

    }
    DEVICE_FUNC(DeviceManager::stopCalibration)
    {

    }

    // For astrometry and astap
    DEVICE_FUNC_J(DeviceManager::solveImage)
    {

    }

    bool DeviceManager::startINDIServer()
    {
        if (!m_indimanager->is_running())
        {
            return m_indimanager->start_server();
        }
        return true;
    }

        bool DeviceManager::stopINDIServer()
        {
if (m_indimanager->is_running())
        {
            return m_indimanager->stop_server();
        }
        return true;
        }
        bool DeviceManager::startINDIDevice()
        {
            if (!m_indimanager->is_running())
            {
                LOG_F(ERROR,"INDI server is not started(not by lithium server)");
                return false;
            }
            return true;
        }
        bool DeviceManager::stopINDIDevice()
        {

        }

        bool DeviceManager::startASCOMServer()
        {

        }
        bool DeviceManager::stopASCOMServer()
        {

        }
        bool DeviceManager::startASCOMDevice()
        {

        }
        bool DeviceManager::stopASCOMDevice(){

        }
}
