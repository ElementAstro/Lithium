/*
 * manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Device Manager

**************************************************/

#include "manager.hpp"
#include "atom/server/global_ptr.hpp"

#include "atom/driver/camera.hpp"
#include "atom/driver/telescope.hpp"
#include "atom/driver/focuser.hpp"
#include "atom/driver/filterwheel.hpp"
#include "atom/driver/solver.hpp"
#include "atom/driver/guider.hpp"
#include "atom/driver/device_type.hpp"

#include "atom/driver/device_exception.hpp"
#include "atom/driver/camera_utils.hpp"
#include "utils/utils.hpp"
#include "atom/utils/random.hpp"

#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#include <typeinfo>

#include "config.h"

#include "atom/log/loguru.hpp"
#include "magic_enum/magic_enum.hpp"

// For DEVICE_FUNC

#define CHECK_DEVICE(device)                                    \
    do                                                          \
    {                                                           \
        if (!(device))                                          \
        {                                                       \
            LOG_F(ERROR, "Main {} not specified on calling {}", \
                  #device, __func__);                           \
            return DeviceError::NotSpecific;                    \
        }                                                       \
    } while (false)

// For DEVICE_FUNC_J

#define CHECK_DEVICE_J(device, error_message)                   \
    do                                                          \
    {                                                           \
        if (!(device))                                          \
        {                                                       \
            LOG_F(ERROR, "Main {} not specified on calling {}", \
                  #device, __func__);                           \
            return {{"error", error_message}};                  \
        }                                                       \
    } while (false)

#define CHECK_CONNECTED(device)                              \
    do                                                       \
    {                                                        \
        if (!(device)->isConnected())                        \
        {                                                    \
            LOG_F(ERROR, "{} is not connected when call {}", \
                  (device)->getDeviceName(), __func__);      \
            return DeviceError::NotConnected;                \
        }                                                    \
    } while (false)

namespace Lithium
{

    // Constructor
    DeviceManager::DeviceManager(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager)
    {
        m_ModuleLoader = ModuleLoader::createShared("drivers", GetPtr<Atom::Async::ThreadManager>("lithium.async.thread"));
        m_ConfigManager = configManager;
        m_MessageBus = messageBus;
        for (auto &devices : m_devices)
        {
            devices.emplace_back();
        }

        m_hydrogenmanager = std::make_shared<HydrogenManager>();
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

    std::shared_ptr<DeviceManager> DeviceManager::createShared(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager)
    {
        return std::make_shared<DeviceManager>(messageBus, configManager);
    }

    std::unique_ptr<DeviceManager> DeviceManager::createUnique(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager)
    {
        return std::make_unique<DeviceManager>(messageBus, configManager);
    }

    void DeviceManager::connectToMessageBus()
    {
        m_MessageBus->Subscribe<std::shared_ptr<Message>>("device", [this](std::shared_ptr<Message> message) -> void
                                                          {
            switch (message->type())
            {
                case Message::Type::kNumber:
                {
                    std::shared_ptr<NumberMessage> numberMessage = std::dynamic_pointer_cast<NumberMessage>(message);

                    break;
                }
                case Message::Type::kText:
                {
                    std::shared_ptr<TextMessage> stringMessage = std::dynamic_pointer_cast<TextMessage>(message);
                    break;
                }
                case Message::Type::kBoolean:
                {
                    std::shared_ptr<BooleanMessage> booleanMessage = std::dynamic_pointer_cast<BooleanMessage>(message);
                    break;
                }
                default:
                {
                    LOG_F(ERROR, "Unknown message type {}", magic_enum::enum_name(message->type()));
                    break;
                }
            } });
    }

    std::vector<std::string> DeviceManager::getDeviceList()
    {
        std::vector<std::string> deviceList;
        std::initializer_list<DeviceType> devices = {DeviceType::Camera, DeviceType::Focuser, DeviceType::Telescope, DeviceType::Guider, DeviceType::Solver, DeviceType::FilterWheel};
        for (auto &device_type : devices)
        {
            auto &devices = m_devices[static_cast<int>(device_type)];
            for (const auto &device : devices)
            {
                if (device)
                {
                    deviceList.emplace_back(device->getDeviceName());
                }
            }
        }
        return deviceList;
    }

    std::vector<std::string> DeviceManager::getDeviceListByType(DeviceType type)
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
            LOG_F(ERROR, "A device with name {} already exists, please choose a different name", name);
            return false;
        }
        std::string newName = name;
        int index = 1;
        while (static_cast<int>(findDevice(type, newName)) != -1)
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
                LOG_F(ERROR, "Please specific a device library when you want to create a new instance");
                return false;
            }
            else
            {
                json params;
                params["name"] = newName;
                std::string logMsg;

                switch (type)
                {
                case DeviceType::Camera:
                    logMsg = fmt::format("Trying to add a new camera instance : {} from {}", newName, lib_name);
                    m_devices[static_cast<int>(type)].emplace_back(m_ModuleLoader->GetInstance<Camera>(lib_name, params, "GetInstance"));
                    break;
                case DeviceType::Telescope:
                    logMsg = fmt::format("Trying to add a new telescope instance : {}", newName);
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                    break;
                case DeviceType::Focuser:
                    logMsg = fmt::format("Trying to add a new Focuser instance : {}", newName);
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                    break;
                case DeviceType::FilterWheel:
                    logMsg = fmt::format("Trying to add a new filterwheel instance : {}", newName);
                    m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Filterwheel>(newName));
                    break;
                case DeviceType::Solver:
                    logMsg = fmt::format("Trying to add a new solver instance : {} from {}", newName, lib_name);
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
                    break;
                case DeviceType::Guider:
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
                    break;
                default:
                    LOG_F(ERROR, "Invalid device type");
                    throw InvalidDeviceType("Invalid device type");
                }

                if (!logMsg.empty())
                {
                    DLOG_F(INFO, "{}", logMsg);
                }
                DLOG_F(INFO, "Added new {} instance successfully", magic_enum::enum_name(type));
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to add device {} , error : {}", newName, e.what());
            return false;
        }
        if (m_ConfigManager)
        {
#ifdef __cpp_lib_format
            m_ConfigManager->setValue(std::format("driver/{}/name", newName), newName);
#else
            m_ConfigManager->setValue(fmt::format("driver/{}/name", newName), newName);
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
            LOG_F(ERROR, "Failed to load device library : {} in {}", lib_name, lib_path);
            return false;
        }
        return true;
    }

    bool DeviceManager::addDeviceObserver(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto &devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it)
        {
            if (*it && (*it)->getDeviceName() == name)
            {
                /*
                (*it)->AddObserverFunc([this](const std::any &message)
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
                */
                DLOG_F(INFO, "Add device {} observer successfully", name);

                return true;
            }
        }
        LOG_F(ERROR, "Could not find device {} of type %d", name, static_cast<int>(type));
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
                DLOG_F(INFO, "Remove device {} successfully", name);
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
        LOG_F(ERROR, "Could not find device {} of type %d", name, static_cast<int>(type));
        return false;
    }

    bool DeviceManager::removeDeviceByName(const std::string &name)
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
            LOG_F(ERROR, "Failed to remove device library : {} with unload error", lib_name);
            return false;
        }
        return true;
    }

    std::shared_ptr<Device> DeviceManager::getDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t index = findDevice(type, name);
        if (static_cast<int>(index) != -1)
        {
            return m_devices[static_cast<int>(type)][index];
        }
        else
        {
            DLOG_F(WARNING, "Could not find device {} of type %d", name, static_cast<int>(type));
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

    std::shared_ptr<DeviceTask> DeviceManager::getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const json &params)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        DLOG_F(INFO, "Trying to find {} and get {} task", device_name, task_name);
        auto device = findDeviceByName(device_name);

        if (device != nullptr)
        {
            switch (type)
            {
            case DeviceType::Camera:
            {
                DLOG_F(INFO, "Found Camera device: {} with task: {}", device_name, task_name);
                return std::dynamic_pointer_cast<Camera>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Telescope:
            {
                DLOG_F(INFO, "Found Telescope device: {} with driver: {}", device_name, task_name);
                return std::dynamic_pointer_cast<Telescope>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::Focuser:
            {
                DLOG_F(INFO, "Found Focuser device: {} with driver: {}", device_name, task_name);
                return std::dynamic_pointer_cast<Focuser>(device)->getTask(task_name, params);
                break;
            }
            case DeviceType::FilterWheel:
            {
                DLOG_F(INFO, "Found FilterWheel device: {} with driver: {}", device_name, task_name);
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
            DLOG_F(INFO, "Device {} not found", device_name);
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
                                     LOG_F(ERROR, "{} not found",name);
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name, name, e.what());
                                 } },
                                   Atom::Utils::generateRandomString(16));
        return true;
    }

    bool DeviceManager::setDevicePropertyByName(const std::string &name, const std::string &value_name, const std::any &value)
    {
        m_ThreadManager->addThread([this, &value, &name, &value_name]()
                                   {
                                 auto device = findDeviceByName(name);
                                 if (!device)
                                 {
                                     LOG_F(ERROR, "{} not found",name);
                                     return;
                                 }
                                 try
                                 {
                                     device->setProperty(value_name,value);
                                 }
                                 catch (const std::bad_any_cast &e)
                                 {
                                     LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name, name, e.what());
                                 } },
                                   Atom::Utils::generateRandomString(16));
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
                LOG_F(ERROR, "Failed to set main camera to: {} with {}", name, e.what());
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
                LOG_F(ERROR, "Failed to set main camera to: {} with {}", name, e.what());
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
                LOG_F(ERROR, "Failed to set telescope to: {} with {}", name, e.what());
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
                LOG_F(ERROR, "Failed to set focuser to: {} with {}", name, e.what());
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
                LOG_F(ERROR, "Failed to set filterwheel to: {} with {}", name, e.what());
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
                LOG_F(ERROR, "Failed to set guider to: {} with {}", name, e.what());
                return false;
            }
        }
        return true;
    }

    DEVICE_FUNC(DeviceManager::startExposure)
    {
        CHECK_DEVICE(m_main_camera);
        if (m_main_camera->getExposureStatus({}))
        {
            DLOG_F(WARNING, "Main camera is exposed, please do not restart it again!");
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
        CHECK_DEVICE(m_main_camera);
        if (!m_main_camera->getExposureStatus({}))
        {
            // TODO: 这里需要一个错误返回吗？
            DLOG_F(WARNING, "{} is not exposed", m_main_camera->getDeviceName());
        }
        else
        {
            if (!m_main_camera->abortExposure(m_params))
            {
                LOG_F(ERROR, "{} failed to stop exposure", m_main_camera->getDeviceName());
                return DeviceError::ExposureError;
            }
            DLOG_F(INFO, "{} is aborted successfully", m_main_camera->getDeviceName());
        }
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::startCooling)
    {
        CHECK_DEVICE(m_main_camera);
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
        CHECK_DEVICE(m_main_camera);
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
        CHECK_DEVICE(m_main_camera);
        if (!m_params.contains("gain"))
        {
            LOG_F(ERROR, "Failed to set gain: No gain value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isGainAvailable())
            {
                DLOG_F(WARNING, "{} did not support set gain", m_main_camera->getDeviceName());
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
        CHECK_DEVICE(m_main_camera);
        if (!m_params.contains("offset"))
        {
            LOG_F(ERROR, "Failed to set offset: No offset value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isOffsetAvailable())
            {
                DLOG_F(WARNING, "{} did not support set offset", m_main_camera->getDeviceName());
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
        CHECK_DEVICE(m_main_camera);
        if (!m_params.contains("iso"))
        {
            LOG_F(ERROR, "Failed to set iso: No iso value provided");
            return DeviceError::MissingValue;
        }
        else
        {
            if (!m_main_camera->isISOAvailable())
            {
                DLOG_F(WARNING, "{} did not support set iso", m_main_camera->getDeviceName());
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
        CHECK_DEVICE(m_main_camera);
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
        CHECK_DEVICE_J(m_main_camera, "no main camera specific");
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
        CHECK_DEVICE(m_telescope);
        if (m_telescope->isAtPark({}))
        {
            LOG_F(ERROR, "{} had already parked, please unpark before {}", m_telescope->getDeviceName(), __func__);
            return DeviceError::ParkedError;
        }
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
        CHECK_DEVICE(m_telescope);
        if (!m_telescope->isParkAvailable(m_params))
        {
            LOG_F(ERROR, "{} is not support park function", m_telescope->getDeviceName());
            return DeviceError::NotSupported;
        }
        if (m_telescope->isAtPark(m_params))
        {
            DLOG_F(WARNING, "{} is already parked, please do not park again!", m_telescope->getDeviceName());
            return DeviceError::None;
        }
        if (m_telescope->Park(m_params))
        {
            LOG_F(ERROR, "{} failed to park", m_telescope->getDeviceName());
            return DeviceError::ParkError;
        }
        DLOG_F(INFO, "{} parked successfully", m_telescope->getDeviceName());
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::unpark)
    {
        CHECK_DEVICE(m_telescope);
        if (!m_telescope->isParkAvailable(m_params))
        {
            LOG_F(ERROR, "{} is not support park function", m_telescope->getDeviceName());
            return DeviceError::NotSupported;
        }
        if (!m_telescope->isAtPark(m_params))
        {
            DLOG_F(WARNING, "{} is not parked, please do not unpark before!", m_telescope->getDeviceName());
            return DeviceError::None;
        }
        if (m_telescope->Unpark(m_params))
        {
            LOG_F(ERROR, "{} failed to unpark", m_telescope->getDeviceName());
            return DeviceError::ParkError;
        }
        DLOG_F(INFO, "{} parked successfully", m_telescope->getDeviceName());
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::goHome)
    {
        CHECK_DEVICE(m_telescope);
        if (!m_telescope->isConnected())
        {
            LOG_F(ERROR, "{} is not connected when call {}", m_telescope->getDeviceName(), __func__);
            return DeviceError::NotConnected;
        }
        if (!m_telescope->isHomeAvailable({}))
        {
            LOG_F(ERROR, "{} is not support home", m_telescope->getDeviceName());
            return DeviceError::NotSupported;
        }
        if (m_telescope->isAtPark({}))
        {
            LOG_F(ERROR, "{} had already parked, please unpark before {}", m_telescope->getDeviceName(), __func__);
            return DeviceError::ParkedError;
        }
        if (!m_telescope->Home(m_params))
        {
            LOG_F(ERROR, "{} Failed to go home position");
            return DeviceError::HomeError;
        }
        DLOG_F(INFO, "{} go home position successfully!", m_telescope->getDeviceName());
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::sync)
    {
        CHECK_DEVICE(m_telescope);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getCroods)
    {
        CHECK_DEVICE_J(m_telescope, "no telescope specified");
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getObserver)
    {
        CHECK_DEVICE_J(m_telescope, "no telescope specified");
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getTime)
    {
        CHECK_DEVICE_J(m_telescope, "no telescope specified");
        return {};
    }

    DEVICE_FUNC(DeviceManager::setTelescopeParams)
    {
        CHECK_DEVICE(m_telescope);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getTelescopeParams)
    {
        CHECK_DEVICE_J(m_telescope, "no telescope specified");
        return {};
    }

    // For focuser
    DEVICE_FUNC(DeviceManager::moveStep)
    {
        CHECK_DEVICE(m_focuser);
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::moveTo)
    {
        CHECK_DEVICE(m_focuser);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getTemperatrue)
    {
        CHECK_DEVICE_J(m_focuser, "no focuser specified");
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getFocuserPosition)
    {
        CHECK_DEVICE_J(m_focuser, "no focuser specified");
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getBacklash)
    {
        CHECK_DEVICE_J(m_focuser, "no focuser specified");
        return {};
    }

    DEVICE_FUNC(DeviceManager::setFocuserParams)
    {
        CHECK_DEVICE(m_focuser);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFocuserParams)
    {
        CHECK_DEVICE_J(m_focuser, "no focuser specified");
        return {};
    }

    // For filterwheel
    DEVICE_FUNC(DeviceManager::slewTo)
    {
        CHECK_DEVICE(m_filterwheel);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFilterwheelPosition)
    {
        CHECK_DEVICE_J(m_filterwheel, "no filterwheel specified");
        return {};
    }

    DEVICE_FUNC_J(DeviceManager::getFilters)
    {
        CHECK_DEVICE_J(m_filterwheel, "no filterwheel specified");
        return {};
    }
    DEVICE_FUNC_J(DeviceManager::getOffsets)
    {
        CHECK_DEVICE_J(m_filterwheel, "no filterwheel specified");
        return {};
    }

    DEVICE_FUNC(DeviceManager::setFilterwheelParams)
    {
        CHECK_DEVICE(m_filterwheel);
        return DeviceError::None;
    }

    DEVICE_FUNC_J(DeviceManager::getFilterwheelParams)
    {
        CHECK_DEVICE_J(m_filterwheel, "no filterwheel specified");
        return {};
    }

    // For guider
    DEVICE_FUNC(DeviceManager::startGuiding)
    {
        CHECK_DEVICE(m_guider);
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopGuiding)
    {
        CHECK_DEVICE(m_guider);
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::startCalibration)
    {
        CHECK_DEVICE(m_guider);
        return DeviceError::None;
    }

    DEVICE_FUNC(DeviceManager::stopCalibration)
    {
        CHECK_DEVICE(m_guider);
        return DeviceError::None;
    }

    // For astrometry and astap
    DEVICE_FUNC_J(DeviceManager::solveImage)
    {
        CHECK_DEVICE_J(m_guider, "no guider specified");
        return DeviceError::None;
    }

    bool DeviceManager::startHydrogenServer()
    {
        if (!m_hydrogenmanager->isRunning())
        {
            m_hydrogenmanager->startServer();
        }
        return true;
    }

    bool DeviceManager::stopHydrogenServer()
    {
        if (m_hydrogenmanager->isRunning())
        {
            m_hydrogenmanager->stopServer();
        }
        return true;
    }

    bool DeviceManager::startHydrogenDevice()
    {
        if (!m_hydrogenmanager->isRunning())
        {
            LOG_F(ERROR, "Hydrogen server is not started(not by lithium server)");
            return false;
        }
        return true;
    }

    bool DeviceManager::stopHydrogenDevice()
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

    bool DeviceManager::runHydrogenServer(const json &m_params)
    {
#ifdef _WIN32

#else

#endif
        return true;
    }
    bool DeviceManager::startHydrogenDriver(const json &m_params)
    {
#ifdef _WIN32

#else

#endif
        return true;
    }
    bool DeviceManager::stopHydrogenDriver(const json &m_params)
    {
#ifdef _WIN32

#else

#endif
        return true;
    }
}
