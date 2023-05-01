/*
 * openapt.hpp
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
 
Date: 2023-3-27
 
Description: Main 
 
**************************************************/

#ifndef _OPENAPT_HPP_
#define _OPENAPT_HPP_

#include "crow.h"

#include "plugins/thread.hpp"
#include "task/runner.hpp"
#include "device/manager.hpp"
#include "module/modloader.hpp"
#include "module/pythonloader.hpp"
#include "config/configor.hpp"
#include "package/packageloader.hpp"

#include "websocketpp/config/asio.hpp"
#include "websocketpp/server.hpp"

class MyApp {
public:
    MyApp();
    ~MyApp();

    void Initialize(bool useSSL = false, const std::string& certPath = "");

    crow::SimpleApp & GetApp() {
        return app;
    }

    OpenAPT::ThreadManager* GetThreadManager() const {
        return m_ThreadManager;
    }

    OpenAPT::TaskManager* GetTaskManager() const {
        return m_TaskManager;
    }

    OpenAPT::DeviceManager* GetDeviceManager() const {
        return m_DeviceManager;
    }

    OpenAPT::ModuleLoader* GetModuleLoader() const {
        return m_ModuleLoader;
    }

    OpenAPT::ConfigManager* GetConfigManager() const {
        return m_ConfigManager;
    }

    OpenAPT::PackageManager* GetPackageManager() const {
        return m_PackageManager;
    }

    OpenAPT::PyModuleLoader* GetPythonLoader() const {
        return m_PythonLoader;
    }

    void sendJSONMessage(const nlohmann::json& msg);

private:
    crow::SimpleApp app;

    std::shared_ptr<websocketpp::server<websocketpp::config::asio_tls>> m_Server;
    std::map<websocketpp::connection_hdl, std::string> m_Connections;

    OpenAPT::ThreadManager* m_ThreadManager;
    OpenAPT::TaskManager* m_TaskManager;
    OpenAPT::DeviceManager* m_DeviceManager;
    OpenAPT::ModuleLoader* m_ModuleLoader;
    OpenAPT::ConfigManager* m_ConfigManager;
    OpenAPT::PackageManager* m_PackageManager;
    OpenAPT::PyModuleLoader* m_PythonLoader;

    bool m_UseSSL;
    std::string m_CertPath;

    void on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
};

extern bool DEBUG;

#endif