#include "utils.hpp"

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
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdio>
#define WIN_FLAG false
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace atom::web {
#ifdef __linux__ || __APPLE__
auto dumpAddrInfo(struct addrinfo** dst, struct addrinfo* src) -> int {
    if (src == nullptr) {
        return -1;
    }

    int ret = 0;
    struct addrinfo* aiDst = nullptr;
    struct addrinfo* aiSrc = src;
    struct addrinfo* aiCur = nullptr;

    while (aiSrc != nullptr) {
        size_t aiSize =
            sizeof(struct addrinfo) + sizeof(struct sockaddr_storage);
        auto ai = std::unique_ptr<struct addrinfo>(
            reinterpret_cast<struct addrinfo*>(calloc(1, aiSize)));
        if (ai == nullptr) {
            ret = -1;
            break;
        }
        memcpy(ai.get(), aiSrc, aiSize);
        ai->ai_addr = reinterpret_cast<struct sockaddr*>(ai.get() + 1);
        ai->ai_next = nullptr;
        if (aiSrc->ai_canonname != nullptr) {
            ai->ai_canonname = strdup(aiSrc->ai_canonname);
        }

        if (aiDst == nullptr) {
            aiDst = ai.release();
        } else {
            aiCur->ai_next = ai.release();
        }
        aiCur = aiDst->ai_next;
        aiSrc = aiSrc->ai_next;
    }

    if (ret != 0) {
        freeaddrinfo(aiDst);
        return ret;
    }

    *dst = aiDst;
    return ret;
}

auto addrInfoToString(struct addrinfo* addrInfo,
                      bool jsonFormat) -> std::string {
    std::ostringstream oss;
    if (jsonFormat) {
        oss << "[\n";  // Start JSON array
    }

    while (addrInfo != nullptr) {
        if (jsonFormat) {
            oss << "  {\n";
            oss << "    \"ai_flags\": " << addrInfo->ai_flags << ",\n";
            oss << "    \"ai_family\": " << addrInfo->ai_family << ",\n";
            oss << "    \"ai_socktype\": " << addrInfo->ai_socktype << ",\n";
            oss << "    \"ai_protocol\": " << addrInfo->ai_protocol << ",\n";
            oss << "    \"ai_addrlen\": " << addrInfo->ai_addrlen << ",\n";
            oss << R"(    "ai_canonname": ")"
                << (addrInfo->ai_canonname ? addrInfo->ai_canonname : "null")
                << "\",\n";

            // Handling IPv4 and IPv6 addresses
            if (addrInfo->ai_family == AF_INET) {
                auto addr_in =
                    reinterpret_cast<struct sockaddr_in*>(addrInfo->ai_addr);
                std::array<char, INET_ADDRSTRLEN> ip_str;
                inet_ntop(AF_INET, &addr_in->sin_addr, ip_str.data(),
                          ip_str.size());
                oss << R"(    "address": ")" << ip_str.data() << "\",\n";
            } else if (addrInfo->ai_family == AF_INET6) {
                auto addr_in6 =
                    reinterpret_cast<struct sockaddr_in6*>(addrInfo->ai_addr);
                std::array<char, INET6_ADDRSTRLEN> ip_str;
                inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str.data(),
                          ip_str.size());
                oss << R"(    "address": ")" << ip_str.data() << "\",\n";
            }
            oss << "  },\n";  // Close JSON object
        } else {
            oss << "ai_flags: " << addrInfo->ai_flags << "\n";
            oss << "ai_family: " << addrInfo->ai_family << "\n";
            oss << "ai_socktype: " << addrInfo->ai_socktype << "\n";
            oss << "ai_protocol: " << addrInfo->ai_protocol << "\n";
            oss << "ai_addrlen: " << addrInfo->ai_addrlen << "\n";
            oss << "ai_canonname: "
                << (addrInfo->ai_canonname ? addrInfo->ai_canonname : "null")
                << "\n";

            // Handling IPv4 and IPv6 addresses
            if (addrInfo->ai_family == AF_INET) {
                auto addr_in =
                    reinterpret_cast<struct sockaddr_in*>(addrInfo->ai_addr);
                std::array<char, INET_ADDRSTRLEN> ip_str;
                inet_ntop(AF_INET, &addr_in->sin_addr, ip_str.data(),
                          ip_str.size());
                oss << "Address (IPv4): " << ip_str.data() << "\n";
            } else if (addrInfo->ai_family == AF_INET6) {
                auto addr_in6 =
                    reinterpret_cast<struct sockaddr_in6*>(addrInfo->ai_addr);
                std::array<char, INET6_ADDRSTRLEN> ip_str;
                inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str.data(),
                          ip_str.size());
                oss << "Address (IPv6): " << ip_str.data() << "\n";
            }
            oss << "-------------------------\n";  // Separator for clarity
        }

        addrInfo = addrInfo->ai_next;
    }

    if (jsonFormat) {
        oss << "]\n";  // Close JSON array
    }

    return oss.str();
}

