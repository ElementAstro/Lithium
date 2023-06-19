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

/**
 * @brief 一个服务器启动器类，用于管理和启动服务器，包括资源下载、依赖检查、配置文件解析等操作。
 */
class ServerLauncher
{
public:
    /**
     * @brief 构造函数，初始化配置文件路径和日志文件路径，但不进行任何初始化操作。
     * @param config_file_path 服务器配置文件路径
     * @param log_file_path 服务器日志文件路径
     */
    ServerLauncher(const std::string &config_file_path, const std::string &log_file_path);

    /**
     * @brief 启动服务器并开始监听连接请求。
     *
     * 该函数首先检查运行所需的资源是否已下载和依赖项是否已安装，然后加载配置文件，
     * 启动服务器进程并开始监听连接请求。启动过程中，服务器输出将定向至日志文件中。
     * 
     * 如果启动失败，则会发送警告邮件，并打印相关错误并退出程序。
     * 
     * 在 start_server() 函数内部，会创建一个新的线程来启动服务器进程，并等待子进程结束。
     */
    void run();

    /**
     * @brief 停止服务器进程和启动器。
     *
     * 停止服务器进程前会发送停止信号以确保服务器进程优雅地退出。
     */
    void stop();

    /**
     * @brief 检查服务器是否正在运行。
     * @return 如果服务器正在运行，则返回 true，否则返回 false。
     */
    bool is_running() const;

    /**
     * @brief 重定向服务器的标准输出和标准错误输出到日志文件中。
     *
     * 该函数在服务器启动后应当被调用，以便将服务器输出记录到日志文件中。
     * 
     * @param log_file_path 日志文件路径。
     */
    void redirect_stdout_stderr(const std::string &log_file_path);

    /**
     * @brief 计算指定文件的 SHA-256 哈希值。
     * @param filename 要计算哈希值的文件名。
     * @param sha256_val 输出参数，保存计算出的哈希值。
     * @return 如果文件存在并成功计算出哈希值，则返回 true，否则返回 false。
     */
    bool calculate_sha256(const std::string &filename, std::string &sha256_val);

private:
    /**
     * @brief 加载配置文件，并检查其中的参数是否合法。
     *
     * 该函数在 ServerLauncher::run() 函数中被调用，用于加载服务器的配置文件，
     * 并检查其中的参数是否设置正确。如果参数检查失败，则会打印相关错误信息并退出程序。
     */
    void load_config();

    /**
     * @brief 检查启动器所需的资源是否已下载。
     * 
     * 该函数在 ServerLauncher::run() 函数中被调用，用于检查启动器所需的资源是否已下载。
     * 如果没有下载，则会提示用户开始下载，或自动下载并安装缺失的资源。
     *
     * @return 如果所有必要的资源均已下载，则返回 true，否则返回 false。
     */
    bool check_resources();

    /**
     * @brief 下载启动器所需的资源。
     * 
     * 该函数在 ServerLauncher::check_resources() 函数中被调用，用于下载启动器所需的资源，
     * 包括服务器程序、配置文件、数据文件等。下载完成后，将自动解压和安装这些资源。
     */
    void download_resources();

    /**
     * @brief 检查启动器所需的依赖项是否已安装。
     * 
     * 该函数在 ServerLauncher::run() 函数中被调用，用于检查启动器所需的依赖项是否已安装。
     * 如果某些依赖项未安装，则会提示用户开始安装，或自动下载并安装这些依赖项。
     *
     * @return 如果所有必要的依赖项均已安装，则返回 true，否则返回 false。
     */
    bool check_dependencies();

    /**
     * @brief 检查服务器配置文件是否合法。
     * @param config_file 支持 JSON 或 YAML 格式的配置文件路径。
     * @return 如果配置文件有效，则返回 true，否则返回 false。
     */
    bool check_config_file(const std::string &config_file);

    /**
     * @brief 检查已安装的模组是否符合要求。
     * 
     * 该函数在 ServerLauncher::run() 函数中被调用，用于检查已安装的模组是否符合要求，
     * 如果存在不支持或过时的模组，则会提示用户更新或删除这些模组。
     * 
     * @param modules_dir 模组所在的目录路径。
     * @param module_list 从配置文件中读取的模组列表。
     * @return 如果所有必要的模组均已安装并且都是最新版，则返回 true，否则返回 false。
     */
    bool check_modules(const std::string &modules_dir, const json &module_list);

