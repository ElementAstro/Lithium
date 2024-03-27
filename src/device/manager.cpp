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
#include "task/pool.hpp"

#include "atom/driver/camera.hpp"
#include "atom/driver/device_type.hpp"
#include "atom/driver/filterwheel.hpp"
#include "atom/driver/focuser.hpp"
#include "atom/driver/guider.hpp"
#include "atom/driver/solver.hpp"
#include "atom/driver/telescope.hpp"

#include "atom/driver/camera_utils.hpp"
#include "atom/driver/exception.hpp"
#include "atom/utils/random.hpp"
#include "utils/utils.hpp"

#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#include <typeinfo>

#include "config.h"

#include "atom/log/loguru.hpp"
#include "magic_enum/magic_enum.hpp"

namespace Lithium {

// Constructor
DeviceManager::DeviceManager(
    std::shared_ptr<Atom::Server::MessageBus> messageBus,
    std::shared_ptr<ConfigManager> configManager) {
    m_ModuleLoader = ModuleLoader::createShared("drivers");
    m_ConfigManager = configManager;
    m_MessageBus = messageBus;
    for (auto &devices : m_devices) {
        devices.emplace_back();
    }

    m_hydrogenmanager = std::make_shared<HydrogenManager>();
}

DeviceManager::~DeviceManager() {
    for (auto &devices : m_devices) {
        for (auto &device : devices) {
            if (device) {
                // device->disconnect();
            }
        }
    }
}

std::shared_ptr<DeviceManager> DeviceManager::createShared(
    std::shared_ptr<Atom::Server::MessageBus> messageBus,
    std::shared_ptr<ConfigManager> configManager) {
    return std::make_shared<DeviceManager>(messageBus, configManager);
}

std::unique_ptr<DeviceManager> DeviceManager::createUnique(
    std::shared_ptr<Atom::Server::MessageBus> messageBus,
    std::shared_ptr<ConfigManager> configManager) {
    return std::make_unique<DeviceManager>(messageBus, configManager);
}

void DeviceManager::connectToMessageBus() {
    m_MessageBus->Subscribe<std::shared_ptr<Message>>(
        "device", [this](std::shared_ptr<Message> message) -> void {
            switch (message->type()) {
                case Message::Type::kNumber: {
                    std::shared_ptr<NumberMessage> numberMessage =
                        std::dynamic_pointer_cast<NumberMessage>(message);

                    break;
                }
                case Message::Type::kText: {
                    std::shared_ptr<TextMessage> stringMessage =
                        std::dynamic_pointer_cast<TextMessage>(message);
                    break;
                }
                case Message::Type::kBoolean: {
                    std::shared_ptr<BooleanMessage> booleanMessage =
                        std::dynamic_pointer_cast<BooleanMessage>(message);
                    break;
                }
                default: {
                    LOG_F(ERROR, "Unknown message type {}",
                          magic_enum::enum_name(message->type()));
                    break;
                }
            }
        });
}

std::vector<std::string> DeviceManager::getDeviceList() {
    std::vector<std::string> deviceList;
    std::initializer_list<DeviceType> devices = {
        DeviceType::Camera, DeviceType::Focuser, DeviceType::Telescope,
        DeviceType::Guider, DeviceType::Solver,  DeviceType::FilterWheel};
    for (auto &device_type : devices) {
        auto &devices = m_devices[static_cast<int>(device_type)];
        for (const auto &device : devices) {
            if (device) {
                deviceList.emplace_back(device->getName());
            }
        }
    }
    return deviceList;
}

std::vector<std::string> DeviceManager::getDeviceListByType(DeviceType type) {
    std::vector<std::string> deviceList;
    auto &devices = m_devices[static_cast<int>(type)];
    for (const auto &device : devices) {
        if (device) {
            deviceList.emplace_back(device->getName());
        }
    }
    return deviceList;
}

bool DeviceManager::addDevice(DeviceType type, const std::string &name,
                              const std::string &lib_name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (type < DeviceType::Camera || type > DeviceType::Guider) {
        throw InvalidDeviceType("Invalid device type");
    }
    if (findDeviceByName(name)) {
        LOG_F(ERROR,
              "A device with name {} already exists, please choose a different "
              "name",
              name);
        return false;
    }
    std::string newName = name;
    int index = 1;
    while (static_cast<int>(findDevice(type, newName)) != -1) {
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
    try {
        if (lib_name.empty()) {
            LOG_F(ERROR,
                  "Please specific a device library when you want to create a "
                  "new instance");
            return false;
        } else {
            json params;
            params["name"] = newName;
            std::string logMsg;

            switch (type) {
                case DeviceType::Camera:
                    logMsg = fmt::format(
                        "Trying to add a new camera instance : {} from {}",
                        newName, lib_name);
                    m_devices[static_cast<int>(type)].emplace_back(
                        m_ModuleLoader->GetInstance<AtomCamera>(lib_name, params,
                                                            "GetInstance"));
                    break;
                case DeviceType::Telescope:
                    logMsg = fmt::format(
                        "Trying to add a new telescope instance : {}", newName);
                    m_devices[static_cast<int>(type)].emplace_back(
                        std::make_shared<Telescope>(newName));
                    break;
                case DeviceType::Focuser:
                    logMsg = fmt::format(
                        "Trying to add a new Focuser instance : {}", newName);
                    m_devices[static_cast<int>(type)].emplace_back(
                        std::make_shared<Focuser>(newName));
                    break;
                case DeviceType::FilterWheel:
                    logMsg = fmt::format(
                        "Trying to add a new filterwheel instance : {}",
                        newName);
                    m_devices[static_cast<int>(type)].emplace_back(
                        std::make_shared<Filterwheel>(newName));
                    break;
                case DeviceType::Solver:
                    logMsg = fmt::format(
                        "Trying to add a new solver instance : {} from {}",
                        newName, lib_name);
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
                    break;
                case DeviceType::Guider:
                    // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
                    break;
                default:
                    LOG_F(ERROR, "Invalid device type");
                    throw InvalidDeviceType("Invalid device type");
            }

            if (!logMsg.empty()) {
                DLOG_F(INFO, "{}", logMsg);
            }
            DLOG_F(INFO, "Added new {} instance successfully",
                   magic_enum::enum_name(type));
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to add device {} , error : {}", newName, e.what());
        return false;
    }
    if (m_ConfigManager) {
#ifdef __cpp_lib_format
        m_ConfigManager->setValue(std::format("driver/{}/name", newName),
                                  newName);
#else
        m_ConfigManager->setValue(fmt::format("driver/{}/name", newName),
                                  newName);
#endif
    } else {
        LOG_F(ERROR, "Config manager not initialized");
    }
    return true;
}

bool DeviceManager::addDeviceLibrary(const std::string &lib_path,
                                     const std::string &lib_name) {
    if (lib_path.empty() || lib_name.empty()) {
        LOG_F(ERROR, "Library path and name is required!");
        return false;
    }
    if (!m_ModuleLoader->LoadModule(lib_path, lib_name)) {
        LOG_F(ERROR, "Failed to load device library : {} in {}", lib_name,
              lib_path);
        return false;
    }
    return true;
}

bool DeviceManager::addDeviceObserver(DeviceType type,
                                      const std::string &name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto &devices = m_devices[static_cast<int>(type)];
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (*it && (*it)->getName() == name) {
            /*
            (*it)->AddObserverFunc([this](const std::any &message)
                                   {
                                if(message.has_value())
                                {
                                    try
                                    {
                                        if (message.type() ==
            typeid(std::shared_ptr<IStringProperty>))
                                        {
                                            messageBusPublishString(std::any_cast<std::shared_ptr<IStringProperty>>(message));
                                        }
                                        else if(message.type() ==
            typeid(std::shared_ptr<INumberProperty>))
                                        {
                                            messageBusPublishNumber(std::any_cast<std::shared_ptr<INumberProperty>>(message));
                                        }
                                        else if (message.type() ==
            typeid(std::shared_ptr<IBoolProperty>))
                                        {
                                            messageBusPublishBool(std::any_cast<std::shared_ptr<IBoolProperty>>(message));
                                        }
                                        else
                                        {
                                            LOG_F(ERROR,"Unknown property
            type!");
                                        }
                                    }
                                    catch(const std::bad_any_cast &e)
                                    {
                                        LOG_F(ERROR,"Failed to cast property
            {}",e.what());
                                    }
                                } });
            */
            DLOG_F(INFO, "Add device {} observer successfully", name);

            return true;
        }
    }
    LOG_F(ERROR, "Could not find device {} of type %d", name,
          static_cast<int>(type));
    return false;
}

bool DeviceManager::removeDevice(DeviceType type, const std::string &name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto &devices = m_devices[static_cast<int>(type)];
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (*it && (*it)->getName() == name) {
            (*it)->runFunc("disconnect", {});
            devices.erase(it);
            DLOG_F(INFO, "Remove device {} successfully", name);
            if (m_ConfigManager) {
#ifdef __cpp_lib_format
                m_ConfigManager->deleteValue(std::format("driver/{}", name));
#else
                m_ConfigManager->deleteValue(fmt::format("driver/{}", name));
#endif
            } else {
                LOG_F(ERROR, "Config manager not initialized");
            }
            return true;
        }
    }
    LOG_F(ERROR, "Could not find device {} of type %d", name,
          static_cast<int>(type));
    return false;
}

bool DeviceManager::removeDeviceByName(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto &devices : m_devices) {
        devices.erase(
            std::remove_if(devices.begin(), devices.end(),
                           [&](const std::shared_ptr<AtomDriver> &device) {
                               return device && device->getName() == name;
                           }),
            devices.end());
    }
    if (m_ConfigManager) {
#ifdef __cpp_lib_format
        m_ConfigManager->deleteValue(std::format("driver/{}", name));
#else
        m_ConfigManager->deleteValue(fmt::format("driver/{}", name));
#endif
    } else {
        LOG_F(ERROR, "Config manager not initialized");
    }
    return true;
}

