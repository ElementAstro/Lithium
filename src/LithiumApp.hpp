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

        private:
            std::shared_ptr<Thread::ThreadManager> m_ThreadManager;
            std::shared_ptr<Config::ConfigManager> m_ConfigManager;
            std::shared_ptr<DeviceManager> m_DeviceManager;
    };
    extern LithiumApp MyApp;
} // namespace Lithium
