/*
 * launcher.hpp
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

Date: 2023-3-29

Description: OpenAPT Server Launcher

**************************************************/

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <future>
#include <queue>
#include <cpp_httplib/httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

class ServerLauncher
{
public:
    ServerLauncher(const std::string &config_file_path, const std::string &log_file_path);

    void run();

    void stop();

    bool is_running() const;

    template <typename OStream>
    friend OStream &operator<<(OStream &os, const ServerLauncher &launcher)
    {
        os << "Config file path: " << launcher._config_file_path << "\n";
        os << "Log file path: " << launcher._log_file_path << "\n";
        os << "Stop requested: " << std::boolalpha << launcher._stop_requested << "\n";
        os << "Server running: " << std::boolalpha << launcher._server_running << "\n";
        return os;
    }

private:
    void load_config();

    bool check_resources();

    void download_resources();

    void start_server();

    void stop_server();

    void wait_for_server_to_exit();

    void read_server_output();

    void send_warning_email(const std::string& message);

    std::string _config_file_path;
    std::string _log_file_path;
    json _config;
    std::atomic_bool _stop_requested = false;
    std::atomic_bool _server_running = false;
    std::jthread _server_thread;
    std::mutex _server_mutex;
    std::condition_variable _server_cv;
    std::shared_ptr<FILE> _server_process;
};

class ThreadPool
{
public:
    ThreadPool(std::size_t n_threads)
        : stop(false)
    {
        for (std::size_t i = 0; i < n_threads; ++i)
        {
            threads.emplace_back([this]
                                 {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                } });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &thread : threads)
        {
            thread.join();
        }
    }

    template <typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task]
                          { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
