#include "network_manager.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <unordered_map>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace atom::system {
class NetworkInterface::NetworkInterfaceImpl {
public:
    std::string name;
    std::vector<std::string> addresses;
    std::string mac;
    bool isUp;

    NetworkInterfaceImpl(std::string name, std::vector<std::string> addresses,
                         std::string mac, bool isUp)
        : name(std::move(name)),
          addresses(std::move(addresses)),
          mac(std::move(mac)),
          isUp(isUp) {}
};

NetworkInterface::NetworkInterface(std::string name,
                                   std::vector<std::string> addresses,
                                   std::string mac, bool isUp)
    : impl_(
          std::make_unique<NetworkInterfaceImpl>(name, addresses, mac, isUp)) {}

[[nodiscard]] auto NetworkInterface::getName() const -> const std::string& {
    return impl_->name;
}
[[nodiscard]] auto NetworkInterface::getAddresses() const
    -> const std::vector<std::string>& {
    return impl_->addresses;
}
auto NetworkInterface::getAddresses() -> std::vector<std::string>& {
    return impl_->addresses;
}
[[nodiscard]] auto NetworkInterface::getMac() const -> const std::string& {
    return impl_->mac;
}
[[nodiscard]] auto NetworkInterface::isUp() const -> bool {
    return impl_->isUp;
}

class NetworkManager::NetworkManagerImpl {
public:
    std::mutex mtx_;
    bool running_{true};
#ifdef _WIN32
    WSADATA wsaData_;
#endif
};
NetworkManager::NetworkManager() {
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
        THROW_RUNTIME_ERROR("WSAStartup failed");
    }
#endif
}

NetworkManager::~NetworkManager() {
    impl_->running_ = false;
#ifdef _WIN32
    WSACleanup();
#endif
}

auto NetworkManager::getNetworkInterfaces() -> std::vector<NetworkInterface> {
    std::lock_guard lock(impl_->mtx_);
    std::vector<NetworkInterface> interfaces;

#ifdef _WIN32
    ULONG outBufLen = 15000;
    std::vector<BYTE> buffer(outBufLen);
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    PIP_ADAPTER_ADDRESSES pAddresses =
        reinterpret_cast<PIP_ADAPTER_ADDRESSES*>(buffer.data());
    ULONG family = AF_UNSPEC;

    DWORD dwRetVal =
        GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(outBufLen);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES*>(buffer.data());
        dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses,
                                        &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        THROW_RUNTIME_ERROR("GetAdaptersAddresses failed with error: " +
                            std::to_string(dwRetVal));
    }

    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
         pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
        std::vector<std::string> ips;
        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast =
                 pCurrAddresses->FirstUnicastAddress;
             pUnicast != nullptr; pUnicast = pUnicast->Next) {
            char ipStr[INET6_ADDRSTRLEN];
            int result = getnameinfo(pUnicast->Address.lpSockaddr,
                                     pUnicast->Address.iSockaddrLength, ipStr,
                                     sizeof(ipStr), nullptr, 0, NI_NUMERICHOST);
            if (result != 0) {
                continue;
            }
            ips.emplace_back(ipStr);
        }

        bool isUp = (pCurrAddresses->OperStatus == IfOperStatusUp);
        interfaces.emplace_back(
            pCurrAddresses->AdapterName, ips,
            getMacAddress(pCurrAddresses->AdapterName).value_or("N/A"), isUp);
    }
#else
    struct ifaddrs* ifAddrStruct = nullptr;
    if (getifaddrs(&ifAddrStruct) == -1) {
        THROW_RUNTIME_ERROR("getifaddrs failed");
    }

    std::unordered_map<std::string, NetworkInterface> ifaceMap;

    for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr;
         ifa = ifa->ifa_next) {
        if ((ifa->ifa_addr != nullptr) && ifa->ifa_addr->sa_family == AF_INET) {
            std::string name = ifa->ifa_name;
            char address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
                      address, sizeof(address));

            if (ifaceMap.find(name) == ifaceMap.end()) {
                bool isUp = (ifa->ifa_flags & IFF_UP) != 0;
                ifaceMap.emplace(
                    name, NetworkInterface(
                              name, std::vector<std::string>{address},
                              getMacAddress(name).value_or("N/A"), isUp));
            } else {
                ifaceMap[name].getAddresses().emplace_back(address);
            }
        }
    }

    freeifaddrs(ifAddrStruct);

    interfaces.reserve(ifaceMap.size());
    for (const auto& pair : ifaceMap) {
        interfaces.push_back(pair.second);
    }
