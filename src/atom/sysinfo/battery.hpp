#ifndef ATOM_SYSTEM_MODULE_BATTERY_HPP
#define ATOM_SYSTEM_MODULE_BATTERY_HPP

#include "atom/macro.hpp"

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

    /**
     * @brief Default constructor.
     */
    BatteryInfo() = default;

    auto operator==(const BatteryInfo& other) const -> bool {
        return isBatteryPresent == other.isBatteryPresent &&
               isCharging == other.isCharging &&
               batteryLifePercent == other.batteryLifePercent &&
               batteryLifeTime == other.batteryLifeTime &&
               batteryFullLifeTime == other.batteryFullLifeTime &&
               energyNow == other.energyNow && energyFull == other.energyFull &&
               energyDesign == other.energyDesign &&
               voltageNow == other.voltageNow && currentNow == other.currentNow;
    }

    auto operator!=(const BatteryInfo& other) const -> bool {
        return !(*this == other);
    }

    auto operator=(const BatteryInfo& other) -> BatteryInfo& {
        if (this != &other) {
            isBatteryPresent = other.isBatteryPresent;
            isCharging = other.isCharging;
            batteryLifePercent = other.batteryLifePercent;
            batteryLifeTime = other.batteryLifeTime;
            batteryFullLifeTime = other.batteryFullLifeTime;
            energyNow = other.energyNow;
            energyFull = other.energyFull;
            energyDesign = other.energyDesign;
            voltageNow = other.voltageNow;
            currentNow = other.currentNow;
        }
        return *this;
    }
} ATOM_ALIGNAS(64);

/**
 * @brief Get battery information.
 * @return BatteryInfo
 */
[[nodiscard("Result of getBatteryInfo is not used")]] BatteryInfo
getBatteryInfo();
}  // namespace atom::system
#endif
