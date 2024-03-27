#ifndef ATOM_SYSTEM_MODULE_WIFI_HPP
#define ATOM_SYSTEM_MODULE_WIFI_HPP

#include <string>

namespace Atom::System {
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
}  // namespace Atom::System

#endif