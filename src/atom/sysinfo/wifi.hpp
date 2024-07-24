/*
 * wifi.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Wifi Information

**************************************************/

#ifndef ATOM_SYSTEM_MODULE_WIFI_HPP
#define ATOM_SYSTEM_MODULE_WIFI_HPP

#include <string>
#include <vector>

#include "macro.hpp"

namespace atom::system {
/**
 * @brief Get current wifi name
 * @return Current wifi name
 */
ATOM_NODISCARD auto getCurrentWifi() -> std::string;

/**
 * @brief Get current wired network name
 * @return Current wired network name
 */
ATOM_NODISCARD auto getCurrentWiredNetwork() -> std::string;

/**
 * @brief Check if hotspot is connected
 * @return True if hotspot is connected
 */
ATOM_NODISCARD auto isHotspotConnected() -> bool;

/*
 * @brief Get host IP addresses
 * @return Vector of host IP addresses
 */
ATOM_NODISCARD auto getHostIPs() -> std::vector<std::string>;

/**
 * @brief Get IPv4 addresses
 * @return Vector of IPv4 addresses
 */
ATOM_NODISCARD auto getIPv4Addresses() -> std::vector<std::string>;

/**
 * @brief Get IPv6 addresses
 * @return Vector of IPv6 addresses
 */
ATOM_NODISCARD auto getIPv6Addresses() -> std::vector<std::string>;
}  // namespace atom::system

#endif
