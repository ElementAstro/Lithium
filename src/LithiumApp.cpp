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

    std::vector<std::string> LithiumApp::getDeviceList(DeviceType type)
    {
        return m_DeviceManager->getDeviceList(type);
    }

    void LithiumApp::addDevice(DeviceType type, const std::string &name, const std::string &lib_name)
    {
        m_DeviceManager->addDevice(type, name, lib_name);
    }

    void LithiumApp::removeDevice(DeviceType type, const std::string &name)
    {
        m_DeviceManager->removeDevice(type, name);
    }

    void LithiumApp::removeDevicesByName(const std::string &name)
    {
        m_DeviceManager->removeDevicesByName(name);
    }

    std::shared_ptr<Device> LithiumApp::getDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager->getDevice(type, name);
    }

    size_t LithiumApp::findDevice(DeviceType type, const std::string &name)
    {
        return m_DeviceManager->findDevice(type, name);
    }

    std::shared_ptr<Device> LithiumApp::findDeviceByName(const std::string &name) const
    {
        return m_DeviceManager->findDeviceByName(name);
    }

    std::shared_ptr<SimpleTask> LithiumApp::getSimpleTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        return m_DeviceManager->getSimpleTask(type, device_type, device_name, task_name, params);
    }

}