#include "virtual.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <array>
#include <thread>
#include <algorithm>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <intrin.h>
#include <tchar.h>
#include <fstream>
#include <cstdlib>
// clang-format on
#else
#include <cpuid.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fstream>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
constexpr int CPUID_HYPERVISOR = 0x40000000;
constexpr int CPUID_FEATURES = 1;
constexpr int VENDOR_STRING_LENGTH = 12;
constexpr int BIOS_INFO_LENGTH = 256;
constexpr int HYPERVISOR_PRESENT_BIT = 31;
constexpr int TIME_DRIFT_UPPER_BOUND = 1005;
constexpr int TIME_DRIFT_LOWER_BOUND = 995;

// 获取 Hypervisor 厂商信息
auto getHypervisorVendor() -> std::string {
    LOG_F(INFO, "Starting getHypervisorVendor function");
    std::array<unsigned int, 4> cpuInfo = {0};

#ifdef _WIN32
    __cpuid(reinterpret_cast<int*>(cpuInfo.data()), CPUID_HYPERVISOR);  // Hypervisor CPUID
#else
    __get_cpuid(CPUID_HYPERVISOR, &cpuInfo[0], &cpuInfo[1], &cpuInfo[2], &cpuInfo[3]);
#endif

    std::array<char, VENDOR_STRING_LENGTH + 1> vendor = {0};
    std::copy(reinterpret_cast<const char*>(&cpuInfo[1]), reinterpret_cast<const char*>(&cpuInfo[1]) + 4, vendor.begin());
    std::copy(reinterpret_cast<const char*>(&cpuInfo[2]), reinterpret_cast<const char*>(&cpuInfo[2]) + 4, vendor.begin() + 4);
    std::copy(reinterpret_cast<const char*>(&cpuInfo[3]), reinterpret_cast<const char*>(&cpuInfo[3]) + 4, vendor.begin() + 8);

    std::string vendorStr(vendor.data());
    LOG_F(INFO, "Hypervisor vendor: {}", vendorStr);
    return vendorStr;
}

// 使用 CPUID 指令检测是否在虚拟机中运行
auto isVirtualMachine() -> bool {
    LOG_F(INFO, "Starting isVirtualMachine function");
    std::array<unsigned int, 4> cpuInfo = {0};

#ifdef _WIN32
    __cpuid(reinterpret_cast<int*>(cpuInfo.data()), CPUID_FEATURES);  // 调用 CPUID 指令，取页码 1
#else
    __get_cpuid(CPUID_FEATURES, &cpuInfo[0], &cpuInfo[1], &cpuInfo[2], &cpuInfo[3]);
#endif

    bool isVM = static_cast<bool>(cpuInfo[2] & (1u << HYPERVISOR_PRESENT_BIT));
    LOG_F(INFO, "Is virtual machine: {}", isVM);
    return isVM;
}

// 检查 BIOS 信息以识别虚拟机
auto checkBIOS() -> bool {
    LOG_F(INFO, "Starting checkBIOS function");
#ifdef _WIN32
    HKEY hKey;
    std::array<TCHAR, BIOS_INFO_LENGTH> biosInfo;
    DWORD bufSize = sizeof(biosInfo);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     _T("HARDWARE\\DESCRIPTION\\System\\BIOS"), 0, KEY_READ,
                     &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, _T("SystemManufacturer"), nullptr, nullptr,
                            reinterpret_cast<LPBYTE>(biosInfo.data()), &bufSize) == ERROR_SUCCESS) {
            std::string bios(biosInfo.data());
            LOG_F(INFO, "BIOS SystemManufacturer: {}", bios);
            if (bios.find("VMware") != std::string::npos ||
                bios.find("VirtualBox") != std::string::npos ||
                bios.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        RegCloseKey(hKey);
    }
#else
    std::ifstream file("/sys/class/dmi/id/product_name");
    std::string biosInfo;
    if (file.is_open()) {
        std::getline(file, biosInfo);
        file.close();
        LOG_F(INFO, "BIOS product name: {}", biosInfo);
        if (biosInfo.find("VMware") != std::string::npos ||
            biosInfo.find("VirtualBox") != std::string::npos ||
            biosInfo.find("QEMU") != std::string::npos) {
            return true;
        }
    }
#endif
    return false;
}

// 检查网络适配器，常见的虚拟机适配器如 "VMware Virtual Ethernet Adapter"
auto checkNetworkAdapter() -> bool {
    LOG_F(INFO, "Starting checkNetworkAdapter function");
#ifdef _WIN32
    std::system("ipconfig /all > network_info.txt");
    std::ifstream netFile("network_info.txt");
    std::string line;
    if (netFile.is_open()) {
        while (std::getline(netFile, line)) {
            LOG_F(INFO, "Network adapter info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos) {
                return true;
            }
        }
        netFile.close();
    }
    if (remove("network_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file network_info.txt");
    }
#else
    std::system("ip a > network_info.txt");
    std::ifstream netFile("network_info.txt");
    std::string line;
    if (netFile.is_open()) {
        while (std::getline(netFile, line)) {
            LOG_F(INFO, "Network adapter info: {}", line);
            if (line.find("virbr") != std::string::npos ||
                line.find("vbox") != std::string::npos ||
                line.find("vmnet") != std::string::npos) {
                return true;
            }
        }
        netFile.close();
    }
    if (remove("network_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file network_info.txt");
    }
#endif
    return false;
}

