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

namespace atom::system {
/**
 * @brief Battery information.
 * 电池信息
 */
struct BatteryInfo {
    bool isBatteryPresent = false;    // 是否存在电池
    bool isCharging = false;          // 是否正在充电
    float batteryLifePercent = 0.0;   // 电量百分比
    float batteryLifeTime = 0.0;      // 剩余电量时间(分钟)
    float batteryFullLifeTime = 0.0;  // 满电状态下电量时间(分钟)
    float energyNow = 0.0;            // 当前剩余电量(微焦耳)
    float energyFull = 0.0;           // 电池总容量(微焦耳)
    float energyDesign = 0.0;         // 电池设计容量(微焦耳)
    float voltageNow = 0.0;           // 当前电压(伏特)
    float currentNow = 0.0;           // 电池当前电流(安培)
};

BatteryInfo getBatteryInfo();
}  // namespace atom::system
#endif