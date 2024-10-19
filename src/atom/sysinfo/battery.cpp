/*
 * battery.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Battery

**************************************************/

#include "atom/sysinfo/battery.hpp"

#include <fstream>
#include <string>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <conio.h>
// clang-format on
#elif defined(__APPLE__)
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>
#elif defined(__linux__)
#include <csignal>
#include <cstdio>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
auto getBatteryInfo() -> BatteryInfo {
    LOG_F(INFO, "Starting getBatteryInfo function");
    BatteryInfo info;

#ifdef _WIN32
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus) != 0) {
        LOG_F(INFO, "Successfully retrieved power status");
        info.isBatteryPresent = powerStatus.BatteryFlag != 128;
        info.isCharging =
            powerStatus.BatteryFlag == 8 || powerStatus.ACLineStatus == 1;
        info.batteryLifePercent =
            static_cast<float>(powerStatus.BatteryLifePercent);
        info.batteryLifeTime =
            powerStatus.BatteryLifeTime == 0xFFFFFFFF
                ? 0.0
                : static_cast<float>(powerStatus.BatteryLifeTime);
        info.batteryFullLifeTime =
            powerStatus.BatteryFullLifeTime == 0xFFFFFFFF
                ? 0.0
                : static_cast<float>(powerStatus.BatteryFullLifeTime);
        LOG_F(INFO,
              "Battery Present: %d, Charging: %d, Battery Life Percent: %.2f, "
              "Battery Life Time: %.2f, Battery Full Life Time: %.2f",
              info.isBatteryPresent, info.isCharging, info.batteryLifePercent,
              info.batteryLifeTime, info.batteryFullLifeTime);
    } else {
        LOG_F(ERROR, "Failed to get system power status");
    }
#elif defined(__APPLE__)
    CFTypeRef powerSourcesInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourcesInfo);

    CFIndex count = CFArrayGetCount(powerSources);
    if (count > 0) {
        CFDictionaryRef powerSource = CFArrayGetValueAtIndex(powerSources, 0);

        CFBooleanRef isCharging =
            (CFBooleanRef)CFDictionaryGetValue(powerSource, kIOPSIsChargingKey);
        if (isCharging != nullptr) {
            info.isCharging = CFBooleanGetValue(isCharging);
        }

        CFNumberRef capacity = (CFNumberRef)CFDictionaryGetValue(
            powerSource, kIOPSCurrentCapacityKey);
        if (capacity != nullptr) {
            SInt32 value;
            CFNumberGetValue(capacity, kCFNumberSInt32Type, &value);
            info.batteryLifePercent = static_cast<float>(value);
        }

        CFNumberRef timeToEmpty =
            (CFNumberRef)CFDictionaryGetValue(powerSource, kIOPSTimeToEmptyKey);
        if (timeToEmpty != nullptr) {
            SInt32 value;
            CFNumberGetValue(timeToEmpty, kCFNumberSInt32Type, &value);
            info.batteryLifeTime = static_cast<float>(value) / 60.0f;
        }

        CFNumberRef capacityMax =
            (CFNumberRef)CFDictionaryGetValue(powerSource, kIOPSMaxCapacityKey);
        if (capacityMax != nullptr) {
            SInt32 value;
            CFNumberGetValue(capacityMax, kCFNumberSInt32Type, &value);
            info.batteryFullLifeTime = static_cast<float>(value);
        }

        LOG_F(INFO,
              "Battery Info - Charging: %d, Battery Life Percent: %.2f, "
              "Battery Life Time: %.2f, Battery Full Life Time: %.2f",
              info.isCharging, info.batteryLifePercent, info.batteryLifeTime,
              info.batteryFullLifeTime);
    } else {
        LOG_F(WARNING, "No power sources found");
    }

    CFRelease(powerSources);
    CFRelease(powerSourcesInfo);
#elif defined(__linux__)
    std::ifstream batteryInfo("/sys/class/power_supply/BAT0/uevent");
    if (batteryInfo.is_open()) {
        LOG_F(INFO, "Opened battery info file");
        std::string line;
        while (std::getline(batteryInfo, line)) {
            if (line.find("POWER_SUPPLY_PRESENT") != std::string::npos) {
                info.isBatteryPresent = line.substr(line.find('=') + 1) == "1";
                LOG_F(INFO, "Battery Present: %d", info.isBatteryPresent);
            } else if (line.find("POWER_SUPPLY_STATUS") != std::string::npos) {
                std::string status = line.substr(line.find('=') + 1);
                info.isCharging = status == "Charging" || status == "Full";
                LOG_F(INFO, "Battery Charging: %d", info.isCharging);
            } else if (line.find("POWER_SUPPLY_CAPACITY") !=
                       std::string::npos) {
                info.batteryLifePercent =
                    std::stof(line.substr(line.find('=') + 1));
                LOG_F(INFO, "Battery Life Percent: %.2f",
                      info.batteryLifePercent);
            } else if (line.find("POWER_SUPPLY_TIME_TO_EMPTY_MIN") !=
                       std::string::npos) {
                info.batteryLifeTime =
                    std::stof(line.substr(line.find('=') + 1));
                LOG_F(INFO, "Battery Life Time: %.2f", info.batteryLifeTime);
            } else if (line.find("POWER_SUPPLY_TIME_TO_FULL_NOW") !=
                       std::string::npos) {
                info.batteryFullLifeTime =
                    std::stof(line.substr(line.find('=') + 1));
                LOG_F(INFO, "Battery Full Life Time: %.2f",
                      info.batteryFullLifeTime);
            } else if (line.find("POWER_SUPPLY_ENERGY_NOW") !=
                       std::string::npos) {
                info.energyNow = std::stof(line.substr(line.find('=') + 1));
                LOG_F(INFO, "Energy Now: %.2f", info.energyNow);
            } else if (line.find("POWER_SUPPLY_ENERGY_FULL_DESIGN") !=
                       std::string::npos) {
                info.energyDesign = std::stof(line.substr(line.find('=') + 1));
                LOG_F(INFO, "Energy Design: %.2f", info.energyDesign);
            } else if (line.find("POWER_SUPPLY_VOLTAGE_NOW") !=
                       std::string::npos) {
                info.voltageNow =
                    std::stof(line.substr(line.find('=') + 1)) / 1000000.0f;
                LOG_F(INFO, "Voltage Now: %.2f", info.voltageNow);
            } else if (line.find("POWER_SUPPLY_CURRENT_NOW") !=
                       std::string::npos) {
                info.currentNow =
                    std::stof(line.substr(line.find('=') + 1)) / 1000000.0f;
                LOG_F(INFO, "Current Now: %.2f", info.currentNow);
            }
        }
        batteryInfo.close();
    } else {
        LOG_F(ERROR, "Failed to open battery info file");
    }
#endif
    LOG_F(INFO, "Finished getBatteryInfo function");
    return info;
}
}  // namespace atom::system
