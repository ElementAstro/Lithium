/*
 * uuid.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#include "uuid.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>
#include <iomanip>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <objbase.h>
// clang-format on
#elif defined(__linux__) || defined(__APPLE__)
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <uuid/uuid.h>
#endif

#include <openssl/md5.h>
#include <openssl/sha.h>

namespace atom::utils {
UUID::UUID() { generate_random(); }

UUID::UUID(const std::array<uint8_t, 16>& data) : data(data) {}

std::string UUID::to_string() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < data.size(); ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) {
            oss << '-';
        }
    }
    return oss.str();
}

UUID UUID::from_string(const std::string& str) {
    UUID uuid;
    size_t pos = 0;
    for (size_t i = 0; i < uuid.data.size(); ++i) {
        if (str[pos] == '-') {
            ++pos;
        }
        uuid.data[i] = std::stoi(str.substr(pos, 2), nullptr, 16);
        pos += 2;
    }
    return uuid;
}

bool UUID::operator==(const UUID& other) const { return data == other.data; }

bool UUID::operator!=(const UUID& other) const { return !(*this == other); }

bool UUID::operator<(const UUID& other) const { return data < other.data; }

std::ostream& operator<<(std::ostream& os, const UUID& uuid) {
    return os << uuid.to_string();
}

std::istream& operator>>(std::istream& is, UUID& uuid) {
    std::string str;
    is >> str;
    uuid = UUID::from_string(str);
    return is;
}

std::array<uint8_t, 16> UUID::get_data() const { return data; }

uint8_t UUID::version() const { return (data[6] & 0xF0) >> 4; }

uint8_t UUID::variant() const { return (data[8] & 0xC0) >> 6; }

UUID UUID::generate_v3(const UUID& namespace_uuid, const std::string& name) {
    return generate_name_based<MD5_CTX, MD5_Init, MD5_Update, MD5_Final>(
        namespace_uuid, name, 3);
}

UUID UUID::generate_v5(const UUID& namespace_uuid, const std::string& name) {
    return generate_name_based<SHA_CTX, SHA1_Init, SHA1_Update, SHA1_Final>(
        namespace_uuid, name, 5);
}

UUID UUID::generate_v1() {
    UUID uuid;
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto timestamp = static_cast<uint64_t>(millis * 10000) + 0x01B21DD213814000;

    uint16_t clock_seq = static_cast<uint16_t>(rand() % 0x4000);
    uint64_t node = generate_node();

    std::memcpy(uuid.data.data(), &timestamp, 8);
    std::memcpy(uuid.data.data() + 8, &clock_seq, 2);
    std::memcpy(uuid.data.data() + 10, &node, 6);

    uuid.data[6] = (uuid.data[6] & 0x0F) | 0x10;  // Version 1
    uuid.data[8] = (uuid.data[8] & 0x3F) | 0x80;  // Variant

    return uuid;
}

void UUID::generate_random() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (auto& byte : data) {
        byte = dis(gen);
    }

    data[6] = (data[6] & 0x0F) | 0x40;  // Version 4
    data[8] = (data[8] & 0x3F) | 0x80;  // Variant
}

uint64_t UUID::generate_node() {
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFF);
    return dist(rd) | 0x010000000000;  // Multicast bit set to 1
}

std::string getMAC() {
    std::string mac;

#if defined(_WIN32)
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD bufferSize = sizeof(adapterInfo);

    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (UINT i = 0; i < pAdapterInfo->AddressLength; i++) {
            oss << std::setw(2) << static_cast<int>(pAdapterInfo->Address[i]);
        }
        mac = oss.str();
    }

#elif defined(__linux__) || defined(__APPLE__)
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        return mac;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        close(sock);
        return mac;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (!(ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
    }

    if (success) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < 6; i++) {
            oss << std::setw(2)
                << static_cast<int>(
                       static_cast<unsigned char>(ifr.ifr_hwaddr.sa_data[i]));
        }
        mac = oss.str();
    }

    close(sock);
#endif

    return mac;
}

std::string getCPUSerial() {
    std::string cpuSerial;

#if defined(_WIN32)
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 1);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; i++) {
        oss << std::setw(8) << cpuInfo[i];
    }
    cpuSerial = oss.str();

#elif defined(__linux__)
    std::ifstream file("/proc/cpuinfo");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("Serial") != std::string::npos) {
                std::istringstream iss(line);
                std::string key, value;
                if (std::getline(iss, key, ':') && std::getline(iss, value)) {
                    cpuSerial = value;
                    break;
                }
            }
        }
        file.close();
    }

#elif defined(__APPLE__)
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen("sysctl -n machdep.cpu.brand_string", "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    cpuSerial = result;
#else
#error "Unsupported platform"
#endif

    return cpuSerial;
}

std::string formatUUID(const std::string& uuid) {
    std::string formattedUUID;
    formattedUUID.reserve(36);

    for (size_t i = 0; i < uuid.length(); i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            formattedUUID.push_back('-');
        }
        formattedUUID.push_back(uuid[i]);
    }

    return formattedUUID;
}

std::string generateUniqueUUID() {
    std::string mac = getMAC();
    std::string cpuSerial = getCPUSerial();

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < mac.length(); i += 2) {
        oss << std::setw(2)
            << static_cast<int>(std::stoul(mac.substr(i, 2), nullptr, 16));
    }
    for (size_t i = 0; i < cpuSerial.length(); i += 2) {
        oss << std::setw(2)
            << static_cast<int>(
                   std::stoul(cpuSerial.substr(i, 2), nullptr, 16));
    }

    std::string uuid = oss.str();
    uuid.erase(std::remove_if(uuid.begin(), uuid.end(), ::isspace), uuid.end());
    uuid = formatUUID(uuid);

    return uuid;
}
}  // namespace atom::utils