#endif

    return interfaces;
}

auto NetworkManager::getMacAddress(const std::string& interfaceName)
    -> std::optional<std::string> {
#ifdef _WIN32
    ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
    PIP_ADAPTER_ADDRESSES pAddresses =
        reinterpret_cast<PIP_ADAPTER_ADDRESSES>(malloc(outBufLen));
    if (!pAddresses) {
        THROW_RUNTIME_ERROR(
            "Memory allocation failed for MAC address retrieval");
    }

    DWORD dwRetVal =
        GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(malloc(outBufLen));
        if (!pAddresses) {
            THROW_RUNTIME_ERROR(
                "Memory allocation failed for MAC address retrieval");
        }
        dwRetVal =
            GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, pAddresses, &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        free(pAddresses);
        THROW_RUNTIME_ERROR("GetAdaptersAddresses failed with error: " +
                            std::to_string(dwRetVal));
    }

    std::optional<std::string> mac = std::nullopt;
    for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != nullptr;
         pCurr = pCurr->Next) {
        if (interfaceName == pCurr->AdapterName) {
            if (pCurr->PhysicalAddressLength == 0) {
                break;
            }
            std::array<char, 18> macAddress;
            snprintf(macAddress.data(), macAddress.size(),
                     "%02X-%02X-%02X-%02X-%02X-%02X", pCurr->PhysicalAddress[0],
                     pCurr->PhysicalAddress[1], pCurr->PhysicalAddress[2],
                     pCurr->PhysicalAddress[3], pCurr->PhysicalAddress[4],
                     pCurr->PhysicalAddress[5]);
            mac = std::string(macAddress.data());
            break;
        }
    }

    free(pAddresses);
    return mac;
#else
    int socketFd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd < 0) {
        THROW_RUNTIME_ERROR(
            "Failed to create socket for MAC address retrieval");
    }

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);

    if (::ioctl(socketFd, SIOCGIFHWADDR, &ifr) < 0) {
        ::close(socketFd);
        THROW_RUNTIME_ERROR("ioctl SIOCGIFHWADDR failed for interface: " +
                            interfaceName);
    }
    ::close(socketFd);

    const auto* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
    std::string macAddress =
        std::format("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", mac[0], mac[1],
                    mac[2], mac[3], mac[4], mac[5]);
    return macAddress;
#endif
}

auto NetworkManager::isInterfaceUp(const std::string& interfaceName) -> bool {
    auto interfaces = getNetworkInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.getName() == interfaceName) {
            return iface.isUp();
        }
    }
    return false;
}

void NetworkManager::enableInterface(const std::string& interfaceName) {
#ifdef _WIN32
    MIB_IFROW ifRow;
    memset(&ifRow, 0, sizeof(MIB_IFROW));
    strncpy_s(reinterpret_cast<char*>(ifRow.wszName), interfaceName.c_str(),
              interfaceName.size());

    if (GetIfEntry(&ifRow) == NO_ERROR) {
        ifRow.dwAdminStatus = MIB_IF_ADMIN_STATUS_UP;
        if (SetIfEntry(&ifRow) != NO_ERROR) {
            THROW_RUNTIME_ERROR("Failed to enable interface: " + interfaceName);
        }
    } else {
        THROW_RUNTIME_ERROR("Failed to get interface entry: " + interfaceName);
    }
#else
    // Enable interface on Linux (requires sudo)
    std::string command = "sudo ip link set " + interfaceName + " up";
    int ret = executeCommandWithStatus(command).second;
    if (ret != 0) {
        THROW_RUNTIME_ERROR("Failed to enable interface: " + interfaceName);
    }
#endif
}

