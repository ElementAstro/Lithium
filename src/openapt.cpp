/*
 * openapt.cpp
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

#include "openapt.hpp"

#include <spdlog/spdlog.h> // 引入 spdlog 日志库

crow::SimpleApp app;

#include <fstream>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <cstdlib>
#include <functional>
#include <exception>

#include "nlohmann/json.hpp"

#include "device/basic_device.hpp"

using json = nlohmann::json;

OpenAPT::ThreadManager m_ThreadManager;
OpenAPT::TaskManager m_TaskManager;
OpenAPT::DeviceManager m_DeviceManager;

bool DEBUG = true;

void parse_args(int argc, char* argv[]) {
    std::vector<std::string> argList;
    for (int i = 1; i < argc; i++) { // 从第二个参数开始遍历
        argList.push_back(argv[i]); // 保存命令行参数到vector中
    }

    try {
        // 解析命令行参数
        for (int i = 0; i < argList.size(); i++) {
            if (argList[i] == "-d" || argList[i] == "--debug") {
                spdlog::info("DEBUG Mode is enabled by command line argument");
                DEBUG = true;
            }
            else {
                throw std::invalid_argument("Invalid argument"); // 抛出异常: 无效参数
            }
        }
    } catch (std::invalid_argument& e) {
        spdlog::error(e.what()); // 记录错误日志
    }
}

void LoadUrl() {
    CROW_ROUTE(app, "/")
    ([]{
        return crow::mustache::load("index.html").render();
    });

    CROW_ROUTE(app, "/client")
    ([]{
        return crow::mustache::load("client.html").render();
    });
}

void TestAll() {

    std::shared_ptr<OpenAPT::ConditionalTask> conditionalTask(new OpenAPT::ConditionalTask(
        []() { spdlog::info("conditional task executed!"); },
        {{"threshold", 10}},
        [](const json& params) -> bool { 
            spdlog::info("Conditon function was called!");
            return params["threshold"].get<int>() > 5; 

            }
    ));

    m_TaskManager.addTask(conditionalTask);

    m_TaskManager.executeAllTasks();

    m_DeviceManager.addDevice(OpenAPT::DeviceType::Camera, "Camera1");

    auto cameraList = m_DeviceManager.getDeviceList(OpenAPT::DeviceType::Camera);
    std::cout << "相机列表: ";
    for (auto& name : cameraList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    auto telescopeList = m_DeviceManager.getDeviceList(OpenAPT::DeviceType::Telescope);
    std::cout << "望远镜列表: ";
    for (auto& name : telescopeList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {

    parse_args(argc, argv);

    LoadUrl();

    if (DEBUG) {
        spdlog::set_level(spdlog::level::debug);
        app.loglevel(crow::LogLevel::DEBUG);
        TestAll();
    } else {
        spdlog::set_level(spdlog::level::info);
        app.loglevel(crow::LogLevel::ERROR);
    }

    CROW_WEBSOCKET_ROUTE(app, "/app")
      .onopen([&](crow::websocket::connection& conn) {
        spdlog::info("WebSocket connection opened.");
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
        spdlog::warn("WebSocket connection closed. Reason: {}", reason);
      })
      .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
        try {
            auto j = json::parse(data); // 解析 JSON
            std::string event = j["event"];
            std::string message = j["message"];
            std::string remote_event = j["remote_event"]; // 获取远程事件类型

            if (event == "start_coroutine") {
                spdlog::info("Starting coroutine...");
                //co_await process_event_in_coroutine(message, remote_event);
            } else if (event == "start_thread") {
                spdlog::info("Starting thread...");
                //std::thread t(process_event_in_thread, message);
                //t.detach();
            } else if (event == "start_process") {
                spdlog::info("Starting process...");
                //std::system("python my_script.py");
            } else {
                spdlog::error("Invalid event type: {}", event);
            }
        } catch (const json::exception &e) {
            spdlog::error("Failed to parse JSON: {}", e.what());
        }
      });

    app.port(8080).multithreaded().run(); // 启动 Web 服务器

    return 0;
}
