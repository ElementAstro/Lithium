/*
 * gpu.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - GPU

**************************************************/

#include "atom/sysinfo/gpu.hpp"

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <versionHelpers.h>
#include <setupapi.h>
// clang-format on
#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#if __has_include(<X11/extensions/Xrandr.h>)
#include <X11/extensions/Xrandr.h>
#endif
#include <fstream>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {

auto getGPUInfo() -> std::string {
    LOG_F(INFO, "Starting getGPUInfo function");
    std::string gpuInfo;

#ifdef _WIN32
    if (IsWindows10OrGreater()) {
        LOG_F(INFO, "Windows 10 or greater detected");
        HDEVINFO deviceInfoSet =
            SetupDiGetClassDevsA(nullptr, "DISPLAY", nullptr, DIGCF_PRESENT);
        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            LOG_F(ERROR, "Failed to get GPU information: INVALID_HANDLE_VALUE");
            return "Failed to get GPU information.";
        }

        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        for (DWORD i = 0;
             SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData) != 0;
             ++i) {
            CHAR buffer[4096];
            DWORD dataSize = sizeof(buffer);
            if (SetupDiGetDeviceRegistryPropertyA(
                    deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, nullptr,
                    (PBYTE)buffer, dataSize, nullptr) != 0) {
                gpuInfo += buffer;
                gpuInfo += "\n";
                LOG_F(INFO, "GPU Info: {}", buffer);
            }
        }
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
    } else {
        gpuInfo =
            "Windows version is not supported for GPU information retrieval.";
        LOG_F(
            WARNING,
            "Windows version is not supported for GPU information retrieval.");
    }
#elif defined(__linux__)
    LOG_F(INFO, "Linux detected");
    std::ifstream file("/proc/driver/nvidia/gpus/0/information");
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            gpuInfo += line;
            gpuInfo += "\n";
            LOG_F(INFO, "GPU Info: {}", line);
        }
        file.close();
    } else {
        gpuInfo = "Failed to open GPU information file.";
        LOG_F(ERROR, "Failed to open GPU information file.");
    }
#else
    gpuInfo = "GPU information retrieval is not supported on this platform.";
    LOG_F(WARNING,
          "GPU information retrieval is not supported on this platform.");
#endif

    LOG_F(INFO, "Finished getGPUInfo function");
    return gpuInfo;
}

#ifdef _WIN32
auto getMonitorModel(const DISPLAY_DEVICE& displayDevice) -> std::string {
    LOG_F(INFO, "Getting monitor model: {}", displayDevice.DeviceString);
    return {displayDevice.DeviceString};
}

void getMonitorResolutionAndRefreshRate(const std::string& deviceName,
                                        int& width, int& height,
                                        int& refreshRate) {
    LOG_F(INFO, "Getting monitor resolution and refresh rate for device: {}",
          deviceName);
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (EnumDisplaySettings(deviceName.c_str(), ENUM_CURRENT_SETTINGS,
                            &devMode)) {
        width = static_cast<int>(devMode.dmPelsWidth);
        height = static_cast<int>(devMode.dmPelsHeight);
        refreshRate = static_cast<int>(devMode.dmDisplayFrequency);
        LOG_F(INFO, "Resolution: {}x{}, Refresh Rate: {}", width, height,
              refreshRate);
    } else {
        LOG_F(ERROR, "Failed to get display settings for device: {}",
              deviceName);
    }
}

auto getAllMonitorsInfo() -> std::vector<MonitorInfo> {
    LOG_F(INFO, "Starting getAllMonitorsInfo function");
    std::vector<MonitorInfo> monitors;
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(displayDevice);
    int deviceIndex = 0;

    while (EnumDisplayDevices(nullptr, deviceIndex, &displayDevice, 0)) {
        if ((displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) != 0u) {
            MonitorInfo info;
            info.model = getMonitorModel(displayDevice);
            info.identifier = std::string(displayDevice.DeviceName);

            int width = 0;
            int height = 0;
            int refreshRate = 0;
            getMonitorResolutionAndRefreshRate(displayDevice.DeviceName, width,
                                               height, refreshRate);

            info.width = width;
            info.height = height;
            info.refreshRate = refreshRate;

            monitors.push_back(info);
            LOG_F(INFO,
                  "Monitor Info - Model: {}, Identifier: {}, Resolution: "
                  "{}x{}, Refresh Rate: {}",
                  info.model, info.identifier, info.width, info.height,
                  info.refreshRate);
        }
        deviceIndex++;
        ZeroMemory(&displayDevice, sizeof(displayDevice));
        displayDevice.cb = sizeof(displayDevice);
    }

    LOG_F(INFO, "Finished getAllMonitorsInfo function");
    return monitors;
}

