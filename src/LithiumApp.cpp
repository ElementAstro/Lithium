#include "LithiumApp.hpp"

#include "loguru/loguru.hpp"

namespace Lithium
{
    LithiumApp MyApp;
    LithiumApp::LithiumApp()
    {
        LOG_F(INFO, "Loading Lithium App and preparing ...");
        try
        {
            m_ThreadManager = std::make_shared<Thread::ThreadManager>(10);
            m_ConfigManager = std::make_shared<Config::ConfigManager>();
            m_DeviceManager = std::make_shared<DeviceManager>();
            LOG_F(INFO, "Lithium App Loaded.");
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load Lithium App , error : %s", e.what());
        }
    }

    LithiumApp::~LithiumApp()
    {
    }

    nlohmann::json LithiumApp::GetConfig(const std::string &key_path) const
    {
        LOG_F(INFO, "Get value : %s", key_path.c_str());
        return m_ConfigManager->getValue(key_path);
    }

    void LithiumApp::SetConfig(const std::string &key_path, const nlohmann::json &value)
    {
        LOG_F(INFO, "Set %s to %s", key_path.c_str(), value.dump().c_str());
        m_ConfigManager->setValue(key_path, value);
    }

}