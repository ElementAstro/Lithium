#include <cstring>
#include <format>
#include <string>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define WIN_FLAG true
#define close closesocket
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif
#elif __linux__ || __APPLE__
#include <cstdio>
#include <cstring>
#endif

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

auto initializeWindowsSocketAPI() -> bool {
#ifdef _WIN32
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        LOG_F(ERROR, "Failed to initialize Windows Socket API: %d", ret);
        return false;
    }
#endif
    return true;
}

auto createSocket() -> int {
    int sockfd = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (sockfd < 0) {
        LOG_F(ERROR, "Failed to create socket: {}", strerror(errno));
#ifdef _WIN32
        WSACleanup();
#endif
    }
    return sockfd;
}

auto bindSocket(int sockfd, int port) -> bool {
    struct sockaddr_in addr {};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        if (errno == EADDRINUSE) {
            DLOG_F(WARNING, "The port({}) is already in use", port);
            return false;
        }
        LOG_F(ERROR, "Failed to bind socket: {}", strerror(errno));
        return false;
    }
    return true;
}

auto getProcessIDOnPort(int port) -> std::string {
    std::string cmd;
#ifdef __cpp_lib_format
    cmd = std::format("{}{}",
                      (WIN_FLAG ? R"(netstat -ano | find "LISTENING" | find ")"
                                : "lsof -i :{} -t"),
                      port);
#else
    cmd = fmt::format("{}{}",
                      (WIN_FLAG ? "netstat -ano | find \"LISTENING\" | find \""
                                : "lsof -i :{} -t"),
                      port);
#endif

    std::string pidStr =
        atom::system::executeCommand(cmd, false, [](const std::string &line) {
            return line.find("LISTENING") != std::string::npos;
        });
    pidStr.erase(pidStr.find_last_not_of('\n') + 1);
    return pidStr;
}

auto killProcess(const std::string &pidStr) -> bool {
    std::string killCmd;
#ifdef __cpp_lib_format
    killCmd =
        std::format("{}{}", (WIN_FLAG ? "taskkill /F /PID " : "kill "), pidStr);
#else
    killCmd =
        fmt::format("{}{}", (WIN_FLAG ? "taskkill /F /PID " : "kill "), pidStr);
#endif

    if (!atom::system::executeCommand(killCmd, false,
                                      [pidStr](const std::string &line) {
                                          return line.find(pidStr) !=
                                                 std::string::npos;
                                      })
             .empty()) {
        LOG_F(ERROR, "Failed to kill the process: {}", pidStr);
        return false;
    }
    DLOG_F(INFO, "The process({}) is killed successfully", pidStr);
    return true;
}

auto checkAndKillProgramOnPort(int port) -> bool {
    if (!initializeWindowsSocketAPI()) {
        return false;
    }

    int sockfd = createSocket();
    if (sockfd < 0) {
        return false;
    }

    if (!bindSocket(sockfd, port)) {
        std::string pidStr = getProcessIDOnPort(port);
        if (pidStr.empty()) {
            LOG_F(ERROR, "Failed to get the PID of the process on port({}): {}",
                  port, pidStr);
            return false;
        }

        if (!killProcess(pidStr)) {
            return false;
        }
    }

    close(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return true;
}