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
// clang-format on
#else
#include <cpuid.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <fstream>
#endif

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

    return std::string(vendor.data());
}

// 使用 CPUID 指令检测是否在虚拟机中运行
auto isVirtualMachine() -> bool {
    std::array<unsigned int, 4> cpuInfo = {0};

#ifdef _WIN32
    __cpuid(reinterpret_cast<int*>(cpuInfo.data()), CPUID_FEATURES);  // 调用 CPUID 指令，取页码 1
#else
    __get_cpuid(CPUID_FEATURES, &cpuInfo[0], &cpuInfo[1], &cpuInfo[2], &cpuInfo[3]);
#endif

    // ECX 寄存器的第 31 位表示 Hypervisor present bit
    return static_cast<bool>(cpuInfo[2] & (1u << HYPERVISOR_PRESENT_BIT));
}

// 检查 BIOS 信息以识别虚拟机
auto checkBIOS() -> bool {
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
#ifdef _WIN32
    system("ipconfig /all > network_info.txt");
    std::ifstream netFile("network_info.txt");
    std::string line;
    if (netFile.is_open()) {
        while (std::getline(netFile, line)) {
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos) {
                return true;
            }
        }
        netFile.close();
    }
    if (remove("network_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#else
    system("ip a > network_info.txt");
    std::ifstream netFile("network_info.txt");
    std::string line;
    if (netFile.is_open()) {
        while (std::getline(netFile, line)) {
            if (line.find("virbr") != std::string::npos ||
                line.find("vbox") != std::string::npos ||
                line.find("vmnet") != std::string::npos) {
                return true;
            }
        }
        netFile.close();
    }
    if (remove("network_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#endif
    return false;
}

// 检查磁盘信息：虚拟机常用的磁盘标识
auto checkDisk() -> bool {
#ifdef _WIN32
    system("wmic diskdrive get caption > disk_info.txt");
    std::ifstream diskFile("disk_info.txt");
    std::string line;
    if (diskFile.is_open()) {
        while (std::getline(diskFile, line)) {
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        diskFile.close();
    }
    if (remove("disk_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#else
    system("lsblk -o NAME,MODEL > disk_info.txt");
    std::ifstream diskFile("disk_info.txt");
    std::string line;
    if (diskFile.is_open()) {
        while (std::getline(diskFile, line)) {
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        diskFile.close();
    }
    if (remove("disk_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#endif
    return false;
}

// 检查显卡设备，虚拟机通常使用特定的显卡
auto checkGraphicsCard() -> bool {
#ifdef _WIN32
    system("wmic path win32_videocontroller get caption > gpu_info.txt");
    std::ifstream gpuFile("gpu_info.txt");
    std::string line;
    if (gpuFile.is_open()) {
        while (std::getline(gpuFile, line)) {
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        gpuFile.close();
    }
    if (remove("gpu_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#else
    system("lspci | grep VGA > gpu_info.txt");
    std::ifstream gpuFile("gpu_info.txt");
    std::string line;
    if (gpuFile.is_open()) {
        while (std::getline(gpuFile, line)) {
            if (line.find("VMware") != std::string::npos ||
                line.find("VirtualBox") != std::string::npos ||
                line.find("QEMU") != std::string::npos) {
                return true;
            }
        }
        gpuFile.close();
    }
    if (remove("gpu_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#endif
    return false;
}

// 检查系统中是否存在常见的虚拟机进程
auto checkProcesses() -> bool {
#ifdef _WIN32
    system("tasklist > process_info.txt");
    std::ifstream procFile("process_info.txt");
    std::string line;
    if (procFile.is_open()) {
        while (std::getline(procFile, line)) {
            if (line.find("vmtoolsd.exe") != std::string::npos ||
                line.find("VBoxService.exe") != std::string::npos ||
                line.find("qemu-ga") != std::string::npos) {
                return true;
            }
        }
        procFile.close();
    }
    if (remove("process_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#else
    system("ps aux > process_info.txt");
    std::ifstream procFile("process_info.txt");
    std::string line;
    if (procFile.is_open()) {
        while (std::getline(procFile, line)) {
            if (line.find("vmtoolsd") != std::string::npos ||
                line.find("VBoxService") != std::string::npos ||
                line.find("qemu-ga") != std::string::npos) {
                return true;
            }
        }
        procFile.close();
    }
    if (remove("process_info.txt") != 0) {
        std::cerr << "Error deleting temporary file" << std::endl;
    }
#endif
    return false;
}

// 检查 PCI 总线设备是否为虚拟化设备
auto checkPCIBus() -> bool {
#ifdef _WIN32
    system("wmic path Win32_PnPEntity get Name > pci_info.txt");
    std::ifstream pciFile("pci_info.txt");
#else
    system("lspci > pci_info.txt");  // 在 Linux 上使用 lspci
    std::ifstream pciFile("pci_info.txt");
#endif
    std::string line;
    if (pciFile.is_open()) {
        while (std::getline(pciFile, line)) {
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
        std::cerr << "Error deleting temporary file" << std::endl;
    }
    return false;
}

// 检测系统时间的跳动和偏移，虚拟机的时间管理可能存在问题
auto checkTimeDrift() -> bool {
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
    return duration > TIME_DRIFT_UPPER_BOUND || duration < TIME_DRIFT_LOWER_BOUND;
}
}
