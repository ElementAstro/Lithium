/*
 * gpu.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - GPU

**************************************************/

#include "gpu.hpp"

#ifdef _WIN32
#include <windows.h>
#include <VersionHelpers.h>
#include <setupapi.h>
#elif defined(__linux__)
#include <fstream>
#endif

namespace atom::system {
std::string getGPUInfo() {
    std::string gpuInfo;

#ifdef _WIN32
    if (IsWindows10OrGreater()) {
        // Windows 10 或更高版本
        // 使用 Windows API 获取 GPU 信息
        HDEVINFO deviceInfoSet =
            SetupDiGetClassDevsA(nullptr, "DISPLAY", nullptr, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            return "Failed to get GPU information.";
        }

        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (DWORD i = 0;
             SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); ++i) {
            CHAR buffer[4096];
            DWORD dataSize = sizeof(buffer);
            if (SetupDiGetDeviceRegistryPropertyA(
                    deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, nullptr,
                    (PBYTE)buffer, dataSize, nullptr)) {
                gpuInfo += buffer;
                gpuInfo += "\n";
            }
        }
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    } else {
        gpuInfo =
            "Windows version is not supported for GPU information retrieval.";
    }
#elif defined(__linux__)
    // Linux 平台
    // 读取 GPU 相关文件获取信息
    std::ifstream file("/proc/driver/nvidia/gpus/0/information");
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            gpuInfo += line;
            gpuInfo += "\n";
        }
        file.close();
    } else {
        gpuInfo = "Failed to open GPU information file.";
    }
#else
    // 其他操作系统
    gpuInfo = "GPU information retrieval is not supported on this platform.";
#endif

    return gpuInfo;
}
}  // namespace atom::system
