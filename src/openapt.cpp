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

#include "nlohmann/json.hpp"

using json = nlohmann::json;

OpenAPT::ThreadManager m_ThreadManager;
OpenAPT::TaskManager m_TaskManager;
OpenAPT::DeviceManager m_DeviceManager;

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

int main() {

    LoadUrl();

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