void NetworkManager::disableInterface(const std::string& interfaceName) {
#ifdef _WIN32
    MIB_IFROW ifRow;
    memset(&ifRow, 0, sizeof(MIB_IFROW));
    strncpy_s(reinterpret_cast<char*>(ifRow.wszName), interfaceName.c_str(),
              interfaceName.size());

    if (GetIfEntry(&ifRow) == NO_ERROR) {
        ifRow.dwAdminStatus = MIB_IF_ADMIN_STATUS_DOWN;
        if (SetIfEntry(&ifRow) != NO_ERROR) {
            THROW_RUNTIME_ERROR("Failed to disable interface: " +
                                interfaceName);
        }
    } else {
        THROW_RUNTIME_ERROR("Failed to get interface entry: " + interfaceName);
    }
#else
    // Disable interface on Linux (requires sudo)
    std::string command = "sudo ip link set " + interfaceName + " down";
    int ret = std::system(command.c_str());
    if (ret != 0) {
        THROW_RUNTIME_ERROR("Failed to disable interface: " + interfaceName);
    }
#endif
}

auto NetworkManager::resolveDNS(const std::string& hostname) -> std::string {
    struct addrinfo hints {};
    struct addrinfo* res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (ret != 0) {
        THROW_RUNTIME_ERROR("DNS resolution failed for " + hostname + ": " +
                            gai_strerror(ret));
    }

    std::array<char, INET_ADDRSTRLEN> ipStr;
    inet_ntop(AF_INET, &((struct sockaddr_in*)res->ai_addr)->sin_addr,
              ipStr.data(), ipStr.size());
    freeaddrinfo(res);
    return std::string(ipStr.data());
}

auto NetworkManager::getDNSServers() -> std::vector<std::string> {
    std::vector<std::string> dnsServers;
#ifdef _WIN32
    DWORD bufLen = 0;
    GetNetworkParams(nullptr, &bufLen);
    std::unique_ptr<BYTE[]> buffer(new BYTE[bufLen]);
    FIXED_INFO* pFixedInfo = reinterpret_cast<FIXED_INFO*>(buffer.get());

    if (GetNetworkParams(pFixedInfo, &bufLen) != NO_ERROR) {
        THROW_RUNTIME_ERROR("GetNetworkParams failed");
    }

    IP_ADDR_STRING* pAddr = &pFixedInfo->DnsServerList;
    while (pAddr) {
        dnsServers.emplace_back(pAddr->IpAddress.String);
        pAddr = pAddr->Next;
    }
#else
    std::ifstream resolvFile("/etc/resolv.conf");
    if (!resolvFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open /etc/resolv.conf");
    }

    std::string line;
    while (std::getline(resolvFile, line)) {
        if (line.compare(0, 10, "nameserver") == 0) {
            std::istringstream iss(line);
            std::string keyword;
            std::string ip;
            if (iss >> keyword >> ip) {
                dnsServers.emplace_back(ip);
            }
        }
    }
#endif
    return dnsServers;
}

void NetworkManager::setDNSServers(const std::vector<std::string>& dnsServers) {
#ifdef _WIN32
    // Windows-specific DNS server setting
    // This implementation sets DNS servers for all adapters
    // For more granular control, iterate through adapters and set individually

    ULONG outBufLen = 15000;
    std::vector<BYTE> buffer(outBufLen);
    PIP_ADAPTER_ADDRESSES pAddresses =
        reinterpret_cast<PIP_ADAPTER_ADDRESSES*>(buffer.data());
    ULONG family = AF_UNSPEC;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    DWORD dwRetVal =
        GetAdaptersAddresses(family, flags, nullptr, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(outBufLen);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES*>(buffer.data());
        dwRetVal = GetAdaptersAddresses(family, flags, nullptr, pAddresses,
                                        &outBufLen);
    }

    if (dwRetVal != NO_ERROR) {
        THROW_RUNTIME_ERROR("GetAdaptersAddresses failed with error: " +
                            std::to_string(dwRetVal));
    }

    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
         pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
        std::vector<IP_ADDRESS_STRING> dnsList;
        for (const auto& dns : dnsServers) {
            IP_ADDRESS_STRING dnsAddr;
            memset(&dnsAddr, 0, sizeof(IP_ADDRESS_STRING));
            strncpy_s(dnsAddr.String, dns.c_str(), sizeof(dnsAddr.String) - 1);
            dnsList.emplace_back(dnsAddr);
        }

        // Allocate and set DNS servers
        // Note: This is a simplified implementation. Proper implementation
        // requires more detailed handling.
        OVERLAPPED overlapped = {0};
        if (!SetAdapterDnsServerAddresses(
                pCurrAddresses->AdapterName, IPv4,
                dnsServers.empty()
                    ? nullptr
                    : reinterpret_cast<PIP_ADDR_STRING>(dnsList.data()),
                dnsServers.empty() ? 0 : dnsList.size())) {
            THROW_RUNTIME_ERROR("Failed to set DNS servers for adapter: " +
                                std::string(pCurrAddresses->AdapterName));
        }
    }