#elif __linux__
auto getAllMonitorsInfo() -> std::vector<MonitorInfo> {
    LOG_F(INFO, "Starting getAllMonitorsInfo function");
    std::vector<MonitorInfo> monitors;

#if __has_include(<X11/extensions/Xrandr.h>)
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        LOG_F(ERROR, "Unable to open X display");
        return monitors;
    }
    Window root = DefaultRootWindow(display);
    XRRScreenResources* screenRes = XRRGetScreenResources(display, root);
    if (screenRes == nullptr) {
        XCloseDisplay(display);
        LOG_F(ERROR, "Unable to get screen resources");
        return monitors;
    }

    for (int i = 0; i < screenRes->noutput; ++i) {
        XRROutputInfo* outputInfo =
            XRRGetOutputInfo(display, screenRes, screenRes->outputs[i]);
        if (outputInfo == nullptr ||
            outputInfo->connection == RR_Disconnected) {
            continue;
        }

        MonitorInfo info;
        info.model = std::string(outputInfo->name);
        info.identifier = std::string(outputInfo->name);

        if (outputInfo->crtc) {
            XRRCrtcInfo* crtcInfo =
                XRRGetCrtcInfo(display, screenRes, outputInfo->crtc);
            if (crtcInfo) {
                info.width = crtcInfo->width;
                info.height = crtcInfo->height;
                info.refreshRate = static_cast<int>(
                    crtcInfo->mode == None ? 0 : crtcInfo->rotation);
                XRRFreeCrtcInfo(crtcInfo);
                LOG_F(INFO,
                      "Monitor Info - Model: {}, Identifier: {}, Resolution: "
                      "{}x{}, Refresh Rate: {}",
                      info.model, info.identifier, info.width, info.height,
                      info.refreshRate);
            }
        }

        monitors.push_back(info);
        XRRFreeOutputInfo(outputInfo);
    }

    XRRFreeScreenResources(screenRes);
    XCloseDisplay(display);
    LOG_F(INFO, "Finished getAllMonitorsInfo function");
#else
    LOG_F(ERROR, "Xrandr extension not found");
#endif
    return monitors;
}

#elif __APPLE__
auto getAllMonitorsInfo() -> std::vector<MonitorInfo> {
    LOG_F(INFO, "Starting getAllMonitorsInfo function");
    std::vector<MonitorInfo> monitors;

    uint32_t displayCount;
    CGGetActiveDisplayList(0, nullptr, &displayCount);
    std::vector<CGDirectDisplayID> displays(displayCount);
    CGGetActiveDisplayList(displayCount, displays.data(), &displayCount);

    for (uint32_t i = 0; i < displayCount; ++i) {
        CGDirectDisplayID displayID = displays[i];
        MonitorInfo info;

        info.identifier = std::to_string(displayID);
        info.width = static_cast<int>(CGDisplayPixelsWide(displayID));
        info.height = static_cast<int>(CGDisplayPixelsHigh(displayID));
        info.refreshRate = static_cast<int>(
            CGDisplayModeGetRefreshRate(CGDisplayCopyDisplayMode(displayID)));

        info.model = "Unknown";

        monitors.push_back(info);
        LOG_F(INFO,
              "Monitor Info - Identifier: {}, Resolution: {}x{}, Refresh Rate: "
              "{}",
              info.identifier, info.width, info.height, info.refreshRate);
    }

    LOG_F(INFO, "Finished getAllMonitorsInfo function");
    return monitors;
}

#endif
}  // namespace atom::system
