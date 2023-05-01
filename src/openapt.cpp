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

/*

   ____                   ___    ____  ______
  / __ \____  ___  ____  /   |  / __ \/_  __/
 / / / / __ \/ _ \/ __ \/ /| | / /_/ / / /
/ /_/ / /_/ /  __/ / / / ___ |/ ____/ / /
\____/ .___/\___/_/ /_/_/  |_/_/     /_/
    /_/

    __  ___      __           ___         __                   __          __                               __             ______           _ __
   /  |/  /___ _/ /_____     /   |  _____/ /__________  ____  / /_  ____  / /_____  ____ __________ _____  / /_  __  __   / ____/___ ______(_) /_  __
  / /|_/ / __ `/ //_/ _ \   / /| | / ___/ __/ ___/ __ \/ __ \/ __ \/ __ \/ __/ __ \/ __ `/ ___/ __ `/ __ \/ __ \/ / / /  / __/ / __ `/ ___/ / / / / /
 / /  / / /_/ / ,< /  __/  / ___ |(__  ) /_/ /  / /_/ / /_/ / / / / /_/ / /_/ /_/ / /_/ / /  / /_/ / /_/ / / / / /_/ /  / /___/ /_/ (__  ) / / /_/ /
/_/  /_/\__,_/_/|_|\___/  /_/  |_/____/\__/_/   \____/ .___/_/ /_/\____/\__/\____/\__, /_/   \__,_/ .___/_/ /_/\__, /  /_____/\__,_/____/_/_/\__, /
                                                    /_/                          /____/          /_/          /____/                        /____/
*/

#include "openapt.hpp"
#include "plugins/crash.hpp"
#include "plugins/terminal.hpp"

#include <spdlog/spdlog.h>

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
#include <stdexcept>
#include <unistd.h>
#include <getopt.h>

#ifdef _WIN32
#include <winsock2.h> // Windows socket API
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#endif

#include "nlohmann/json.hpp"

#include "webapi/http_api.hpp"

// 这些引用都只是为了测试用的
#include "device/basic_device.hpp"
#include "task/define.hpp"
#include "config/achievement.hpp"
#include "config/achievement_list.hpp"
#include "module/compiler.hpp"
#include "asx/search.hpp"
#include "api/astrometry.hpp"
#include "task/camera_task.hpp"
#include "indi/indicamera.hpp"
#include "asx/search.hpp"
#include "module/sheller.hpp"

using json = nlohmann::json;

MyApp m_App;

bool DEBUG = true;

void print_help(int argc, char *argv[])
{
    std::cout << "Usage: " << argv[0] << " [-d|--debug] [-p|--port PORT] [-s|--ssl] [-f|--certfile FILE] [-k|--keyfile FILE] [-c|--config CONFIG_FILE]\n"
          << "Options:\n"
          << " -d, --debug Enable debug mode\n"
          << " -p, --port PORT Specify listening port (default is 8080)\n"
          << " -s, --ssl Enable SSL mode\n"
          << " -f, --certfile FILE Specify certificate file (default is cert.pem)\n"
          << " -k, --keyfile FILE Specify key file (default is key.pem)\n"
          << " -c, --config CONFIG_FILE Use custom config file (default is config.json)\n";
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char *argv[])
{
    int opt;
    int option_index = 0;
    const char *short_opts = "dp:sf:k:c:";
    const option long_opts[] = {
        {"debug", no_argument, nullptr, 'd'},
        {"port", required_argument, nullptr, 'p'},
        {"ssl", no_argument, nullptr, 's'},
        {"certfile", required_argument, nullptr, 'f'},
        {"keyfile", required_argument, nullptr, 'k'},
        {"config", required_argument, nullptr, 'c'},
        {nullptr, 0, nullptr, 0}};

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'd':
            DEBUG = true;
            m_App.GetConfigManager()->setValue("server/debug", true);
            spdlog::info("DEBUG Mode is enabled by command line argument");
            break;
        case 'p':
            m_App.GetConfigManager()->setValue("server/port", atoi(optarg));
            spdlog::info("Listening port is set to {}", optarg);
            break;
        case 's':
            m_App.GetConfigManager()->setValue("server/ssl", true);
            spdlog::info("SSL is enabled by command line argument");
            break;
        case 'f':
            spdlog::info("Certificate file is set to {}", optarg);
            break;
        case 'k':
            spdlog::info("Key file is set to {}", optarg);
            break;
        case 'c':
            m_App.GetConfigManager()->setValue("server/config", optarg);
            spdlog::info("Config file is set to {}", optarg);
            break;
        case '?':
            print_help(argc, argv);
            break;

        default:
            break;
        }
    }

    if (optind < argc)
    {
        // process remaining non-option arguments
    }
}


