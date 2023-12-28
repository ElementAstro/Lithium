#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>
#include <mutex>
#include <getopt.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "atom/log/loguru.hpp"

#include "config.h"

const int BUFFER_SIZE = 4096;

std::mutex mutex;

// 处理数据传输
void forward_data(int src_sockfd, int dst_sockfd)
{
    char buffer[BUFFER_SIZE];
    int num_bytes;

    try
    {
        while ((num_bytes = recv(src_sockfd, buffer, BUFFER_SIZE, 0)) > 0)
        {
            send(dst_sockfd, buffer, num_bytes, 0);
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, _("Failed to forward data: {}"), e.what());
    }
}

// 启动代理服务器
void start_proxy_server(const std::string &src_ip, int src_port, const std::string &dst_ip, int dst_port)
{
#ifdef _WIN32
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        LOG_F(ERROR, _("Failed to initialize Winsock."));
        return;
    }

    SOCKET src_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET dst_sockfd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int src_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int dst_sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

    if (src_sockfd == -1 || dst_sockfd == -1)
    {
        LOG_F(ERROR, _("Failed to create socket."));
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    // 绑定源地址和端口
    sockaddr_in src_addr{};
    src_addr.sin_family = AF_INET;
    src_addr.sin_addr.s_addr = inet_addr(src_ip.c_str());
    src_addr.sin_port = htons(src_port);

    if (bind(src_sockfd, reinterpret_cast<sockaddr *>(&src_addr), sizeof(src_addr)) == -1)
    {
        LOG_F(ERROR, _("Failed to bind source address."));
#ifdef _WIN32
        closesocket(src_sockfd);
        WSACleanup();
#else
        close(src_sockfd);
#endif
        return;
    }

    // 连接目标地址和端口
    sockaddr_in dst_addr{};
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = inet_addr(dst_ip.c_str());
    dst_addr.sin_port = htons(dst_port);

    if (connect(dst_sockfd, reinterpret_cast<sockaddr *>(&dst_addr), sizeof(dst_addr)) == -1)
    {
        LOG_F(ERROR, _("Failed to connect to destination address."));
#ifdef _WIN32
        closesocket(src_sockfd);
        closesocket(dst_sockfd);
        WSACleanup();
#else
        close(src_sockfd);
        close(dst_sockfd);
#endif
        return;
    }

    // 开始转发数据
    forward_data(src_sockfd, dst_sockfd);

#ifdef _WIN32
    closesocket(src_sockfd);
    closesocket(dst_sockfd);
    WSACleanup();
#else
    close(src_sockfd);
    close(dst_sockfd);
#endif
}

int main()
{
#ifdef _WIN32
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0)
    {
        LOG_F(ERROR, _("Failed to initialize Winsock."));
        return -1;
    }
#endif

    std::string src_ip = "127.0.0.1"; // 源IP地址
    int src_port = 12345;             // 源端口

    std::string dst_ip = "127.0.0.1"; // 目标IP地址
    int dst_port = 54321;             // 目标端口

    int option;
    while ((option = getopt(argc, argv, "s:p:d:o:")) != -1)
    {
        switch (option)
        {
        case 's':
            src_ip = optarg;
            break;
        case 'p':
            src_port = std::stoi(optarg);
            break;
        case 'd':
            dst_ip = optarg;
            break;
        case 'o':
            dst_port = std::stoi(optarg);
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " -s <src_ip> -p <src_port> -d <dst_ip> -o <dst_port>" << std::endl;
            return 1;
        }
    }

    std::vector<std::thread> threads;

    // 启动多个线程处理并发连接
    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back(start_proxy_server, src_ip, src_port, dst_ip, dst_port);
    }

    // 等待所有线程结束
    for (auto &thread : threads)
    {
        thread.join();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