bool DeviceManager::removeDeviceLibrary(const std::string &lib_name) {
    if (lib_name.empty()) {
        LOG_F(ERROR, "Library name is required");
        return false;
    }
    if (!m_ModuleLoader->UnloadModule(lib_name)) {
        LOG_F(ERROR, "Failed to remove device library : {} with unload error",
              lib_name);
        return false;
    }
    return true;
}

std::shared_ptr<AtomDriver> DeviceManager::getDevice(DeviceType type,
                                                     const std::string &name) {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t index = findDevice(type, name);
    if (static_cast<int>(index) != -1) {
        return m_devices[static_cast<int>(type)][index];
    } else {
        DLOG_F(WARNING, "Could not find device {} of type %d", name,
               static_cast<int>(type));
        return nullptr;
    }
}

size_t DeviceManager::findDevice(DeviceType type, const std::string &name) {
    auto &devices = m_devices[static_cast<int>(type)];
    for (size_t i = 0; i < devices.size(); ++i) {
        if (devices[i] && devices[i]->getName() == name) {
            return i;
        }
    }
    return -1;
}

std::shared_ptr<AtomDriver> DeviceManager::findDeviceByName(
    const std::string &name) const {
    for (const auto &devices : m_devices) {
        for (const auto &device : devices) {
            if (device && device->getName() == name) {
                return device;
            }
        }
    }
    return nullptr;
}