void quit();

#ifdef _WIN32
    // Define the signal handler function for Windows platform
    BOOL WINAPI interruptHandler(DWORD signalNumber)
    {
        if (signalNumber == CTRL_C_EVENT)
        {
            spdlog::info("Keyboard interrupt received.");
        }
        return TRUE;
    }

#else
    // Define the signal handler function
    void interruptHandler(int signalNumber, siginfo_t *info, void *context)
    {
        spdlog::info("Keyboard interrupt received.");
        quit();
    }
#endif

void registerInterruptHandler()
{
#ifdef _WIN32
    // Register the signal handler function to deal with Ctrl+C on Windows platform
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)interruptHandler, TRUE);
#else
    // Register the signal handler function to deal with SIGINT on non-Windows platforms
    struct sigaction sa;
    sa.sa_sigaction = &interruptHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaction(SIGINT, &sa, NULL);
#endif
}

bool CheckAndKillProgramOnPort(int port)
{
#ifdef OS_Windows
    // 初始化 Windows socket API
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        std::cerr << "Failed to initialize Windows Socket API: " << ret << std::endl;
        return false;
    }
#endif

    // 创建一个新的套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
#ifdef OS_Windows
        WSACleanup();
#endif
        return false;
    }

    // 绑定到指定端口上
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        if (errno == EADDRINUSE)
        {
            std::cerr << "The port(" << port << ") is already in use" << std::endl;

            // 获取占用端口的进程 ID
            std::string cmd;
#ifdef OS_Windows
            cmd = fmt::format("netstat -ano | find \"LISTENING\" | find \"{0}\"", port);
#else
            cmd = fmt::format("lsof -i :{} -t", port);
#endif

            FILE *fp = popen(cmd.c_str(), "r");
            if (fp == nullptr)
            {
                std::cerr << "Failed to execute command: " << cmd << std::endl;
                close(sockfd);
#ifdef OS_Windows
                WSACleanup();
#endif
                return false;
            }

            char buf[1024];
            std::string pid_str;
            while (fgets(buf, 1024, fp) != nullptr)
            {
                pid_str += buf;
            }
            pclose(fp);
            pid_str.erase(pid_str.find_last_not_of("\n") + 1);

            // 如果获取到了 PID，则杀死该进程
            if (!pid_str.empty())
            {
                std::cout << "Killing the process on port(" << port << "): PID=" << pid_str << std::endl;
#ifdef OS_Windows
                ret = std::system(fmt::format("taskkill /F /PID {}", pid_str).c_str());
#else
                int ret = std::system(fmt::format("kill {}", pid_str).c_str());
#endif
                if (ret != 0)
                {
                    std::cerr << "Failed to kill the process: " << pid_str << std::endl;
                    close(sockfd);
#ifdef OS_Windows
                    WSACleanup();
#endif
                    return false;
                }
                std::cout << "The process(" << pid_str << ") is killed successfully" << std::endl;
            }
            else
            {
                std::cerr << "Failed to get process ID on port(" << port << ")" << std::endl;
                close(sockfd);
#ifdef OS_Windows
                WSACleanup();
#endif
                return false;
            }
        }
        else
        {
            std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
            close(sockfd);
#ifdef OS_Windows
            WSACleanup();
#endif
            return false;
        }
    }

    close(sockfd);
#ifdef OS_Windows
    WSACleanup();
#endif
    return true;
}

