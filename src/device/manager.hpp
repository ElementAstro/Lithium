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

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

#include "atom/driver/device.hpp"
#include "atom/driver/device_type.hpp"
#include "atom/type/message.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/async/thread.hpp"
#include "atom/driver/iproperty.hpp"

#include "addon/loader.hpp"
#include "config/configor.hpp"

#include "server/hydrogen.hpp"

#include "error/error_code.hpp"

class Camera;
class Telescope;
class Focuser;
class Filterwheel;
class Guider;
class Solver;

using json = nlohmann::json;

#define DEVICE_FUNC(func_name) \
    DeviceError func_name(const json &m_params)

#define DEVICE_FUNC_J(func_name) \
    const json func_name(const json &m_params)

namespace Lithium
{    
    /**
     * @class DeviceManager
     * @brief 设备管理器类，用于管理各种设备对象。
     */
    class DeviceManager
    {
    public:
        /**
         * @brief 构造函数，创建一个设备管理器对象。
         * @param messageBus 消息总线对象的共享指针。
         * @param configManager 配置管理器对象的共享指针。
         */
        DeviceManager(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager);

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
        static std::shared_ptr<DeviceManager> createShared(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager);

        static std::unique_ptr<DeviceManager> createUnique(std::shared_ptr<Atom::Server::MessageBus> messageBus, std::shared_ptr<ConfigManager> configManager);

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
        bool addDevice(DeviceType type, const std::string &name, const std::string &lib_name);

        /**
         * @brief 添加设备库到设备管理器中。
         * @param lib_path 设备库路径。
         * @param lib_name 设备库名称。
         * @return 如果添加成功返回true，否则返回false。
         */
        bool addDeviceLibrary(const std::string &lib_path, const std::string &lib_name);

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
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);

        /**
         * @brief 查找指定设备类型和名称的设备在设备管理器中的索引。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @return 返回设备在设备管理器中的索引，如果设备不存在则返回`std::string::npos`。
         */
        size_t findDevice(DeviceType type, const std::string &name);

        /**
         * @brief 根据设备名称查找设备对象。
         * @param name 设备名称。
         * @return 返回指向设备对象的共享指针，如果设备不存在则返回空指针。
         */
        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;

        /**
         * @brief 获取指定设备类型、设备名称、任务名称和参数的简单任务对象。
         * @param type 设备类型枚举值。
         * @param device_name 设备名称。
         * @param task_name 任务名称。
         * @param params 任务参数的JSON对象。
         * @return 返回指向简单任务对象的共享指针。
         */
        std::shared_ptr<Atom::Task::SimpleTask> getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const json &params);

        /**
         * @brief 发布字符串类型的消息到消息总线。
         * @param message 字符串属性的共享指针。
         */
        void messageBusPublishString(const std::shared_ptr<IStringProperty> &message);

        /**
         * @brief 发布数字类型的消息到消息总线。
         * @param message 数字属性的共享指针。
         */
        void messageBusPublishNumber(const std::shared_ptr<INumberProperty> &message);

        /**
         * @brief 发布布尔类型的消息到消息总线。
         * @param message 布尔属性的共享指针。
         */
        void messageBusPublishBool(const std::shared_ptr<IBoolProperty> &message);

        /**
         * @brief 设置设备属性值。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @param value_name 属性名称。
         * @param value 属性值。
         * @return 如果设置成功返回true，否则返回错误信息。
         */
        bool setDeviceProperty(DeviceType type, const std::string &name, const std::string &value_name, const std::any &value);

        /**
         * @brief 根据设备名称设置设备属性值。
         * @param name 设备名称。
         * @param value_name 属性名称。
         * @param value 属性值。
         * @return 如果设置成功返回true，否则返回错误信息。
         */
        bool setDevicePropertyByName(const std::string &name, const std::string &value_name, const std::any &value);

    // Device Dispatch
    public:

        bool setMainCamera(const std::string & name);
        bool setGuidingCamera(const std::string &name);
        bool setTelescope(const std::string &name);
        bool setFocuser(const std::string &name);
        bool setFilterwheel(const std::string &name);
        bool setGuider(const std::string &name);

    // Device Function
    public:

        // For camera
        DEVICE_FUNC(startExposure);
        DEVICE_FUNC(stopExposure);
        DEVICE_FUNC(setGain);
        DEVICE_FUNC(setOffset);
        DEVICE_FUNC(setISO);
        DEVICE_FUNC(startCooling);
        DEVICE_FUNC(stopCooling);
        DEVICE_FUNC(setCamareParams);
        DEVICE_FUNC_J(getCameraParams);

        // For telescope
        DEVICE_FUNC(gotoTarget);
        DEVICE_FUNC(park);
        DEVICE_FUNC(unpark);
        DEVICE_FUNC(goHome);
        DEVICE_FUNC(sync);
        DEVICE_FUNC_J(getCroods);
        DEVICE_FUNC_J(getObserver);
        DEVICE_FUNC_J(getTime);
        DEVICE_FUNC(setTelescopeParams);
        DEVICE_FUNC_J(getTelescopeParams);

        // For focuser
        DEVICE_FUNC(moveStep);
        DEVICE_FUNC(moveTo);
        DEVICE_FUNC_J(getTemperatrue);
        DEVICE_FUNC_J(getFocuserPosition);
        DEVICE_FUNC_J(getBacklash);
        DEVICE_FUNC(setFocuserParams);
        DEVICE_FUNC_J(getFocuserParams);
        
        // For filterwheel
        DEVICE_FUNC(slewTo);
        DEVICE_FUNC_J(getFilterwheelPosition);
        DEVICE_FUNC_J(getFilters);
        DEVICE_FUNC_J(getOffsets);
        DEVICE_FUNC(setFilterwheelParams);
        DEVICE_FUNC_J(getFilterwheelParams);

        // For guider
        DEVICE_FUNC(startGuiding);
        DEVICE_FUNC(stopGuiding);
        DEVICE_FUNC(startCalibration);
        DEVICE_FUNC(stopCalibration);

        // For astrometry and astap
        DEVICE_FUNC_J(solveImage);

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
        std::vector<std::shared_ptr<Device>> m_devices[static_cast<int>(DeviceType::NumDeviceTypes)]; ///< 存储设备对象的数组，每个设备类型对应一个向量。

        std::mutex m_mutex; ///< 互斥锁，用于保护设备管理器的并发访问。

        std::shared_ptr<ModuleLoader> m_ModuleLoader;           ///< 模块加载器对象的共享指针。
        std::shared_ptr<Atom::Server::MessageBus> m_MessageBus;               ///< 消息总线对象的共享指针。
        std::shared_ptr<ConfigManager> m_ConfigManager; ///< 配置管理器对象的共享指针。
        std::shared_ptr<Atom::Async::ThreadManager> m_ThreadManager;

    // Device for quick performance
    private:
        std::shared_ptr<Camera> m_main_camera;
        std::shared_ptr<Camera> m_guiding_camera;
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

}