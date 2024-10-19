/*
 * battery.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Battery

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_BATTERY_HPP
#define ATOM_SYSTEM_MODULE_BATTERY_HPP

#include "macro.hpp"

namespace atom::system {
/**
 * @brief Battery information.
 */
struct BatteryInfo {
    bool isBatteryPresent = false;    // Whether the battery is present
    bool isCharging = false;          // Whether the battery is charging
    float batteryLifePercent = 0.0;   // Battery life percentage
    float batteryLifeTime = 0.0;      // Remaining battery life time (minutes)
    float batteryFullLifeTime = 0.0;  // Full battery life time (minutes)
    float energyNow = 0.0;            // Current remaining energy (microjoules)
    float energyFull = 0.0;           // Total battery capacity (microjoules)
    float energyDesign = 0.0;         // Designed battery capacity (microjoules)
    float voltageNow = 0.0;           // Current voltage (volts)
    float currentNow = 0.0;           // Current battery current (amperes)
} ATOM_ALIGNAS(64);

/**
 * @brief Get battery information.
 * @return BatteryInfo
 */
[[nodiscard("Result of getBatteryInfo is not used")]] BatteryInfo
getBatteryInfo();
}  // namespace atom::system
#endif