// 判断是否有相同的程序在运行，并杀死前一个程序
void check_duplicate_process(const std::string &program_name)
{
#ifdef _WIN32 // Windows平台
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        spdlog::error("CreateToolhelp32Snapshot failed: {}", GetLastError());
        exit(EXIT_FAILURE);
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    BOOL bRet = Process32First(hSnapshot, &pe);
    while (bRet)
    {
        std::string name = pe.szExeFile;
        if (name == program_name)
        {
            spdlog::warn("Found duplicate {} process with PID {}", program_name, pe.th32ProcessID);
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (hProcess == NULL)
            {
                spdlog::error("OpenProcess failed: {}", GetLastError());
                exit(EXIT_FAILURE);
            }
            if (!TerminateProcess(hProcess, 0))
            {
                spdlog::error("TerminateProcess failed: {}", GetLastError());
                exit(EXIT_FAILURE);
            }
            CloseHandle(hProcess);
            break;
        }
        bRet = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
#else // Linux和macOS平台
    DIR *dirp = opendir("/proc");
    if (dirp == NULL)
    {
        spdlog::error("Cannot open /proc directory");
        exit(EXIT_FAILURE);
    }

    std::vector<pid_t> pids;
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        if (!isdigit(dp->d_name[0]))
        {
            continue;
        }
        pid_t pid = atoi(dp->d_name);
        char cmdline_file[256];
        snprintf(cmdline_file, sizeof(cmdline_file), "/proc/%d/cmdline", pid);

        FILE *cmd_file = fopen(cmdline_file, "r");
        if (cmd_file)
        {
            char cmdline[1024];
            if (fgets(cmdline, sizeof(cmdline), cmd_file) == NULL)
            {
                spdlog::error("Failed to get pids");
            }
            fclose(cmd_file);
            std::string name = cmdline;
            if (name == program_name)
            {
                pids.push_back(pid);
            }
        }
    }
    closedir(dirp);

    if (pids.size() <= 1)
    {
        spdlog::info("No duplicate {} process found", program_name);
        return;
    }

    for (auto pid : pids)
    {
        spdlog::warn("Found duplicate {} process with PID {}", program_name, pid);
        if (kill(pid, SIGTERM) != 0)
        {
            spdlog::error("kill failed: {}", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
#endif
}

// 判断是否连接网络
bool is_network_connected()
{
    try
    {
#ifdef _WIN32 // Windows平台
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            spdlog::error("WSAStartup failed: {}", result);
            return false;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            spdlog::error("socket failed: {}", WSAGetLastError());
            return false;
        }

        struct sockaddr_in saServer;
        saServer.sin_family = AF_INET;
        saServer.sin_addr.s_addr = inet_addr("8.8.8.8");
        saServer.sin_port = htons(53); // DNS端口

        int nRet = connect(sock, (struct sockaddr *)&saServer, sizeof(saServer));
        closesocket(sock);
        if (nRet == SOCKET_ERROR)
        {
            spdlog::error("connect failed: {}", WSAGetLastError());
            return false;
        }
        return true;
#else // Linux和macOS平台
        addrinfo hint, *res;
        std::memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family = AF_UNSPEC;
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_CANONNAME;

        if (getaddrinfo("www.baidu.com", "http", &hint, &res) != 0)
        {
            spdlog::error("getaddrinfo failed: {}", strerror(errno));
            return false;
        }

        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == -1)
        {
            spdlog::error("socket failed: {}", strerror(errno));
            freeaddrinfo(res);
            return false;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0)
        {
            spdlog::error("connect failed: {}", strerror(errno));
            close(sock);
            freeaddrinfo(res);
            return false;
        }

        close(sock);
        freeaddrinfo(res);

        spdlog::info("Network checked, connected!");
        return true;
#endif
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
        return false;
    }
}

// 判断操作系统类型和平台，然后进行相应的检测
void platform_check()
{
#ifdef _WIN32 // Windows平台
    check_duplicate_process("openapt.exe");
    is_network_connected();
#else // Linux和macOS平台
    check_duplicate_process("openapt");
    is_network_connected();
#endif
}

int square(int n) { return n * n; }

void TestAll()
{
    spdlog::debug("ModuleManager Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing ModuleLoader and some important functions:");
    spdlog::debug("Load module: {}", m_App.GetModuleLoader()->LoadModule("modules/test/libmylib.so", "mylib"));
    spdlog::debug("Load and run function: ");
    m_App.GetModuleLoader()->LoadAndRunFunction<void>("mylib", "my_func", "test", false);
    spdlog::debug("HasModule Testing: ");
    spdlog::debug("Check if module 'fuckyou' exists: {}", m_App.GetModuleLoader()->HasModule("fuckyou"));
    spdlog::debug("Finished testing ModuleLoader");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("TaskManager Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing SimpleTask:");
    auto simpleTask = m_App.GetTaskManager()->getGenerator()->generateSimpleTask("simpleTask", "Just a test", {}, "", "Print");
    m_App.GetTaskManager()->addTask(simpleTask);
    spdlog::debug("SimpleTask added");
    spdlog::debug("Testing ConditionalTask:");
    auto conditionalTask = m_App.GetTaskManager()->getGenerator()->generateConditionalTask("conditionalTask", "A test conditional task", {{"status", 2}});
    m_App.GetTaskManager()->addTask(conditionalTask);
    spdlog::debug("ConditionalTask added");
    spdlog::debug("Execute all tasks:");
    m_App.GetTaskManager()->executeAllTasks();
    spdlog::debug("Finished testing TaskManager");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("DeviceManager Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing addDevice and getDeviceList:");
    m_App.GetDeviceManager()->addDevice(OpenAPT::DeviceType::Camera, "CCD Simulator");
    auto cameraList = m_App.GetDeviceManager()->getDeviceList(OpenAPT::DeviceType::Camera);
    for (auto &name : cameraList)
        spdlog::debug("Found Camera name {}", name);
    spdlog::debug("Testing findDeviceByName:");
    auto device1 = m_App.GetDeviceManager()->findDeviceByName("CCD Simulator");
    if (device1 != nullptr)
    {
        spdlog::debug("Connecting to device {}...", device1->getName());
        auto connectResult = device1->connect("CCD Simulator");
        if (true)
        {
            auto camera = std::dynamic_pointer_cast<OpenAPT::INDICamera>(device1);
            if (camera)
            {
                spdlog::debug("Found device {} as a Camera", device1->getName());
                spdlog::debug("Testing captureImage:");
                m_App.GetTaskManager()->addTask(camera->getSimpleTask("SingleShot", {}));
                m_App.GetTaskManager()->addTask(m_App.GetDeviceManager()->getSimpleTask(OpenAPT::DeviceType::Camera, "INDI", "CCD Simulator", "SingleShot",{}));
                m_App.GetTaskManager()->addTask(m_App.GetDeviceManager()->getSimpleTask(OpenAPT::DeviceType::Camera, "INDI", "CCD Simulator", "GetGain",{}));
                m_App.GetTaskManager()->executeAllTasks();
            }
            else
            {
                spdlog::error("Device {} is not a Camera", device1->getName());
            }
        }
        else
        {
            spdlog::error("Failed to connect to device {}", device1->getName());
        }
    }
    else
    {
        spdlog::error("Can't find device CCD Simulator");
    }
    m_App.GetDeviceManager()->addDevice(OpenAPT::DeviceType::Focuser, "Focuser Simulator");
    auto focuserList = m_App.GetDeviceManager()->getDeviceList(OpenAPT::DeviceType::Focuser);
    for (auto &name : focuserList)
        spdlog::debug("Found Focuser name {}", name);
    spdlog::debug("Testing findDeviceByName:");
    auto device2 = m_App.GetDeviceManager()->findDeviceByName("Focuser Simulator");
    if (device2 != nullptr) {
        device2->connect("Focuser Simulator");
        m_App.GetTaskManager()->addTask(m_App.GetDeviceManager()->getSimpleTask(OpenAPT::DeviceType::Focuser, "INDI", "Focuser Simulator", "MoveToAbsolute",{}));
    }
    else
        spdlog::error("Can't find device Focuser Simulator");
    m_App.GetDeviceManager()->addDevice(OpenAPT::DeviceType::FilterWheel, "Filter Simulator");
    auto filterList = m_App.GetDeviceManager()->getDeviceList(OpenAPT::DeviceType::FilterWheel);
    for (auto &name : filterList)
        spdlog::debug("Found Filterwheel name {}", name);
    spdlog::debug("Testing findDeviceByName:");
    auto device3 = m_App.GetDeviceManager()->findDeviceByName("Filter Simulator");
    if (device3 != nullptr) {
        device3->connect("Filter Simulator");
    }
    else
        spdlog::error("Can't find device Filter Simulator");
    m_App.GetDeviceManager()->addDevice(OpenAPT::DeviceType::Telescope, "Telescope Simulator");
    auto telescopeList = m_App.GetDeviceManager()->getDeviceList(OpenAPT::DeviceType::Telescope);
    for (auto &name : telescopeList)
        spdlog::debug("Found Telescope name {}", name);
    spdlog::debug("Testing findDeviceByName:");
    auto device4 = m_App.GetDeviceManager()->findDeviceByName("Telescope Simulator");
    if (device4 != nullptr) {
        device4->connect("Telescope Simulator");
    }
    else
        spdlog::error("Can't find device Filter Simulator");
    spdlog::debug("Finished testing DeviceManager");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("ConfigManager Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing setValue and getValue:");
    m_App.GetConfigManager()->setValue("key1", "value1");
    m_App.GetConfigManager()->setValue("key2/inner_key", 3.1415926);
    spdlog::debug("Get value of key2/inner_key: {}", m_App.GetConfigManager()->getValue("key2/inner_key").dump());
    spdlog::debug("Testing printAllValues:");
    m_App.GetConfigManager()->printAllValues();
    spdlog::debug("Finished testing ConfigManager");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("AchievementManager Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing add and complete achievement:");
    spdlog::debug("Printing all achievements:");
    spdlog::debug("Finished testing AchievementManager");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Compiler Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing CompileToSharedLibrary and LoadAndRunFunction:");
    Compiler compiler;
    std::string code = R"""(
    #include <iostream>
    extern "C" void foo()
    {
    std::cout << "Hello from foo()" << std::endl;
    }
    )""";
    std::string moduleName = "MyModule";
    std::string functionName = "foo";
    bool success = compiler.CompileToSharedLibrary(code, moduleName, functionName);
    if (success)
    {
        spdlog::debug("Compilation succeeded");
        m_App.GetModuleLoader()->LoadAndRunFunction<void>("MyModule", "foo", "foo", false);
    }
    else
    {
        spdlog::error("Compilation failed");
    }
    spdlog::debug("Finished testing Compiler");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Python Module Loader Testing");
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Testing load_local_module:");
    m_App.GetPythonLoader()->load_local_module("mymodule");
    spdlog::debug("Testing get_all_functions:");
    m_App.GetPythonLoader()->get_all_functions("mymodule");
    spdlog::debug("Testing set_variable:");
    m_App.GetPythonLoader()->set_variable("mymodule", "my_var", 42);
    spdlog::debug("Finished testing Python Module Loader");
    spdlog::debug("--------------------------------------------------------------");
    m_App.GetPythonLoader()->unload_module("mymodule");
    // nlohmann::json solve_result = OpenAPT::API::Astrometry::solve("apod3.jpg");
    // spdlog::debug("RA {} DEC {}",solve_result["ra"],solve_result["dec"]);
    m_App.GetTaskManager()->executeAllTasks();
    spdlog::debug("--------------------------------------------------------------");
    spdlog::debug("Shell Manager Testing");
    spdlog::debug("--------------------------------------------------------------");
    
    // 获取脚本列表

    // 运行脚本
    std::string patha = "./scripts";
    OpenAPT::ScriptManager scriptManager(patha);
    success = scriptManager.runScript("script1");
    if (success) {
        spdlog::info("Script executed successfully");
    } else {
        spdlog::error("Failed to execute script");
    }
}

void quit()
{
    ::exit(1);
}

// 初始化应用程序
void init_app(int argc, char *argv[], crow::SimpleApp &app)
{
    m_App.Initialize();
    parse_args(argc, argv);

    // 设置日志级别
    if (DEBUG)
    {
        spdlog::set_level(spdlog::level::debug);
        app.loglevel(crow::LogLevel::DEBUG);
        TestAll();
    }
    else
    {
        spdlog::set_level(spdlog::level::info);
        app.loglevel(crow::LogLevel::ERROR);
    }

    platform_check();

    if (!CheckAndKillProgramOnPort(8000))
        quit();

    OpenAPT::init_handler(app);
}

using namespace std;
using json = nlohmann::json;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

MyApp::MyApp() :
    m_ThreadManager(nullptr),
    m_TaskManager(nullptr),
    m_DeviceManager(nullptr),
    m_ModuleLoader(nullptr),
    m_ConfigManager(nullptr),
    m_PackageManager(nullptr),
    m_PythonLoader(nullptr),
    m_UseSSL(false),
    m_CertPath("")
{}

MyApp::~MyApp() {
    delete m_ThreadManager;
    delete m_TaskManager;
    delete m_DeviceManager;
    delete m_ModuleLoader;
    delete m_ConfigManager;
    delete m_PackageManager;
    delete m_PythonLoader;
}

void MyApp::Initialize(bool useSSL, const std::string& certPath) {
    // 使用智能指针代替 new 运算符
    m_ThreadManager = new OpenAPT::ThreadManager();
    m_TaskManager = new OpenAPT::TaskManager();
    m_DeviceManager = new OpenAPT::DeviceManager();
    m_ModuleLoader = new OpenAPT::ModuleLoader(&m_App);
    m_ConfigManager = new OpenAPT::ConfigManager();
    m_PackageManager = new OpenAPT::PackageManager();
    m_PythonLoader = new OpenAPT::PyModuleLoader();

    // 保存 SSL 相关设置
    m_UseSSL = useSSL;
    m_CertPath = certPath;

    // 创建 Server 实例和连接映射表
    m_Server = std::make_shared<websocketpp::server<websocketpp::config::asio_tls>>();
    auto connections = std::make_shared<std::map<websocketpp::connection_hdl, std::string>>();

    // 初始化 Server 的设置
    m_Server->clear_access_channels(websocketpp::log::alevel::all);
    m_Server->clear_error_channels(websocketpp::log::elevel::all);

    m_Server->set_message_handler(std::bind(&MyApp::on_message, this, std::placeholders::_1, std::placeholders::_2));
    m_Server->set_open_handler(std::bind(&MyApp::on_open, this, std::placeholders::_1));
    m_Server->set_close_handler(std::bind(&MyApp::on_close, this, std::placeholders::_1));

    m_Server->set_validate_handler([](websocketpp::connection_hdl) { return true; });
    m_Server->set_fail_handler([](websocketpp::connection_hdl) {});
    m_Server->set_http_handler([](websocketpp::connection_hdl) {});

    m_Server->set_reuse_addr(true);

    // 根据 SSL 设置监听 HTTP 或 HTTPS 端口
    if (m_UseSSL) {
        try {
            // 创建 SSL 上下文对象
            
        } catch (const std::exception& ex) {
            spdlog::error("Failed to initialize SSL: {}", ex.what());
            // 释放已创建的对象以避免内存泄漏
            m_Server->reset();
            return;
        }

        spdlog::info("WebSocket server started with SSL on port {}", 9002);
    } else {
        m_Server->init_asio();
        spdlog::info("WebSocket server started on port {}", 9001);
    }

    try {
        if (m_UseSSL) {
            m_Server->listen(9002);
        } else {
            m_Server->listen(9001);
        }

        m_Server->start_accept();
        m_Server->run();
    } catch (const std::exception& ex) {
        spdlog::error("Exception: {}", ex.what());
        // 释放已创建的对象以避免内存泄漏
        m_Server.reset();
        return;
    }

    spdlog::info("WebSocket server stopped");
}


void MyApp::sendJSONMessage(const json& msg) {
    if (!m_Connections.empty()) {
        // Convert the JSON message to a string
        std::string payload = msg.dump();

        // Find a valid connection and send the message
        for (auto it = m_Connections.begin(); it != m_Connections.end(); ++it) {
            try {
                m_Server->send(it->first, payload, websocketpp::frame::opcode::text);
                break;
            } catch (const std::exception& ex) {
                spdlog::error("Failed to send message to client {}: {}", it->second, ex.what());
            }
        }
    } else {
        spdlog::warn("No WebSocket clients connected to this server!");
    }
}

void MyApp::on_message(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
    // Handle incoming messages from clients
}

void MyApp::on_open(websocketpp::connection_hdl hdl) {
    // Handle new client connections
}

void MyApp::on_close(websocketpp::connection_hdl hdl) {
    // Handle client disconnections
}

// 启动 Web 服务器
void start_server(int port, crow::SimpleApp &app)
{
    app.port(port).multithreaded().run();
}

// 主函数
int main(int argc, char *argv[])
{
    try
    {
        registerInterruptHandler();

        init_app(argc, argv, m_App.GetApp());

        start_server(8000, m_App.GetApp());
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        // 保存崩溃日志到文件中
        OpenAPT::CrashReport::saveCrashLog(e.what());
        std::exit(EXIT_FAILURE);
    }

    return 0;
}