#else
    // Check if NetworkManager is running
    if (executeCommandSimple("pgrep NetworkManager > /dev/null")) {
        // Use NetworkManager to set DNS servers
        for (const auto& dns : dnsServers) {
            std::string command = "nmcli device modify eth0 ipv4.dns " + dns;
            int ret = executeCommandWithStatus(command).second;
            if (ret != 0) {
                THROW_RUNTIME_ERROR("Failed to set DNS server: " + dns);
            }
        }
        if (executeCommandSimple("nmcli connection reload")) {
            THROW_RUNTIME_ERROR("Failed to reload NetworkManager connection");
        }
    } else {
        // Fallback to modifying /etc/resolv.conf directly
        std::ofstream resolvFile("/etc/resolv.conf", std::ios::trunc);
        if (!resolvFile.is_open()) {
            THROW_RUNTIME_ERROR("Failed to open /etc/resolv.conf for writing");
        }

        for (const auto& dns : dnsServers) {
            resolvFile << "nameserver " << dns << "\n";
        }

        resolvFile.close();
    }
#endif
}

void NetworkManager::addDNSServer(const std::string& dns) {
    auto dnsServers = getDNSServers();
    // Check if DNS already exists
    if (std::find(dnsServers.begin(), dnsServers.end(), dns) !=
        dnsServers.end()) {
        LOG_F(INFO, "DNS server {} already exists.", dns);
        return;
    }
    dnsServers.emplace_back(dns);
    setDNSServers(dnsServers);
}

void NetworkManager::removeDNSServer(const std::string& dns) {
    auto dnsServers = getDNSServers();
    auto it = std::remove(dnsServers.begin(), dnsServers.end(), dns);
    if (it == dnsServers.end()) {
        LOG_F(INFO, "DNS server {} not found.", dns);
        return;
    }
    dnsServers.erase(it, dnsServers.end());
    setDNSServers(dnsServers);
}

void NetworkManager::monitorConnectionStatus() {
    std::thread([this]() {
        while (impl_->running_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::lock_guard lock(impl_->mtx_);
            try {
                auto interfaces = getNetworkInterfaces();
                LOG_F(INFO, "----- Network Interfaces Status -----");
                for (const auto& iface : interfaces) {
                    LOG_F(INFO,
                          "Interface: {} | Status: {} | IPs: {} | MAC: {}",
                          iface.getName(), iface.isUp() ? "Up" : "Down",
                          iface.getAddresses(), iface.getMac());
                    for (const auto& ip : iface.getAddresses()) {
                        LOG_F(INFO, "IP: {}", ip);
                    }
                    LOG_F(INFO, "MAC: {}", iface.getMac());
                }
                LOG_F(INFO, "--------------------------------------");
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Error in monitorConnectionStatus: {}", e.what());
            }
        }
    }).detach();
}

auto NetworkManager::getInterfaceStatus(const std::string& interfaceName)
    -> std::string {
    auto interfaces = getNetworkInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.getName() == interfaceName) {
            return iface.isUp() ? "Up" : "Down";
        }
    }
    THROW_RUNTIME_ERROR("Interface not found: " + interfaceName);
}

auto parseAddressPort(const std::string& addressPort)
    -> std::pair<std::string, int> {
    size_t colonPos = addressPort.find_last_of(':');
    if (colonPos != std::string::npos) {
        std::string address = addressPort.substr(0, colonPos);
        int port = std::stoi(addressPort.substr(colonPos + 1));
        return {address, port};
    }
    return {"", 0};
}