// 检查磁盘信息：虚拟机常用的磁盘标识
auto checkDisk() -> bool {
    LOG_F(INFO, "Starting checkDisk function");
#ifdef _WIN32
    std::system("wmic diskdrive get caption > disk_info.txt");
    std::ifstream diskFile("disk_info.txt");
    std::string line;
    if (diskFile.is_open()) {
        while (std::getline(diskFile, line)) {
            LOG_F(INFO, "Disk info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        diskFile.close();
    }
    if (remove("disk_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file disk_info.txt");
    }
#else
    std::system("lsblk -o NAME,MODEL > disk_info.txt");
    std::ifstream diskFile("disk_info.txt");
    std::string line;
    if (diskFile.is_open()) {
        while (std::getline(diskFile, line)) {
            LOG_F(INFO, "Disk info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        diskFile.close();
    }
    if (remove("disk_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file disk_info.txt");
    }
#endif
    return false;
}

// 检查显卡设备，虚拟机通常使用特定的显卡
auto checkGraphicsCard() -> bool {
    LOG_F(INFO, "Starting checkGraphicsCard function");
#ifdef _WIN32
    std::system("wmic path win32_videocontroller get caption > gpu_info.txt");
    std::ifstream gpuFile("gpu_info.txt");
    std::string line;
    if (gpuFile.is_open()) {
        while (std::getline(gpuFile, line)) {
            LOG_F(INFO, "Graphics card info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        gpuFile.close();
    }
    if (remove("gpu_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file gpu_info.txt");
    }
#else
    std::system("lspci | grep VGA > gpu_info.txt");
    std::ifstream gpuFile("gpu_info.txt");
    std::string line;
    if (gpuFile.is_open()) {
        while (std::getline(gpuFile, line)) {
            LOG_F(INFO, "Graphics card info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        gpuFile.close();
    }
    if (remove("gpu_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file gpu_info.txt");
    }
#endif
    return false;
}

// 检查系统中是否存在常见的虚拟机进程
auto checkProcesses() -> bool {
    LOG_F(INFO, "Starting checkProcesses function");
#ifdef _WIN32
    std::system("tasklist > process_info.txt");
    std::ifstream procFile("process_info.txt");
    std::string line;
    if (procFile.is_open()) {
        while (std::getline(procFile, line)) {
            LOG_F(INFO, "Process info: {}", line);
            if (line.find("vmtoolsd.exe") != std::string::npos ||
                line.find("VBoxService.exe") != std::string::npos ||
                line.find("qemu-ga") != std::string::npos) {
                return true;
            }
        }
        procFile.close();
    }
    if (remove("process_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file process_info.txt");
    }
#else
    std::system("ps aux > process_info.txt");
    std::ifstream procFile("process_info.txt");
    std::string line;
    if (procFile.is_open()) {
        while (std::getline(procFile, line)) {
            LOG_F(INFO, "Process info: {}", line);
            if (line.find("vmtoolsd") != std::string::npos ||
                line.find("VBoxService") != std::string::npos ||
                line.find("qemu-ga") != std::string::npos) {
                return true;
            }
        }
        procFile.close();
    }
    if (remove("process_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file process_info.txt");
    }
#endif
    return false;
}

// 检查 PCI 总线设备是否为虚拟化设备
auto checkPCIBus() -> bool {
    LOG_F(INFO, "Starting checkPCIBus function");
#ifdef _WIN32
    std::system("wmic path Win32_PnPEntity get Name > pci_info.txt");
    std::ifstream pciFile("pci_info.txt");
#else
    std::system("lspci > pci_info.txt");  // 在 Linux 上使用 lspci
    std::ifstream pciFile("pci_info.txt");
#endif
    std::string line;
    if (pciFile.is_open()) {
        while (std::getline(pciFile, line)) {
            LOG_F(INFO, "PCI bus info: {}", line);
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos ||
                line.find("Xen") != std::string::npos ||
                line.find("KVM") != std::string::npos) {
                return true;
            }
        }
        pciFile.close();
    }
    if (remove("pci_info.txt") != 0) {
        LOG_F(ERROR, "Error deleting temporary file pci_info.txt");
    }
    return false;
}

// 检测系统时间的跳动和偏移，虚拟机的时间管理可能存在问题
auto checkTimeDrift() -> bool {
    LOG_F(INFO, "Starting checkTimeDrift function");
    // 获取两次系统时间，检测它们的差值是否合理
    auto start = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
    Sleep(1000);  // 暂停 1 秒
#else
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 暂停 1 秒
#endif
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // 如果时间跳动太大，可能是虚拟机中常见的时间管理问题
    bool timeDrift = duration > TIME_DRIFT_UPPER_BOUND || duration < TIME_DRIFT_LOWER_BOUND;
    LOG_F(INFO, "Time drift detected: {}", timeDrift);
    return timeDrift;
}
}
