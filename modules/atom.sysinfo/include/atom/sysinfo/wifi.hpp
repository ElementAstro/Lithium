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

namespace atom::system {
/**
 * @brief Get current wifi name
 * @return Current wifi name
 */
[[nodiscard]] std::string getCurrentWifi();

/**
 * @brief Get current wired network name
 * @return Current wired network name
 */
[[nodiscard]] std::string getCurrentWiredNetwork();

/**
 * @brief Check if hotspot is connected
 * @return True if hotspot is connected
 */
[[nodiscard]] bool isHotspotConnected();

/*
 * @brief Get host IP addresses
 * @return Vector of host IP addresses
 */
[[nodiscard]] std::vector<std::string> getHostIPs();

/**
 * @brief Get IPv4 addresses
 * @return Vector of IPv4 addresses
 */
[[nodiscard]] std::vector<std::string> getIPv4Addresses();

/**
 * @brief Get IPv6 addresses
 * @return Vector of IPv6 addresses
 */
[[nodiscard]] std::vector<std::string> getIPv6Addresses();
}  // namespace atom::system

#endif
