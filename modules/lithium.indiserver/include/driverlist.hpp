#ifndef LITHIUM_INDISERVER_DRIVERLIST_HPP
#define LITHIUM_INDISERVER_DRIVERLIST_HPP

#include <string>
#include <vector>

#include "atom/macro.hpp"

struct Device {
    std::string label;
    std::string manufacturer;
    std::string driverName;
    std::string version;
} ATOM_ALIGNAS(128);

struct DevGroup {
    std::string group;
    std::vector<Device> devices;
} ATOM_ALIGNAS(64);

struct DriversList {
    std::vector<DevGroup> devGroups;
} ATOM_ALIGNAS(32);

auto readDriversListFromFiles(std::string_view filename, std::string_view path)
    -> std::tuple<DriversList, std::vector<DevGroup>, std::vector<Device>>;

#endif
