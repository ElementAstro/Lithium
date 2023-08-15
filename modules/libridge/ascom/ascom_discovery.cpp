#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <regex>

#include <loguru/loguru.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

std::vector<std::string> search_ipv4(int numquery = 2, int timeout = 2)
{
    std::vector<std::string> addrs;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_F(ERROR, "Failed to initialize Winsock");
        return addrs;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Failed to create socket: %d", WSAGetLastError());
        WSACleanup();
        return addrs;
    }

    BOOL broadcast = TRUE;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
    {
        LOG_F(ERROR, "Failed to set socket option: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return addrs;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
    {
        LOG_F(ERROR, "Failed to bind socket: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return addrs;
    }

    ULONG buf_size = sizeof(IP_ADAPTER_ADDRESSES);
    std::vector<IP_ADAPTER_ADDRESSES> adapter_addresses(buf_size);
    DWORD result = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, &adapter_addresses[0], &buf_size);
    if (result != NO_ERROR)
    {
        LOG_F(ERROR, "Failed to get interface addresses: %d", result);
        closesocket(sock);
        WSACleanup();
        return addrs;
    }

    for (const auto &adapter_addr : adapter_addresses)
    {
        sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(adapter_addr.FirstUnicastAddress->Address.lpSockaddr);
        if (addr->sin_family == AF_INET)
        {
            std::string iface_ip = inet_ntoa(addr->sin_addr);
            if (iface_ip == "127.0.0.1")
            {
                sockaddr_in broadcast_addr{};
                broadcast_addr.sin_family = AF_INET;
                broadcast_addr.sin_port = htons(port);
                inet_pton(AF_INET, "127.255.255.255", &broadcast_addr.sin_addr);

                sendto(sock, AlpacaDiscovery.c_str(), AlpacaDiscovery.length(), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
            }
            else
            {
                IP_ADAPTER_UNICAST_ADDRESS *broadcast_address = adapter_addr.FirstUnicastAddress;
                while (broadcast_address)
                {
                    if (broadcast_address->Flags & IP_ADAPTER_ADDRESS_TRANSIENT)
                    {
                        break;
                    }

                    sockaddr_in *broadcast_addr = reinterpret_cast<sockaddr_in *>(broadcast_address->Address.lpSockaddr);
                    if (broadcast_addr->sin_family == AF_INET && broadcast_addr->sin_addr.S_un.S_addr != INADDR_ANY)
                    {
                        sendto(sock, AlpacaDiscovery.c_str(), AlpacaDiscovery.length(), 0, (struct sockaddr *)broadcast_addr, sizeof(*broadcast_addr));
                    }

                    broadcast_address = broadcast_address->Next;
                }
            }

            while (true)
            {
                char buffer[1024];
                sockaddr_in remote_addr{};
                int addr_len = sizeof(remote_addr);
                int recv_len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_addr, &addr_len);
                if (recv_len < 0)
                {
                    break;
                }

                std::string response(buffer, recv_len);
                try
                {
                    auto json = nlohmann::json::parse(response);
                    std::string remport = json["AlpacaPort"].get<std::string>();
                    std::string remip = inet_ntoa(remote_addr.sin_addr);
                    std::string ipp = remip + ":" + remport;
                    if (remip != iface_ip && remip != "127.0.0.1")
                    {
                        if (std::find(addrs.begin(), addrs.end(), ipp) == addrs.end())
                        {
                            addrs.push_back(ipp);
                        }
                    }
                }
                catch (const nlohmann::json::exception &e)
                {
                    continue;
                }
            }
        }
    }

    closesocket(sock);
    WSACleanup();
#endif

    return addrs;
}

std::string getCurrentTimeString()
{
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&currentTime);
    std::stringstream timeStream;
    timeStream << std::put_time(localTime, "[%Y-%m-%d %H:%M:%S]");
    return timeStream.str();
}

// IPV6地址搜索函数
std::vector<std::string> search_ipv6(int numquery = 2, int timeout = 2)
{
    std::vector<std::string> addrs;

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        std::cout << "Failed to get hostname" << std::endl;
        return addrs;
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    std::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;     // IPV6
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // 获取本地IPv6地址列表
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0)
    {
        std::cout << "Failed to get local address info" << std::endl;
        return addrs;
    }

    // 枚举所有查询次数和网络接口
    for (int query = 0; query < numquery; query++)
    {
        for (rp = result; rp != nullptr; rp = rp->ai_next)
        {
            char interfaceName[IF_NAMESIZE];
            if_indextoname(rp->ai_ifindex, interfaceName);

            int sock;
            struct sockaddr_in6 destAddr;
            std::memset(&destAddr, 0, sizeof(struct sockaddr_in6));

#ifdef _WIN32
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            {
                std::cout << "Failed to initialize winsock" << std::endl;
                continue;
            }

            sock = socket(AF_INET6, SOCK_DGRAM, 0);
#else
            sock = socket(rp->ai_family, SOCK_DGRAM, 0);
#endif

            if (sock == -1)
            {
                std::cout << "Failed to create socket" << std::endl;
                continue;
            }

#ifdef __linux__
            if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, interfaceName, strlen(interfaceName)) != 0)
            {
                std::cout << "Failed to bind socket to device" << std::endl;
                close(sock);
                continue;
            }
#elif defined(__APPLE__)
            if (bind(sock, rp->ai_addr, rp->ai_addrlen) != 0)
            {
                std::cout << "Failed to bind socket to address" << std::endl;
                close(sock);
                continue;
            }
#endif

            destAddr.sin6_family = AF_INET6;
            if (inet_pton(AF_INET6, "ff12::a1:9aca", &(destAddr.sin6_addr)) != 1)
            {
                std::cout << "Failed to convert destination address" << std::endl;
                close(sock);
                continue;
            }

            // 发送发现请求
            std::string discoveryMessage = generateDiscoveryMessage();
            int bytesSent = sendto(sock, discoveryMessage.c_str(), discoveryMessage.length(), 0, (struct sockaddr *)&destAddr, sizeof(struct sockaddr_in6));
            if (bytesSent == -1)
            {
                std::cout << "Failed to send discovery message" << std::endl;
                close(sock);
                continue;
            }

            char buffer[MAX_BUFFER_SIZE];
            struct sockaddr_in6 senderAddr;

            while (true)
            {
                std::memset(buffer, 0, MAX_BUFFER_SIZE);
                socklen_t senderAddrLen = sizeof(senderAddr);
                int bytesRead = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&senderAddr, &senderAddrLen);

                if (bytesRead == -1)
                {
                    break;
                }

                std::string receivedMessage(buffer, bytesRead);
                char senderAddress[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &(senderAddr.sin6_addr), senderAddress, INET6_ADDRSTRLEN);
                int senderPort = ntohs(senderAddr.sin6_port);

                if (isDuplicateAddress(senderAddress, senderPort, addrs))
                {
                    continue;
                }

                std::string formattedAddress = formatAddress(senderAddress, senderPort);
                addrs.push_back(formattedAddress);
            }

#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
        }
    }

    freeaddrinfo(result);
    return addrs;
}

int main()
{
    loguru::init(argc, argv);

    std::vector<std::string> discovered_addrs = search_ipv4();

    for (const auto &addr : discovered_addrs)
    {
        LOG_F(INFO, "%s", addr.c_str());
    }

    std::vector<std::string> ipv6Addresses = search_ipv6();

    // 打印搜索结果
    for (const std::string &address : ipv6Addresses)
    {
        std::cout << "Discovered Alpaca device server: " << address << std::endl;
    }

    loguru::shutdown();

    return 0;
}
