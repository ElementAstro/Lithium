#include "discovery.hpp"

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
// clang-format on
#ifdef _NSC_VER
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

#include "atom/type/json.hpp"

using json = nlohmann::json;

class AlpacaDiscovery::Impl {
public:
    static constexpr int PORT = 32227;
    static constexpr const char* ALPACA_DISCOVERY = "alpacadiscovery1";
    static constexpr const char* ALPACA_RESPONSE = "AlpacaPort";

    auto searchIPv4(int numQuery, int timeout) -> std::vector<std::string>;

private:
    auto getInterfaces() -> std::vector<std::string>;
    void sendBroadcast(int sock, const std::string& interfaceA);
    void receiveResponses(int sock, std::vector<std::string>& addresses);
};

AlpacaDiscovery::AlpacaDiscovery() : pImpl(std::make_unique<Impl>()) {}

AlpacaDiscovery::~AlpacaDiscovery() = default;

auto AlpacaDiscovery::searchIPv4(int numQuery,
                                 int timeout) -> std::vector<std::string> {
    return pImpl->searchIPv4(numQuery, timeout);
}

auto AlpacaDiscovery::Impl::searchIPv4(int numQuery, int timeout)
    -> std::vector<std::string> {
    std::vector<std::string> addresses;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
    }
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<char*>(&broadcastEnable),
                   sizeof(broadcastEnable)) < 0) {
        throw std::runtime_error("Failed to set socket option");
    }

    struct timeval tv {};
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv),
                   sizeof(tv)) < 0) {
        throw std::runtime_error("Failed to set socket timeout");
    }

    std::vector<std::string> interfaceAs = getInterfaces();

    for (int i = 0; i < numQuery; ++i) {
        for (const auto& iface : interfaceAs) {
            sendBroadcast(sock, iface);
        }

        receiveResponses(sock, addresses);
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return addresses;
}

auto AlpacaDiscovery::Impl::getInterfaces() -> std::vector<std::string> {
    std::vector<std::string> interfaceAs;

#ifdef _WIN32
    ULONG bufferSize = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    DWORD retVal = 0;

    do {
        pAddresses = *static_cast<PIP_ADAPTER_ADDRESSES*>(malloc(bufferSize));
        if (pAddresses == nullptr) {
            throw std::runtime_error(
                "Memory allocation failed for IP_ADAPTER_ADDRESSES struct");
        }

        retVal = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, nullptr,
                                      pAddresses, &bufferSize);

        if (retVal == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = nullptr;
        }
    } while (retVal == ERROR_BUFFER_OVERFLOW);

    if (retVal == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
             pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
            PIP_ADAPTER_UNICAST_ADDRESS pUnicast =
                pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != nullptr) {
                for (; pUnicast != nullptr; pUnicast = pUnicast->Next) {
                    auto* pAddr = reinterpret_cast<sockaddr_in*>(
                        pUnicast->Address.lpSockaddr);
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(pAddr->sin_addr), ip, INET_ADDRSTRLEN);
                    interfaceAs.emplace_back(ip);
                }
            }
        }
    }

    if (pAddresses) {
        free(pAddresses);
    }
#else
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;

    if (getifaddrs(&ifAddrStruct) == -1) {
        throw std::runtime_error("Failed to get network interfaceAs");
    }

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            interfaceAs.emplace_back(addressBuffer);
        }
    }

    if (ifAddrStruct != nullptr)
        freeifaddrs(ifAddrStruct);
#endif

    return interfaceAs;
}

void AlpacaDiscovery::Impl::sendBroadcast(int sock,
                                          const std::string& interfaceA) {
    struct sockaddr_in broadcastAddr {};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(PORT);

    if (interfaceA == "127.0.0.1") {
        broadcastAddr.sin_addr.s_addr = inet_addr("127.255.255.255");
    } else {
        broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    }

    if (sendto(sock, ALPACA_DISCOVERY,
               static_cast<int>(strlen(ALPACA_DISCOVERY)), 0,
               (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == -1) {
        std::cerr << "Failed to send broadcast on interfaceA " << interfaceA
                  << std::endl;
    }
}

void AlpacaDiscovery::Impl::receiveResponses(
    int sock, std::vector<std::string>& addresses) {
    char buffer[1024];
    struct sockaddr_in senderAddr {};
    socklen_t senderAddrLen = sizeof(senderAddr);

    while (true) {
        int bytesReceived =
            recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr*)&senderAddr, &senderAddrLen);
        if (bytesReceived == -1) {
            break;  // Timeout or error
        }

        buffer[bytesReceived] = '\0';

        try {
            json response = json::parse(buffer);
            int alpacaPort = response[ALPACA_RESPONSE];
            char senderIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP,
                      INET_ADDRSTRLEN);

            std::string addressPort =
                std::format("{}:{}", senderIP, alpacaPort);
            if (std::find(addresses.begin(), addresses.end(), addressPort) ==
                addresses.end()) {
                addresses.push_back(addressPort);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse response: " << e.what() << std::endl;
        }
    }
}