auto getAddrInfo(const std::string& hostname,
                 const std::string& service) -> struct addrinfo* {
    struct addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    struct addrinfo* result = nullptr;
    int ret = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &result);
    if (ret != 0) {
        throw std::runtime_error("getaddrinfo: " +
                                 std::string(gai_strerror(ret)));
    }
    return result;
}

void freeAddrInfo(struct addrinfo* addrInfo) { freeaddrinfo(addrInfo); }

auto compareAddrInfo(const struct addrinfo* addrInfo1,
                     const struct addrinfo* addrInfo2) -> bool {
    if (addrInfo1->ai_family != addrInfo2->ai_family) {
        return false;
    }
    if (addrInfo1->ai_socktype != addrInfo2->ai_socktype) {
        return false;
    }
    if (addrInfo1->ai_protocol != addrInfo2->ai_protocol) {
        return false;
    }
    if (addrInfo1->ai_addrlen != addrInfo2->ai_addrlen) {
        return false;
    }
    if (memcmp(addrInfo1->ai_addr, addrInfo2->ai_addr, addrInfo1->ai_addrlen) !=
        0) {
        return false;
    }
    return true;
}

auto filterAddrInfo(struct addrinfo* addrInfo, int family) -> struct addrinfo* {
    struct addrinfo* filtered = nullptr;
    struct addrinfo** last = &filtered;

    while (addrInfo != nullptr) {
        if (addrInfo->ai_family == family) {
            *last = reinterpret_cast<struct addrinfo*>(
                malloc(sizeof(struct addrinfo)));
            memcpy(*last, addrInfo, sizeof(struct addrinfo));
            (*last)->ai_next = nullptr;
            last = &(*last)->ai_next;
        }
        addrInfo = addrInfo->ai_next;
    }

    return filtered;
}

auto sortAddrInfo(struct addrinfo* addrInfo) -> struct addrinfo* {
    std::vector<struct addrinfo*> vec;
    while (addrInfo != nullptr) {
        vec.push_back(addrInfo);
        addrInfo = addrInfo->ai_next;
    }

    std::sort(vec.begin(), vec.end(),
              [](const struct addrinfo* a, const struct addrinfo* b) {
                  return a->ai_family < b->ai_family;
              });

    struct addrinfo* sorted = nullptr;
    struct addrinfo** last = &sorted;
    for (auto& entry : vec) {
        *last = entry;
        last = &entry->ai_next;
    }
    *last = nullptr;

    return sorted;
}
#endif

auto initializeWindowsSocketAPI() -> bool {
#ifdef _WIN32
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        LOG_F(ERROR, "Failed to initialize Windows Socket API: {}", ret);
        return false;
    }
#endif
    return true;
}

auto createSocket() -> int {
    int sockfd = static_cast<int>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (sockfd < 0) {
        char buf[256];
        LOG_F(ERROR, "Failed to create socket: {}",
              strerror_r(errno, buf, sizeof(buf)));
#ifdef _WIN32
        WSACleanup();
#endif
    }
    return sockfd;
}

auto bindSocket(int sockfd, uint16_t port) -> bool {
    struct sockaddr_in addr {};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        if (errno == EADDRINUSE) {
            DLOG_F(WARNING, "The port({}) is already in use", port);
            return false;
        }
        char buf[256];
        LOG_F(ERROR, "Failed to bind socket: {}",
              strerror_r(errno, buf, sizeof(buf)));
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
        atom::system::executeCommand(cmd, false, [](const std::string& line) {
            return line.find("LISTENING") != std::string::npos;
        });
    pidStr.erase(pidStr.find_last_not_of('\n') + 1);
    return pidStr;
}

auto isPortInUse(int port) -> bool {
    if (!initializeWindowsSocketAPI()) {
        return true;  // Assume port is in use if initialization fails
    }

    int sockfd = createSocket();
    if (sockfd < 0) {
        return true;  // Assume port is in use if socket creation fails
    }

    bool inUse = !bindSocket(sockfd, port);
    close(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return inUse;
}

auto checkAndKillProgramOnPort(int port) -> bool {
    if (isPortInUse(port)) {
        std::string pidStr = getProcessIDOnPort(port);
        if (pidStr.empty()) {
            LOG_F(ERROR, "Failed to get the PID of the process on port({}): {}",
                  port, pidStr);
            return false;
        }
        try {
            atom::system::killProcessByPID(std::stoi(pidStr), 15);
        } catch (const atom::error::SystemCollapse& e) {
            LOG_F(ERROR, "Failed to kill the process on port({}): {}", port,
                  e.what());
            return false;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Unexpected error: {}", e.what());
            return false;
        }
    }
    return true;
}
}  // namespace atom::web