    /**
     * @brief 启动服务器进程并等待其结束。
     *
     * 该函数在 ServerLauncher::run() 函数中被调用，用于启动服务器进程并等待其结束。
     * 在启动服务器进程前，将检查服务器程序是否存在以及配置文件是否设置正确。
     * 若服务器程序不存在或配置文件设置错误，则会输出错误信息并退出程序。
     *
     * 在函数内部，会创建一个新的线程来启动服务器进程，并等待子进程结束。如果服务器进程
     * 提前结束，则会发送警告邮件给管理员，并打印相关错误信息并退出程序。
     */
    void start_server();

    /**
     * @brief 发送警告邮件。
     * 
     * 该函数在 ServerLauncher::run() 函数中被调用，用于发送警告邮件给管理员。
     * 邮件内容包括错误信息以及日志文件的部分内容，以供管理员进行问题排查。
     *
     * @param message 警告信息
     */
    void send_warning_email(const std::string& message);

    /**
     * @brief 发送停止信号给服务器进程，并等待其优雅地退出。
     * 
     * 该函数在 ServerLauncher::stop() 函数中被调用，用于发送停止信号给服务器进程，
     * 并等待其在一段时间内优雅地退出。如果服务器未能在规定时间内退出，则会发送
     * kill 信号以强制结束服务器进程。
     */
    void stop_server();

    /**
     * @brief 等待服务器进程结束。
     * 
     * 该函数在 ServerLauncher::stop_server() 和 ServerLauncher::start_server()
     * 函数中都可能被调用，用于等待服务器进程在子线程中结束。
     * 
     * 通过使用条件变量和互斥锁来控制等待流程，避免了死锁和忙等情况的出现。
     */
    void wait_for_server_to_exit();

    /**
     * @brief 读取服务器输出并将其记录到日志文件中。
     *
     * 该函数在 ServerLauncher::start_server() 中被调用，用于读取服务器进程的标准输出
     * 并将其记录到日志文件中。该函数在一个独立的线程中运行，以避免阻塞其他操作。
     */
    void read_server_output();

    std::string _config_file_path;  ///< 配置文件路径
    std::string _log_file_path;  ///< 日志文件路径
    json _config;  ///< 服务器配置文件
    std::atomic_bool _stop_requested = false;  ///< 是否请求停止服务器的标志
    std::atomic_bool _server_running = false;  ///< 是否正在运行服务器的标志
    std::jthread _server_thread;  ///< 执行服务器进程的线程对象
    std::mutex _server_mutex;  ///< 控制等待服务器进程结束的互斥锁
    std::condition_variable _server_cv;  ///< 控制等待服务器进程结束的条件变量
    std::shared_ptr<FILE> _server_process;  ///< 指向服务器进程标准输出流的指针
};

/**
 * @brief 线程池
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数，初始化线程池大小和停止标志位。
     *
     * 构造函数会创建 n_threads 个线程，并等待来自任务队列的任务分配。
     *
     * @param n_threads 线程池大小
     */
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

    /**
     * @brief 析构函数，销毁所有线程并退出。
     *
     * 析构函数会向任务队列中插入空任务，并等待所有线程完成该任务并退出。
     */
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

    /**
     * @brief 将指定任务添加到任务队列中，并返回该任务的 future 对象。
     *
     * 该函数用于将函数 f 和其参数 args 添加到任务队列中等待执行，并返回一
     * 个 std::future 对象，以便查询任务完成情况。当任务队列已满或线程池被
     * 停止时，将会抛出 std::runtime_error 异常。
     *
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数对象
     * @param args 函数参数
     * @return 返回一个 std::future 对象，用于查询任务完成情况
     */
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
    std::vector<std::thread> threads;          ///< 线程池中的线程列表
    std::queue<std::function<void()>> tasks;   ///< 任务队列

    std::mutex queue_mutex;                  ///< 任务队列的互斥锁
    std::condition_variable condition;       ///< 任务队列的条件变量
    bool stop;                               ///< 停止标志位
};
