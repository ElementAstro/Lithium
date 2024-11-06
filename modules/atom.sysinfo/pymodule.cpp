#include <pybind11/pybind11.h>

#include "atom/sysinfo/battery.hpp"
#include "atom/sysinfo/bios.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/gpu.hpp"
#include "atom/sysinfo/locale.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/sysinfo/sn.hpp"
#include "atom/sysinfo/wifi.hpp"
#include "atom/sysinfo/wm.hpp"

namespace py = pybind11;
using namespace atom::system;

PYBIND11_MODULE(atom_io, m) {
    // CPU
    m.def("cpu_usage", &getCurrentCpuUsage, "Get current CPU usage percentage");
    m.def("cpu_temperature", &getCurrentCpuTemperature,
          "Get current CPU temperature");
    m.def("cpu_model", &getCPUModel, "Get CPU model name");
    m.def("cpu_identifier", &getProcessorIdentifier, "Get CPU identifier");
    m.def("cpu_frequency", &getProcessorFrequency, "Get current CPU frequency");
    m.def("physical_packages", &getNumberOfPhysicalPackages,
          "Get number of physical CPU packages");
    m.def("logical_cpus", &getNumberOfPhysicalCPUs,
          "Get number of logical CPUs");
    m.def("cache_sizes", &getCacheSizes, "Get CPU cache sizes");

    // Memory
    m.def("memory_usage", &getMemoryUsage,
          "Get current memory usage percentage");
    m.def("total_memory", &getTotalMemorySize, "Get total memory size");
    m.def("available_memory", &getAvailableMemorySize,
          "Get available memory size");
    m.def("physical_memory_info", &getPhysicalMemoryInfo,
          "Get physical memory slot info");
    m.def("virtual_memory_max", &getVirtualMemoryMax,
          "Get virtual memory max size");
    m.def("virtual_memory_used", &getVirtualMemoryUsed,
          "Get virtual memory used size");
    m.def("swap_memory_total", &getSwapMemoryTotal,
          "Get swap memory total size");
    m.def("swap_memory_used", &getSwapMemoryUsed, "Get swap memory used size");
    m.def("committed_memory", &getCommittedMemory, "Get committed memory");
    m.def("uncommitted_memory", &getUncommittedMemory,
          "Get uncommitted memory");

    py::class_<MemoryInfo>(m, "MemoryInfo");
    py::class_<MemoryInfo::MemorySlot>(m, "MemorySlot")
        .def_readonly("type", &MemoryInfo::MemorySlot::type)
        .def_readonly("capacity", &MemoryInfo::MemorySlot::capacity)
        .def_readonly("clock_speed", &MemoryInfo::MemorySlot::clockSpeed);

    // Battery
    m.def("get_battery_info", &getBatteryInfo, "Get battery information");
    py::class_<BatteryInfo>(m, "BatteryInfo");

    // Disk
    m.def("disk_usage", &getDiskUsage, "Get current disk usage percentage");
    m.def("get_drive_model", &getDriveModel, "Get drive model");
    m.def("storage_device_models", &getStorageDeviceModels,
          "Get storage device models");
    m.def("available_drives", &getAvailableDrives, "Get available drives");
    m.def("calculate_disk_usage_percentage", &calculateDiskUsagePercentage,
          "Calculate disk usage percentage");
    m.def("file_system_type", &getFileSystemType, "Get file system type");

    // OS
    m.def("get_os_info", &getOperatingSystemInfo,
          "Get operating system information");
    m.def("is_wsl", &isWsl, "Check if running in WSL");
    py::class_<OperatingSystemInfo>(m, "OperatingSystemInfo");

    // SN
    m.def("get_bios_serial_number", &HardwareInfo::getBiosSerialNumber,
          "Get bios serial number");
    m.def("get_motherboard_serial_number",
          &HardwareInfo::getMotherboardSerialNumber,
          "Get motherboard serial number");
    m.def("get_cpu_serial_number", &HardwareInfo::getCpuSerialNumber,
          "Get cpu serial number");
    m.def("get_disk_serial_numbers", &HardwareInfo::getDiskSerialNumbers,
          "Get disk serial numbers");

    // Wifi
    m.def("is_hotspot_connected", &isHotspotConnected,
          "Check if the hotspot is connected");
    m.def("wired_network", &getCurrentWiredNetwork,
          "Get current wired network");
    m.def("wifi_name", &getCurrentWifi, "Get current wifi name");
    m.def("current_ip", &getHostIPs, "Get current IP address");
    m.def("ipv4_addresses", &getIPv4Addresses, "Get IPv4 addresses");
    m.def("ipv6_addresses", &getIPv6Addresses, "Get IPv6 addresses");
    m.def("interface_names", &getInterfaceNames, "Get interface names");

    // GPU
    m.def("get_gpu_info", &getGPUInfo, "Get GPU information");

    py::class_<MonitorInfo>(m, "MonitorInfo")
        .def(py::init<>())
        .def_readwrite("model", &MonitorInfo::model)
        .def_readwrite("identifier", &MonitorInfo::identifier)
        .def_readwrite("width", &MonitorInfo::width)
        .def_readwrite("height", &MonitorInfo::height)
        .def_readwrite("refresh_rate", &MonitorInfo::refreshRate);

    m.def("get_all_monitors_info", &getAllMonitorsInfo,
          "Get all monitors information");

    py::class_<SystemInfo>(m, "SystemInfo")
        .def(py::init<>())
        .def_readwrite("desktop_environment", &SystemInfo::desktopEnvironment)
        .def_readwrite("window_manager", &SystemInfo::windowManager)
        .def_readwrite("wm_theme", &SystemInfo::wmTheme)
        .def_readwrite("icons", &SystemInfo::icons)
        .def_readwrite("font", &SystemInfo::font)
        .def_readwrite("cursor", &SystemInfo::cursor);

    m.def("get_system_info", &getSystemInfo, "Get system information");

    py::class_<BiosInfoData>(m, "BiosInfoData")
        .def(py::init<>())
        .def_readwrite("version", &BiosInfoData::version)
        .def_readwrite("manufacturer", &BiosInfoData::manufacturer)
        .def_readwrite("release_date", &BiosInfoData::releaseDate);

    m.def("get_bios_info", &getBiosInfo, "Get BIOS information");

    py::class_<LocaleInfo>(m, "LocaleInfo")
        .def(py::init<>())
        .def_readwrite("language_code", &LocaleInfo::languageCode)
        .def_readwrite("country_code", &LocaleInfo::countryCode)
        .def_readwrite("locale_name", &LocaleInfo::localeName)
        .def_readwrite("language_display_name",
                       &LocaleInfo::languageDisplayName)
        .def_readwrite("country_display_name", &LocaleInfo::countryDisplayName)
        .def_readwrite("currency_symbol", &LocaleInfo::currencySymbol)
        .def_readwrite("decimal_symbol", &LocaleInfo::decimalSymbol)
        .def_readwrite("thousand_separator", &LocaleInfo::thousandSeparator)
        .def_readwrite("date_format", &LocaleInfo::dateFormat)
        .def_readwrite("time_format", &LocaleInfo::timeFormat)
        .def_readwrite("character_encoding", &LocaleInfo::characterEncoding);

    m.def("get_system_language_info", &getSystemLanguageInfo,
          "Get system language information");
    m.def("print_locale_info", &printLocaleInfo, "Print locale information",
          py::arg("info"));
}
