/*
 * launcher.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Lithium Server Launcher

**************************************************/

#include "launcher.hpp"

#include <cstdio>
#include <regex>
#include <iostream>
#include <openssl/evp.h>

#include "atom/system/crash.hpp"
#include "atom/log/loguru.hpp"

ServerLauncher::ServerLauncher(const std::string &config_file_path, const std::string &DLOG_File_path)
    : _config_file_path(config_file_path), _DLOG_File_path(DLOG_File_path)
{
    try
    {
        // 加载配置文件
        load_config();
    }
    catch (const std::exception &e)
    {
        DLOG_F(ERROR, "Failed to initialize ServerLauncher: {}", e.what());
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
            DLOG_F(INFO, "Some resource files are missing, downloading now...");
            download_resources();
        }

        check_dependencies();

        check_config_file(_config_file_path);

        // 启动服务器
        start_server();

        // 读取服务器输出
        read_server_output();

        // 发送停止命令给服务器，并等待服务器退出
        stop_server();

        // 等待服务器退出
        wait_for_server_to_exit();

        DLOG_F(INFO, "Server stopped.");
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in ServerLauncher::run(): {}", e.what());
        throw;
    }
}

void ServerLauncher::stop()
{
    // 设置请求停止服务器的标志
    _stop_requested = true;

    // 唤醒服务器条件变量，以便服务器检测到停止请求
    _server_cv.notify_all();

    DLOG_F(INFO, "Stop command sent to server.");
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
        LOG_F(ERROR, "Failed to open config file: {}", _config_file_path);
        throw std::runtime_error("Failed to open config file: " + _config_file_path);
    }
    try
    {
        config_file >> _config;
        DLOG_F(INFO, "Config file loaded successfully.");
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred when reading config file: {}", e.what());
        throw;
    }
}

bool ServerLauncher::check_resources()
{
    const auto &resources = _config["resources"];

    for (const auto &res_file : resources)
    {
        const std::string filename = res_file;

        if (!fs::exists(filename))
        {
            LOG_F(ERROR, "Resource file '{}' is missing.", filename);
            return false;
        }

        // 计算 SHA256 值
        std::string sha256_val;
        if (!calculate_sha256(filename, sha256_val))
        {
            LOG_F(ERROR, "Failed to calculate SHA256 value of '" << filename << "'." << std::endl;
            return false;
        }

        const std::string expected_sha256 = res_file["sha256"];
        if (sha256_val != expected_sha256)
        {
            LOG_F(ERROR, "SHA256 check failed for '" << filename << "'." << std::endl);
            return false;
        }
    }

    DLOG_F(INFO, "All resource files are found.");
    return true;
}

void ServerLauncher::download_resources()
{
    DLOG_F(INFO, "Downloading missing resources...");

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
                    LOG_F(ERROR, "Failed to download resource: {}", res_file.get<std::string>());
                    return false;
                }

                // 将下载的数据写入文件
                std::ofstream outfile(res_file);
                outfile.write(res->body.c_str(), res->body.size());

                DLOG_F(INFO,"Resource file '{}' downloaded.", res_file.get<std::string>());
                return true;
            }
            catch (const std::exception &e) {
                LOG_F(ERROR, "Error occurred when downloading resource '{}: {}" ,res_file.get<std::string>(), << e.what());
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
            LOG_F(ERROR, "Failed to download some resources.");
        }
    }

    DLOG_F(INFO, "Downloading finished.");
}

bool check_process(const std::string &name)
{
#ifdef _WIN32
    std::string command = "tasklist /FI \"IMAGENAME eq " + name + "\"";

    // 创建匿名管道
    SECURITY_ATTRIBUTES saAttr;
    HANDLE hReadPipe, hWritePipe;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
    {
        return false;
    }

    // 设置命令行输出重定向到管道
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFOA);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // 启动命令行进程
    if (!CreateProcessA(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return false;
    }

    // 关闭写入端，避免读取阻塞
    CloseHandle(hWritePipe);

    // 读取命令行的输出结果
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    DWORD bytesRead = 0;
    std::string output;

    while (ReadFile(hReadPipe, buffer, BUFFER_SIZE - 1, &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            break;
        }

        buffer[bytesRead] = '\0';
        output += buffer;
    }

    // 关闭管道和进程句柄
    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // 检查输出结果中是否包含进程名
    return (output.find(name) != std::string::npos);
#else
    std::string command = "ps aux | grep -v grep | grep -q '" + name + "'";
    return (system(command.c_str()) == 0);
#endif
}