auto getNetworkConnections(int pid) -> std::vector<NetworkConnection> {
    std::vector<NetworkConnection> connections;

#ifdef _WIN32
    // Windows: Use GetExtendedTcpTable to get TCP connections.
    MIB_TCPTABLE_OWNER_PID* pTCPInfo = nullptr;
    DWORD dwSize = 0;
    GetExtendedTcpTable(nullptr, &dwSize, false, AF_INET,
                        TCP_TABLE_OWNER_PID_ALL, 0);
    pTCPInfo = (MIB_TCPTABLE_OWNER_PID*)malloc(dwSize);
    if (GetExtendedTcpTable(pTCPInfo, &dwSize, false, AF_INET,
                            TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTCPInfo->dwNumEntries; ++i) {
            if (pTCPInfo->table[i].dwOwningPid == pid) {
                NetworkConnection conn;
                conn.protocol = "TCP";
                conn.localAddress =
                    inet_ntoa(*(in_addr*)&pTCPInfo->table[i].dwLocalAddr);
                conn.localPort = ntohs((u_short)pTCPInfo->table[i].dwLocalPort);
                conn.remoteAddress =
                    inet_ntoa(*(in_addr*)&pTCPInfo->table[i].dwRemoteAddr);
                conn.remotePort =
                    ntohs((u_short)pTCPInfo->table[i].dwRemotePort);
                connections.push_back(conn);
                LOG_F(INFO, "Found TCP connection: Local {}:{} -> Remote {}:{}",
                      conn.localAddress, conn.localPort, conn.remoteAddress,
                      conn.remotePort);
            }
        }
    } else {
        LOG_F(ERROR, "Failed to get TCP table. Error: {}", GetLastError());
    }
    free(pTCPInfo);

#elif __APPLE__
    // macOS: Use `lsof` to get network connections.
    std::array<char, 128> buffer;
    std::string command = "lsof -i -n -P | grep " + std::to_string(pid);
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        LOG_F(ERROR, "Failed to run lsof command.");
        return connections;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::istringstream iss(buffer.data());
        std::string proto, local, remote, ignore;
        iss >> ignore >> ignore >> ignore >> proto >> local >> remote;

        auto [localAddr, localPort] = parseAddressPort(local);
        auto [remoteAddr, remotePort] = parseAddressPort(remote);

        connections.push_back(
            {proto, localAddr, remoteAddr, localPort, remotePort});
        LOG_F(INFO, "Found {} connection: Local {}:{} -> Remote {}:{}", proto,
              localAddr, localPort, remoteAddr, remotePort);
    }
    pclose(pipe);

#elif __linux__ || __ANDROID__
    // Linux/Android: Parse /proc/<pid>/net/tcp and /proc/<pid>/net/udp.
    for (const auto& [protocol, path] :
         {std::pair{"TCP", "net/tcp"}, {"UDP", "net/udp"}}) {
        std::ifstream netFile("/proc/" + std::to_string(pid) + "/" + path);
        if (!netFile.is_open()) {
            LOG_F(ERROR, "Failed to open: /proc/{}/{}", pid, path);
            continue;
        }

        std::string line;
        std::getline(netFile, line);  // Skip header line.

        while (std::getline(netFile, line)) {
            std::istringstream iss(line);
            std::string localAddress;
            std::string remoteAddress;
            std::string ignore;
            int state;
            int inode;

            // Parse the fields from the /proc file.
            iss >> ignore >> localAddress >> remoteAddress >> std::hex >>
                state >> ignore >> ignore >> ignore >> inode;

            auto [localAddr, localPort] = parseAddressPort(localAddress);
            auto [remoteAddr, remotePort] = parseAddressPort(remoteAddress);

            connections.push_back(
                {protocol, localAddr, remoteAddr, localPort, remotePort});
            LOG_F(INFO, "Found {} connection: Local {}:{} -> Remote {}:{}",
                  protocol, localAddr, localPort, remoteAddr, remotePort);
        }
    }
#endif

    return connections;
}
}  // namespace atom::system
