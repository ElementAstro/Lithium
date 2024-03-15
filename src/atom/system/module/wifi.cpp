#include "wifi.hpp"

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <wlanapi.h>
#pragma comment(lib, "wlanapi.lib")
#elif defined(__linux__)
#include <fstream>
#include <sstream>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/CaptiveNetwork.h>
#endif

#include "atom/log/loguru.hpp"

namespace Atom::System
{
// 获取当前连接的WIFI
std::string getCurrentWifi() {
    std::string wifiName;

#ifdef _WIN32
    DWORD negotiatedVersion;
    HANDLE handle;
    if (WlanOpenHandle(2, nullptr, &negotiatedVersion, &handle) ==
        ERROR_SUCCESS) {
        WLAN_INTERFACE_INFO_LIST* interfaceInfoList;
        if (WlanEnumInterfaces(handle, nullptr, &interfaceInfoList) ==
            ERROR_SUCCESS) {
            for (DWORD i = 0; i < interfaceInfoList->dwNumberOfItems; ++i) {
                WLAN_INTERFACE_INFO* interfaceInfo =
                    &interfaceInfoList->InterfaceInfo[i];
                if (interfaceInfo->isState == wlan_interface_state_connected) {
                    WLAN_CONNECTION_ATTRIBUTES* connectionAttributes;
                    DWORD dataSize = 0;
                    if (WlanQueryInterface(
                            handle, &interfaceInfo->InterfaceGuid,
                            wlan_intf_opcode_current_connection, nullptr,
                            &dataSize,
                            reinterpret_cast<void**>(&connectionAttributes),
                            nullptr) == ERROR_SUCCESS) {
                        wifiName = reinterpret_cast<const char*>(
                            connectionAttributes->wlanAssociationAttributes
                                .dot11Ssid.ucSSID);
                        break;
                    }
                }
            }
        }
        else {
            LOG_F(ERROR, "Error: WlanEnumInterfaces failed");
        }
        WlanCloseHandle(handle, nullptr);
    }
    else {
        LOG_F(ERROR, "Error: WlanOpenHandle failed");
    }
#elif defined(__linux__)
    std::ifstream file("/proc/net/wireless");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::vector<std::string> tokens(
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>());
            if (tokens.size() >= 2 && tokens[1] != "off/any" &&
                tokens[1] != "any") {
                wifiName = tokens[0].substr(0, tokens[0].find(":"));
                break;
            }
        }
    }
#elif defined(__APPLE__)
    CFArrayRef interfaces = CNCopySupportedInterfaces();
    if (interfaces != nullptr) {
        CFDictionaryRef info =
            CNCopyCurrentNetworkInfo(CFArrayGetValueAtIndex(interfaces, 0));
        if (info != nullptr) {
            CFStringRef ssid = static_cast<CFStringRef>(
                CFDictionaryGetValue(info, kCNNetworkInfoKeySSID));
            if (ssid != nullptr) {
                char buffer[256];
                CFStringGetCString(ssid, buffer, sizeof(buffer),
                                   kCFStringEncodingUTF8);
                wifiName = buffer;
            }
            CFRelease(info);
        }
        CFRelease(interfaces);
    }
    else {
        LOG_F(ERROR, "Error: CNCopySupportedInterfaces failed");
    }
#else
    LOG_F(ERROR, "Unsupported operating system");
#endif

    return wifiName;
}

// 获取当前连接的有线网络
std::string getCurrentWiredNetwork() {
    std::string wiredNetworkName;

#ifdef _WIN32
    PIP_ADAPTER_INFO adapterInfo = nullptr;
    ULONG bufferLength = 0;

    if (GetAdaptersInfo(adapterInfo, &bufferLength) == ERROR_BUFFER_OVERFLOW) {
        adapterInfo =
            reinterpret_cast<IP_ADAPTER_INFO*>(new char[bufferLength]);
        if (GetAdaptersInfo(adapterInfo, &bufferLength) == NO_ERROR) {
            for (PIP_ADAPTER_INFO adapter = adapterInfo; adapter != nullptr;
                 adapter = adapter->Next) {
                if (adapter->Type == MIB_IF_TYPE_ETHERNET ) {
                    wiredNetworkName = adapter->AdapterName;
                    break;
                }
            }
        }
        delete[] reinterpret_cast<char*>(adapterInfo);
    }
    else {
        LOG_F(ERROR, "Error: GetAdaptersInfo failed");
    }
#elif defined(__linux__)
    std::ifstream file("/sys/class/net");
    std::string line;
    while (std::getline(file, line)) {
        if (line != "." && line != "..") {
            std::string path = "/sys/class/net/" + line + "/operstate";
            std::ifstream operStateFile(path);
            if (operStateFile.is_open()) {
                std::string operState;
                std::getline(operStateFile, operState);
                if (operState == "up") {
                    wiredNetworkName = line;
                    break;
                }
            }
        }
    }
#elif defined(__APPLE__)
    // macOS下暂不支持获取当前连接的有线网络
#else
    LOG_F(ERROR, "Unsupported operating system");
#endif

    return wiredNetworkName;
}

// 检查是否连接到热点
bool isHotspotConnected() {
    bool isConnected = false;

#ifdef _WIN32
    DWORD negotiatedVersion;
    HANDLE handle;
    if (WlanOpenHandle(2, nullptr, &negotiatedVersion, &handle) ==
        ERROR_SUCCESS) {
        WLAN_INTERFACE_INFO_LIST* interfaceInfoList;
        if (WlanEnumInterfaces(handle, nullptr, &interfaceInfoList) ==
            ERROR_SUCCESS) {
            for (DWORD i = 0; i < interfaceInfoList->dwNumberOfItems; ++i) {
                WLAN_INTERFACE_INFO* interfaceInfo =
                    &interfaceInfoList->InterfaceInfo[i];
                if (interfaceInfo->isState == wlan_interface_state_connected) {
                    WLAN_CONNECTION_ATTRIBUTES* connectionAttributes;
                    DWORD dataSize = 0;
                    if (WlanQueryInterface(
                            handle, &interfaceInfo->InterfaceGuid,
                            wlan_intf_opcode_current_connection, nullptr,
                            &dataSize,
                            reinterpret_cast<void**>(&connectionAttributes),
                            nullptr) == ERROR_SUCCESS) {
                        if (connectionAttributes->isState ==
                                wlan_interface_state_connected &&
                            connectionAttributes->wlanAssociationAttributes
                                    .dot11BssType ==
                                dot11_BSS_type_independent) {
                            isConnected = true;
                            break;
                        }
                    }
                }
            }
        }
        WlanCloseHandle(handle, nullptr);
    }
    else {
        LOG_F(ERROR, "Error: WlanOpenHandle failed");
    }
#elif defined(__linux__)
    std::ifstream file("/proc/net/dev");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(":") != std::string::npos) {
            std::istringstream iss(line);
            std::vector<std::string> tokens(
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>());
            if (tokens.size() >= 17 && tokens[1].substr(0, 5) == "wlx00") {
                isConnected = true;
                break;
            }
        }
    }
#elif defined(__APPLE__)
    // macOS下暂不支持检查是否连接到热点
#else
    LOG_F(ERROR, "Unsupported operating system");
#endif

    return isConnected;
}
}