bool DeviceManager::setDeviceProperty(DeviceType type, const std::string &name,
                                      const std::string &value_name,
                                      const std::any &value) {
    m_TaskPool->enqueue([this, &value, &type, &name, &value_name]() {
        auto device = getDevice(type, name);
        if (!device) {
            LOG_F(ERROR, "{} not found", name);
            return;
        }
        try {
            // if (value.type() == typeid(std::string) || value.type() ==
            // typeid(const char *))
            //     device->SetVariable<std::string>(value_name,std::any_cast<std::string>(value));
            // if (value.type() == typeid(int) || value.type() ==
            // typeid(double))
            //     device->SetVariable<double>(value_name,
            //     std::any_cast<double>(value));
        } catch (const std::bad_any_cast &e) {
            LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name, name,
                  e.what());
        }
    });
    return true;
}

bool DeviceManager::setDevicePropertyByName(const std::string &name,
                                            const std::string &value_name,
                                            const std::any &value) {
    m_TaskPool->enqueue([this, &value, &name, &value_name]() {
        auto device = findDeviceByName(name);
        if (!device) {
            LOG_F(ERROR, "{} not found", name);
            return;
        }
        try {
            // if (value.type() == typeid(std::string) || value.type() ==
            // typeid(const char *))
            //     device->SetVariable<std::string>(value_name,std::any_cast<std::string>(value));
            // if (value.type() == typeid(int) || value.type() ==
            // typeid(double))
            //     device->SetVariable<double>(value_name,
            //     std::any_cast<double>(value));

        } catch (const std::bad_any_cast &e) {
            LOG_F(ERROR, "Failed to convert {} of {} with {}", value_name, name,
                  e.what());
        }
    });
    return true;
}

