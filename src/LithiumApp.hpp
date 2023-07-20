#pragma once

#include <memory>

#include "modules/thread/thread.hpp"
#include "modules/config/configor.hpp"
#include "modules/device/device_manager.hpp"
#include "modules/system/process.hpp"

namespace Lithium
{
    class LithiumApp
    {
    public:
        LithiumApp();
        ~LithiumApp();

    public:
        nlohmann::json GetConfig(const std::string &key_path) const;
        void SetConfig(const std::string &key_path, const nlohmann::json &value);

    public:
        std::vector<std::string> getDeviceList(DeviceType type);
        bool addDevice(DeviceType type, const std::string &name, const std::string &lib_name = "");
        bool addDeviceLibrary(const std::string &lib_path, const std::string &lib_name);
        bool removeDevice(DeviceType type, const std::string &name);
        bool removeDevicesByName(const std::string &name);
        bool removeDeviceLibrary(const std::string &lib_name);
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);
        size_t findDevice(DeviceType type, const std::string &name);
        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;
        std::shared_ptr<SimpleTask> getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params);

    public:
        void createProcess(const std::string &command, const std::string &identifier);
        void runScript(const std::string &script, const std::string &identifier);

    public:
        void addThread(std::function<void()> func, const std::string &name);
        void joinAllThreads();
        void joinThreadByName(const std::string &name);
        bool sleepThreadByName(const std::string &name, int seconds);
        bool isThreadRunning(const std::string &name);

    private:
        std::shared_ptr<Thread::ThreadManager> m_ThreadManager;
        std::shared_ptr<Config::ConfigManager> m_ConfigManager;
        std::shared_ptr<DeviceManager> m_DeviceManager;
        std::shared_ptr<Process::ProcessManager> m_ProcessManager;
    };
    extern LithiumApp MyApp;
} // namespace Lithium
