/*
 * battery.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Battery

**************************************************/

#include "battery.hpp"

#include <functional>
#include <memory>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <cstdio>
#include <csignal>
#endif

namespace Atom::System
{
BatteryInfo getBatteryInfo()
    {
        BatteryInfo info;

#ifdef _WIN32
        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus))
        {
            info.isBatteryPresent = powerStatus.BatteryFlag != 128;
            info.isCharging = powerStatus.BatteryFlag == 8 || powerStatus.ACLineStatus == 1;
            info.batteryLifePercent = static_cast<float>(powerStatus.BatteryLifePercent);
            info.batteryLifeTime = powerStatus.BatteryLifeTime == 0xFFFFFFFF ? 0.0 : static_cast<float>(powerStatus.BatteryLifeTime);
            info.batteryFullLifeTime = powerStatus.BatteryFullLifeTime == 0xFFFFFFFF ? 0.0 : static_cast<float>(powerStatus.BatteryFullLifeTime);
            // 其他电池信息...
        }
#else
        std::ifstream batteryInfo("/sys/class/power_supply/BAT0/uevent");
        if (batteryInfo.is_open())
        {
            std::string line;
            while (std::getline(batteryInfo, line))
            {
                if (line.find("POWER_SUPPLY_PRESENT") != std::string::npos)
                {
                    info.isBatteryPresent = line.substr(line.find("=") + 1) == "1";
                }
                else if (line.find("POWER_SUPPLY_STATUS") != std::string::npos)
                {
                    std::string status = line.substr(line.find("=") + 1);
                    info.isCharging = status == "Charging" || status == "Full";
                }
                else if (line.find("POWER_SUPPLY_CAPACITY") != std::string::npos)
                {
                    info.batteryLifePercent = std::stof(line.substr(line.find("=") + 1));
                }
                else if (line.find("POWER_SUPPLY_TIME_TO_EMPTY_MIN") != std::string::npos)
                {
                    info.batteryLifeTime = std::stof(line.substr(line.find("=") + 1));
                }
                else if (line.find("POWER_SUPPLY_TIME_TO_FULL_NOW") != std::string::npos)
                {
                    info.batteryFullLifeTime = std::stof(line.substr(line.find("=") + 1));
                }
                else if (line.find("POWER_SUPPLY_ENERGY_NOW") != std::string::npos)
                {
                    info.energyNow = std::stof(line.substr(line.find("=") + 1));
                }
                else if (line.find("POWER_SUPPLY_ENERGY_FULL_DESIGN") != std::string::npos)
                {
                    info.energyDesign = std::stof(line.substr(line.find("=") + 1));
                }
                else if (line.find("POWER_SUPPLY_VOLTAGE_NOW") != std::string::npos)
                {
                    info.voltageNow = std::stof(line.substr(line.find("=") + 1)) / 1000000.0f;
                }
                else if (line.find("POWER_SUPPLY_CURRENT_NOW") != std::string::npos)
                {
                    info.currentNow = std::stof(line.substr(line.find("=") + 1)) / 1000000.0f;
                }
            }
        }
#endif
        return info;
    }
}