bool DeviceManager::setMainCamera(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_main_camera =
                std::dynamic_pointer_cast<AtomCamera>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set main camera to: {} with {}", name,
                  e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::setGuidingCamera(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_guiding_camera =
                std::dynamic_pointer_cast<AtomCamera>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set main camera to: {} with {}", name,
                  e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::setTelescope(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_telescope =
                std::dynamic_pointer_cast<Telescope>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set telescope to: {} with {}", name,
                  e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::setFocuser(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_focuser =
                std::dynamic_pointer_cast<Focuser>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set focuser to: {} with {}", name,
                  e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::setFilterwheel(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_filterwheel =
                std::dynamic_pointer_cast<Filterwheel>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set filterwheel to: {} with {}", name,
                  e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::setGuider(const std::string &name) {
    if (name.empty())
        return false;
    if (findDeviceByName(name)) {
        try {
            m_guider =
                std::dynamic_pointer_cast<Guider>(findDeviceByName(name));
        } catch (const std::bad_alloc &e) {
            LOG_F(ERROR, "Failed to set guider to: {} with {}", name, e.what());
            return false;
        }
    }
    return true;
}

bool DeviceManager::startHydrogenServer() {
    if (!m_hydrogenmanager->isRunning()) {
        m_hydrogenmanager->startServer();
    }
    return true;
}

bool DeviceManager::stopHydrogenServer() {
    if (m_hydrogenmanager->isRunning()) {
        m_hydrogenmanager->stopServer();
    }
    return true;
}

bool DeviceManager::startHydrogenDevice() {
    if (!m_hydrogenmanager->isRunning()) {
        LOG_F(ERROR, "Hydrogen server is not started(not by lithium server)");
        return false;
    }
    return true;
}

bool DeviceManager::stopHydrogenDevice() { return true; }

bool DeviceManager::startASCOMServer() { return true; }

bool DeviceManager::stopASCOMServer() { return true; }

bool DeviceManager::startASCOMDevice() { return true; }

bool DeviceManager::stopASCOMDevice() { return true; }

bool DeviceManager::runHydrogenServer(const json &m_params) {
#ifdef _WIN32

#else

#endif
    return true;
}
bool DeviceManager::startHydrogenDriver(const json &m_params) {
#ifdef _WIN32

#else

#endif
    return true;
}
bool DeviceManager::stopHydrogenDriver(const json &m_params) {
#ifdef _WIN32

#else

#endif
    return true;
}
}  // namespace Lithium
