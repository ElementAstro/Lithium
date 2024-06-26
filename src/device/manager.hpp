/*
 * device_manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Device Manager

**************************************************/

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "atom/driver/device.hpp"
#include "atom/driver/device_type.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/type/message.hpp"

#include "addon/loader.hpp"
#include "config/configor.hpp"
#include "task/pool.hpp"

#include "server/hydrogen.hpp"

#include "error/error_code.hpp"

class AtomCamera;
class Telescope;
class Focuser;
class Filterwheel;
class Guider;
class Solver;

using json = nlohmann::json;

namespace lithium {
/**
 * @class DeviceManager
 * @brief 设备管理器类，用于管理各种设备对象。
 */
class DeviceManager {
public:
    /**
     * @brief 构造函数，创建一个设备管理器对象。
     * @param messageBus 消息总线对象的共享指针。
     * @param configManager 配置管理器对象的共享指针。
     */
    explicit DeviceManager(std::shared_ptr<atom::server::MessageBus> messageBus,
                  std::shared_ptr<ConfigManager> configManager);

    /**
     * @brief 析构函数，销毁设备管理器对象。
     */
    ~DeviceManager();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * @brief 创建一个共享的设备管理器对象。
     * @param messageBus 消息总线对象的共享指针。
     * @param configManager 配置管理器对象的共享指针。
     * @return 返回一个指向设备管理器对象的共享指针。
     */
    static std::shared_ptr<DeviceManager> createShared(
        std::shared_ptr<atom::server::MessageBus> messageBus,
        std::shared_ptr<ConfigManager> configManager);

    static std::unique_ptr<DeviceManager> createUnique(
        std::shared_ptr<atom::server::MessageBus> messageBus,
        std::shared_ptr<ConfigManager> configManager);

    // -------------------------------------------------------------------
    // Message methods
    // -------------------------------------------------------------------

    void connectToMessageBus();

    // -------------------------------------------------------------------
    // Device methods
    // -------------------------------------------------------------------

    std::vector<std::string> getDeviceList();
    /**
     * @brief 获取指定类型设备的设备列表。
     * @param type 设备类型枚举值。
     * @return 返回包含设备名称的字符串向量。
     */
    std::vector<std::string> getDeviceListByType(DeviceType type);

    /**
     * @brief 添加设备到设备管理器中。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @param lib_name 设备库名称。
     * @return 如果添加成功返回true，否则返回false。
     */
    bool addDevice(DeviceType type, const std::string &name,
                   const std::string &lib_name);

    /**
     * @brief 添加设备库到设备管理器中。
     * @param lib_path 设备库路径。
     * @param lib_name 设备库名称。
     * @return 如果添加成功返回true，否则返回false。
     */
    bool addDeviceLibrary(const std::string &lib_path,
                          const std::string &lib_name);

    /**
     * @brief 添加设备观察者。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @return 如果添加成功返回true，否则返回false。
     */
    bool addDeviceObserver(DeviceType type, const std::string &name);

    /**
     * @brief 从设备管理器中移除指定设备。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @return 如果移除成功返回true，否则返回false。
     */
    bool removeDevice(DeviceType type, const std::string &name);

    /**
     * @brief 根据设备名称从设备管理器中移除设备。
     * @param name 设备名称。
     * @return 如果移除成功返回true，否则返回false。
     */
    bool removeDeviceByName(const std::string &name);

    /**
     * @brief 从设备管理器中移除指定设备库。
     * @param lib_name 设备库名称。
     * @return 如果移除成功返回true，否则返回false。
     */
    bool removeDeviceLibrary(const std::string &lib_name);

    /**
     * @brief 获取指定设备类型和名称的设备对象。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @return 返回指向设备对象的共享指针，如果设备不存在则返回空指针。
     */
    std::shared_ptr<AtomDriver> getDevice(DeviceType type,
                                          const std::string &name);

    /**
     * @brief 查找指定设备类型和名称的设备在设备管理器中的索引。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @return
     * 返回设备在设备管理器中的索引，如果设备不存在则返回`std::string::npos`。
     */
    size_t findDevice(DeviceType type, const std::string &name);

    /**
     * @brief 根据设备名称查找设备对象。
     * @param name 设备名称。
     * @return 返回指向设备对象的共享指针，如果设备不存在则返回空指针。
     */
    std::shared_ptr<AtomDriver> findDeviceByName(const std::string &name) const;

    /**
     * @brief 设置设备属性值。
     * @param type 设备类型枚举值。
     * @param name 设备名称。
     * @param value_name 属性名称。
     * @param value 属性值。
     * @return 如果设置成功返回true，否则返回错误信息。
     */
    bool setDeviceProperty(DeviceType type, const std::string &name,
                           const std::string &value_name,
                           const std::any &value);

    /**
     * @brief 根据设备名称设置设备属性值。
     * @param name 设备名称。
     * @param value_name 属性名称。
     * @param value 属性值。
     * @return 如果设置成功返回true，否则返回错误信息。
     */
    bool setDevicePropertyByName(const std::string &name,
                                 const std::string &value_name,
                                 const std::any &value);

    // Device Dispatch
public:
    bool setMainCamera(const std::string &name);
    bool setGuidingCamera(const std::string &name);
    bool setTelescope(const std::string &name);
    bool setFocuser(const std::string &name);
    bool setFilterwheel(const std::string &name);
    bool setGuider(const std::string &name);

public:
    bool startHydrogenServer();
    bool stopHydrogenServer();
    bool startHydrogenDevice();
    bool stopHydrogenDevice();

    bool startASCOMServer();
    bool stopASCOMServer();
    bool startASCOMDevice();
    bool stopASCOMDevice();

private:
    std::vector<std::shared_ptr<AtomDriver>> m_devices[static_cast<int>(
        DeviceType::
            NumDeviceTypes)];  ///< 存储设备对象的数组，每个设备类型对应一个向量。

    std::mutex m_mutex;  ///< 互斥锁，用于保护设备管理器的并发访问。

    std::shared_ptr<ModuleLoader>
        m_ModuleLoader;  ///< 模块加载器对象的共享指针。
    std::shared_ptr<atom::server::MessageBus>
        m_MessageBus;  ///< 消息总线对象的共享指针。
    std::shared_ptr<ConfigManager>
        m_ConfigManager;  ///< 配置管理器对象的共享指针。
    std::shared_ptr<TaskPool> m_TaskPool;

    // Device for quick performance
private:
    std::shared_ptr<AtomCamera> m_main_camera;
    std::shared_ptr<AtomCamera> m_guiding_camera;
    std::shared_ptr<Telescope> m_telescope;
    std::shared_ptr<Focuser> m_focuser;
    std::shared_ptr<Filterwheel> m_filterwheel;
    std::shared_ptr<Guider> m_guider;
    std::shared_ptr<Solver> m_solver;

    std::shared_ptr<HydrogenManager> m_hydrogenmanager;
    std::shared_ptr<HydrogenDriverCollection> m_hydrogencollection;

    // For Hydrogen Inside Server
public:
    bool runHydrogenServer(const json &m_params);
    bool startHydrogenDriver(const json &m_params);
    bool stopHydrogenDriver(const json &m_params);

private:
#if __cplusplus >= 202002L
    std::jthread m_hydrogen_server_thread;
#else
    std::thread m_hydrogen_server_thread;
#endif
};

}  // namespace lithium
