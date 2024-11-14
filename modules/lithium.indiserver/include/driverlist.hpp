#ifndef LITHIUM_INDISERVER_DRIVERLIST_HPP
#define LITHIUM_INDISERVER_DRIVERLIST_HPP

#include <string>
#include <vector>

#include "atom/macro.hpp"

/**
 * @struct Device
 * @brief A structure to hold information about an INDI device.
 *
 * This structure contains details about an INDI device, including its label,
 * manufacturer, driver name, and version.
 */
struct Device {
    std::string label;         ///< The label of the device.
    std::string manufacturer;  ///< The manufacturer of the device.
    std::string driverName;    ///< The name of the driver.
    std::string version;       ///< The version of the device.
} ATOM_ALIGNAS(128);

/**
 * @struct DevGroup
 * @brief A structure to hold a group of INDI devices.
 *
 * This structure contains a group name and a list of devices belonging to that
 * group.
 */
struct DevGroup {
    std::string group;            ///< The name of the device group.
    std::vector<Device> devices;  ///< A list of devices in the group.
} ATOM_ALIGNAS(64);

/**
 * @struct DriversList
 * @brief A structure to hold a list of device groups.
 *
 * This structure contains a list of device groups, each containing multiple
 * devices.
 */
struct DriversList {
    std::vector<DevGroup> devGroups;  ///< A list of device groups.
} ATOM_ALIGNAS(32);

/**
 * @brief Parses a drivers list from a file.
 * @param filename The path to the file containing the drivers list.
 * @return A vector of DevGroup structures parsed from the file.
 */
auto parseDriversList(const std::string& filename) -> std::vector<DevGroup>;

/**
 * @brief Parses devices from a specified path.
 * @param path The directory path to search for device files.
 * @param devicesFrom A reference to a vector of Device structures to store the
 * parsed devices.
 * @return A vector of DevGroup structures parsed from the specified path.
 */
auto parseDevicesFromPath(const std::string& path,
                          std::vector<Device>& devicesFrom)
    -> std::vector<DevGroup>;

/**
 * @brief Reads a drivers list from files.
 * @param filename The path to the file containing the drivers list.
 * @param path The directory path to search for additional device files.
 * @return A tuple containing a DriversList structure, a vector of DevGroup
 * structures, and a vector of Device structures.
 */
auto readDriversListFromFiles(std::string_view filename, std::string_view path)
    -> std::tuple<DriversList, std::vector<DevGroup>, std::vector<Device>>;

#endif  // LITHIUM_INDISERVER_DRIVERLIST_HPP