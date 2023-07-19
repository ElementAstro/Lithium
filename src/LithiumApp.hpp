#pragma once

#include <memory>

#include "modules/thread/thread.hpp"
#include "modules/config/configor.hpp"
#include "modules/device/device_manager.hpp"

namespace Lithium
{
    class LithiumApp
    {
    public:
        LithiumApp();
        ~LithiumApp();

        std::shared_ptr<Config::ConfigManager> GetConfigManager()
        {
            return m_ConfigManager;
        }

        std::shared_ptr<DeviceManager> GetDeviceManager()
        {
            return m_DeviceManager;
        }

    public:
        nlohmann::json GetConfig(const std::string &key_path) const;
        void SetConfig(const std::string &key_path, const nlohmann::json &value);

        std::vector<std::string> getDeviceList(DeviceType type);
        void addDevice(DeviceType type, const std::string &name, const std::string &lib_name = "");
        void removeDevice(DeviceType type, const std::string &name);
        void removeDevicesByName(const std::string &name);
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);
        size_t findDevice(DeviceType type, const std::string &name);
        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;
        std::shared_ptr<SimpleTask> getSimpleTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params);

    private:
        std::shared_ptr<Thread::ThreadManager> m_ThreadManager;
        std::shared_ptr<Config::ConfigManager> m_ConfigManager;
        std::shared_ptr<DeviceManager> m_DeviceManager;
    };
    extern LithiumApp MyApp;
} // namespace Lithium