bool ServerLauncher::check_dependencies()
{
    const std::vector<std::string> dependencies = {"redis-server", "mysqld"};

    for (const auto &dependency : dependencies)
    {
        if (!check_process(dependency))
        {
            DLOG_F(INFO, "Dependency process '{}' is not running.", dependency);
            return false;
        }
    }
    DLOG_F(INFO, "All dependencies are ready.");
    return true;
}

bool ServerLauncher::check_config_file(const std::string &config_file)
{
    if (!std::filesystem::exists(config_file))
    {
        LOG_F(ERROR, "Config file not found: {}", config_file);
        return false;
    }

    try
    {
        std::ifstream ifs(config_file);
        json config = json::parse(ifs);

        // 检查 "port" 配置项是否存在
        if (config.find("port") == config.end())
        {
            LOG_F(ERROR, "Config item 'port' not found in config file.");
            return false;
        }

        // 检查 "port" 配置项是否合法
        int port = config["port"].get<int>();
        if (port < 0 || port > 65535)
        {
            LOG_F(ERROR, "Invalid 'port' configuration value: {}", port);
            return false;
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to parse config file: {}", e.what());
        return false;
    }

    return true;
}

bool ServerLauncher::check_modules(const std::string &modules_dir, const json &module_list)
{
    if (!std::filesystem::exists(modules_dir))
    {
        DLOG_F(INFO, "Modules directory not found: {}", modules_dir);
        if (!std::filesystem::create_directory(modules_dir))
        {
            LOG_F(ERROR, "Failed to create modules directory: {}", modules_dir);
            return false;
        }
    }

    bool all_found = true;
    for (const auto &module : module_list)
    {
        std::string module_path = modules_dir + "/" + module.get<std::string>();
        if (!std::filesystem::exists(module_path))
        {
            DLOG_F(ERROR, "Required module not found: {}", module_path);
            all_found = false;
        }
    }

    return all_found;
}

void ServerLauncher::start_server()
{
    DLOG_F(INFO, "Starting server...");

    // 执行启动服务器的命令
    const std::string cmd = _config["server_command"];

    _server_process = std::shared_ptr<FILE>(_popen(cmd.c_str(), "r"), [](FILE *f)
                                            { if (f) { _pclose(f); } });

    if (!_server_process)
    {
        DLOG_F(ERROR, "Failed to execute server command: {}", cmd);
        throw std::runtime_error("Failed to execute server command: " + cmd);
    }
    else
    {
        DLOG_F(INFO, "Server process started with command: {}", cmd);
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
            DLOG_F(INFO,"Stop command sent to server process.");
        } });

    DLOG_F(INFO, "Server started.");
}

void ServerLauncher::stop_server()
{
    DLOG_F(INFO, "Stopping server...");

    // 发送停止命令给服务器
    fprintf(_server_process.get(), "%c", _config["stop_command"]);
    fflush(_server_process.get());

    DLOG_F(INFO, "Stop command sent to server process.");
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
                    //Lithium::CrashReport::saveCrashLog(error_message);
                }
                else if (error_type == "WARNING") {
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

bool ServerLauncher::calculate_sha256(const std::string &filename, std::string &sha256_val)
{
    // 打开文件
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        return false;
    }

    // 计算 SHA256 值
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)))
    {
        EVP_DigestUpdate(mdctx, buffer, sizeof(buffer));
    }

    if (file.gcount() > 0)
    {
        EVP_DigestUpdate(mdctx, buffer, file.gcount());
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    // 转换为十六进制字符串
    sha256_val.clear();
    for (unsigned int i = 0; i < hash_len; ++i)
    {
        char hex_str[3];
        sprintf(hex_str, "%02x", hash[i]);
        sha256_val += hex_str;
    }

    return true;
}

void setupLogFile()
{
    std::filesystem::path logsFolder = std::filesystem::current_path() / "logs";
    if (!std::filesystem::exists(logsFolder))
    {
        std::filesystem::create_directory(logsFolder);
    }
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time_t);
    char filename[100];
    std::strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.log", local_time);
    std::filesystem::path logFilePath = logsFolder / filename;
    loguru::add_file(logFilePath.string().c_str(), loguru::Append, loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message)
                              { Lithium::CrashReport::saveCrashLog(std::string(message.prefix) + message.message); });
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.size() < 2)
    {
        LOG_F(INFO, "Error: Missing arguments.");
        LOG_F(INFO, "Usage: launcher <config file> <log file>");
        return 1;
    }

    try
    {
        ServerLauncher launcher(args[0], args[1]);
        launcher.run();

        if (launcher.is_running())
        {
            launcher.stop();
        }
    }
    catch (const std::exception &e)
    {
        // 输出错误信息并返回
        LOG_F(ERROR, "Error: {}", e.what());
        return 1;
    }

    return 0;
}