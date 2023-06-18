/*
 * launcher.cpp
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

#include "launcher.hpp"
#include "crash.hpp"

#include <cstdio>
#include <regex>
#include <boost/asio.hpp>
#include <iostream>

ServerLauncher::ServerLauncher(const std::string &config_file_path, const std::string &log_file_path)
    : _config_file_path(config_file_path), _log_file_path(log_file_path)
{
    try
    {
        // 初始化日志
        ////spdlog::set_default_logger(//spdlog::basic_logger_mt("server_launcher", log_file_path));

        // 加载配置文件
        load_config();
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to initialize ServerLauncher: " << e.what() << std::endl;
        throw;
    }
}

void ServerLauncher::run()
{
    try
    {
        // 检查服务器所需的资源文件是否存在
        if (!check_resources())
        {
            // spdlog::info("Some resource files are missing, downloading now...");
            std::cout << "Some resource files are missing, downloading now..." << std::endl;
            // 如果不完整则下载缺失的资源文件
            download_resources();
        }

        // 启动服务器
        start_server();

        // 读取服务器输出
        read_server_output();

        // 发送停止命令给服务器，并等待服务器退出
        stop_server();

        // 等待服务器退出
        wait_for_server_to_exit();

        // spdlog::info("Server stopped.");
        std::cout << "Server stopped." << std::endl;
    }
    catch (const std::exception &e)
    {
        // spdlog::error("Error occurred in ServerLauncher::run(): {}", e.what());
        std::cout << "Error occurred in ServerLauncher::run(): " << e.what() << std::endl;
        throw;
    }
}

void ServerLauncher::stop()
{
    // 设置请求停止服务器的标志
    _stop_requested = true;

    // 唤醒服务器条件变量，以便服务器检测到停止请求
    _server_cv.notify_all();

    // spdlog::info("Stop command sent to server.");
    std::cout << "Stop command sent to server." << std::endl;
}

bool ServerLauncher::is_running() const
{
    return _server_running;
}

void ServerLauncher::load_config()
{
    std::ifstream config_file(_config_file_path);
    if (!config_file)
    {
        throw std::runtime_error("Failed to open config file: " + _config_file_path);
    }
    try
    {
        config_file >> _config;
        // spdlog::info("Config file loaded successfully.");
        std::cout << "Config file loaded successfully." << std::endl;
    }
    catch (const std::exception &e)
    {
        // spdlog::error("Error occurred when reading config file: {}", e.what());
        std::cout << "Error occurred when reading config file: " << e.what() << std::endl;
        throw;
    }
}

bool ServerLauncher::check_resources()
{
    for (const auto &res_file : _config["resources"])
    {
        if (!fs::exists(res_file))
        {
            // spdlog::debug("Resource file '{}' is missing.", res_file.get<std::string>());
            std::cout << "Resource file '" << res_file.get<std::string>() << "' is missing." << std::endl;
            return false;
        }
    }
    // spdlog::info("All resource files are found.");
    std::cout << "All resource files are found." << std::endl;
    return true;
}

void ServerLauncher::download_resources()
{
    // spdlog::info("Downloading missing resources...");
    std::cout << "Downloading missing resources..." << std::endl;

    // 创建线程池
    ThreadPool pool(std::thread::hardware_concurrency());

    // 创建任务列表
    std::vector<std::future<bool>> tasks;

    for (const auto &res_file : _config["resources"])
    {
        // 发送 HTTP GET 请求下载文件
        const std::string url = _config["resource_server"].get<std::string>() + "/" + res_file.get<std::string>();

        // 添加下载任务到线程池
        tasks.emplace_back(pool.enqueue([url, res_file, this]
                                        {
            try {
                httplib::Client client(_config["resource_server"]);
                auto res = client.Get(url.c_str());

                if (!res) {
                    //spdlog::error("Failed to download resource: {}", res_file.get<std::string>());
                    std::cout << "Failed to download resource: " << res_file.get<std::string>() << std::endl;
                    return false;
                }

                // 将下载的数据写入文件
                std::ofstream outfile(res_file);
                outfile.write(res->body.c_str(), res->body.size());

                //spdlog::info("Resource file '{}' downloaded.", res_file.get<std::string>());
                std::cout << "Resource file '" << res_file.get<std::string>() << "' downloaded." << std::endl;
                return true;
            }
            catch (const std::exception &e) {
                //spdlog::error("Error occurred when downloading resource '{}': {}", res_file.get<std::string>(), e.what());
                std::cout << "Error occurred when downloading resource '" << res_file.get<std::string>() << "': " << e.what() << std::endl;
                return false;
            } }));
    }

    // 等待所有任务完成
    for (auto &&task : tasks)
    {
        task.wait();
    }

    // 检查是否有任务失败
    for (auto &&task : tasks)
    {
        if (!task.get())
        {
            throw std::runtime_error("Failed to download some resources.");
        }
    }

    // spdlog::info("Downloading finished.");
    std::cout << "Downloading finished." << std::endl;
}

void ServerLauncher::start_server()
{
    // spdlog::info("Starting server...");
    std::cout << "Starting server..." << std::endl;

    // 执行启动服务器的命令
    const std::string cmd = _config["server_command"];

    _server_process = std::shared_ptr<FILE>(_popen(cmd.c_str(), "r"), [](FILE *f)
                                            { if (f) { _pclose(f); } });

    if (!_server_process)
    {
        throw std::runtime_error("Failed to execute server command: " + cmd);
    }
    else
    {
        // spdlog::info("Server process started with command: {}", cmd);
        std::cout << "Server process started with command: " << cmd << std::endl;
    }

    // 创建一个线程来等待服务器启动
    _server_thread = std::jthread([&]
                                  {
        std::unique_lock<std::mutex> lock(_server_mutex);

        while (!_stop_requested) {
            // 在条件变量上等待，直到服务器启动
            _server_cv.wait(lock, [&] { return _server_running || _stop_requested; });
        }

        // 如果请求停止服务器，则发送停止命令给服务器
        if (_stop_requested) {
            fprintf(_server_process.get(), "%c", _config["stop_command"]);
            fflush(_server_process.get());
            //spdlog::debug("Stop command sent to server process.");
            std::cout << "Stop command sent to server process." << std::endl;
        } });

    // spdlog::info("Server started.");
    std::cout << "Server started." << std::endl;
}

void ServerLauncher::stop_server()
{
    // spdlog::info("Stopping server...");
    std::cout << "Stopping server..." << std::endl;

    // 发送停止命令给服务器
    fprintf(_server_process.get(), "%c", _config["stop_command"]);
    fflush(_server_process.get());

    // spdlog::info("Stop command sent to server process.");
    std::cout << "Stop command sent to server process." << std::endl;
}

void ServerLauncher::wait_for_server_to_exit()
{
    // 等待服务器退出并获取返回值
    int status = -1;
    _server_thread.join();
    _server_process.reset();
    _server_running = false;
}

void ServerLauncher::read_server_output()
{
    // 定义正则表达式模板，匹配错误信息
    std::regex error_regex("ERROR: \\[(\\S+)\\] (.*)");

    // 创建一个线程来读取服务器输出
    std::thread read_thread([&]
                            {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), _server_process.get())) {
            std::cout << buffer;

            // 判断输出中是否包含错误信息
            std::string line(buffer);
            std::smatch match;

            if (std::regex_search(line, match, error_regex)) {
                // 匹配成功，提取错误类型和错误消息
                std::string error_type = match[1].str();
                std::string error_message = match[2].str();

                // 根据错误类型处理错误
                if (error_type == "CRITICAL") {
                    // 生成冲突日志
                    //OpenAPT::CrashReport::saveCrashLog(error_message);
                }
                else if (error_type == "WARNING") {
                    // 发送警告邮件
                    send_warning_email(error_message);
                }

                // 读取结束，设置服务器运行标志为 false
                _server_running = false;

                // 唤醒等待服务器退出的条件变量
                _server_cv.notify_all();
                return;
            }
        }

        // 读取结束，设置服务器运行标志为 false
        _server_running = false;

        // 唤醒等待服务器退出的条件变量
        _server_cv.notify_all(); });

    // 启动成功，设置服务器运行标志为 true
    _server_running = true;

    // 让分离线程自行运行，不阻塞 run() 函数
    read_thread.detach();
}

void ServerLauncher::send_warning_email(const std::string &message)
{
    std::string smtp_server = "smtp.example.com";
    int smtp_port = 25;
    std::string from_address = "noreply@example.com";
    std::string to_address = "admin@example.com";
    std::string subject = "Server Warning";
    std::string body = "Warning message:\n" + message;

    // 使用 Boost.Asio 进行 SMTP 邮件发送
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::socket socket(io_service);
    boost::asio::connect(socket, resolver.resolve({smtp_server, std::to_string(smtp_port)}));
    std::string response;

    // 接收服务端欢迎信息
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送 HELO 命令
    socket.send(boost::asio::buffer("HELO example.com\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送 MAIL FROM 命令
    socket.send(boost::asio::buffer("MAIL FROM:<" + from_address + ">\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送 RCPT TO 命令
    socket.send(boost::asio::buffer("RCPT TO:<" + to_address + ">\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送 DATA 命令
    socket.send(boost::asio::buffer("DATA\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送邮件头部
    std::string header =
        "From: " + from_address + "\r\n"
                                  "To: " +
        to_address + "\r\n"
                     "Subject: " +
        subject + "\r\n"
                  "Content-Type: text/plain; charset=utf-8\r\n"
                  "\r\n";
    socket.send(boost::asio::buffer(header));

    // 发送邮件正文
    socket.send(boost::asio::buffer(body));

    // 发送结束符和换行符
    socket.send(boost::asio::buffer("\r\n.\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");

    // 发送 QUIT 命令并关闭连接
    socket.send(boost::asio::buffer("QUIT\r\n"));
    boost::asio::read_until(socket, boost::asio::dynamic_buffer(response), "\n");
    socket.close();

    std::cout << "Sent warning email: " << message << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <config file> <log file>\n";
        return 1;
    }

    try
    {
        ServerLauncher launcher(argv[1], argv[2]);
        launcher.run();

        // 模拟运行一段时间后停止服务器
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if (launcher.is_running())
        {
            launcher.stop();
        }
    }
    catch (const std::exception &e)
    {
        // spdlog::error("Error occurred in main function: {}", e.what());
        return 1;
    }

    return 0;
}