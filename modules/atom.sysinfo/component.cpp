#include "atom/components/component.hpp"
#include "atom/components/registry.hpp"

#include "atom/sysinfo/battery.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/gpu.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/sysinfo/sn.hpp"
#include "atom/sysinfo/wifi.hpp"

#include "atom/log/loguru.hpp"

using namespace atom::system;

ATOM_MODULE(atom_io, [](Component &component) {
    DLOG_F(INFO, "Loading module {}", component.getName());

    // ------------------------------------------------------------
    // CPU
    // ------------------------------------------------------------

    component.def("cpu_usage", &getCurrentCpuUsage, "cpu",
                  "Get current CPU usage percentage");
    component.def("cpu_temperature", &getCurrentCpuTemperature, "cpu",
                  "Get current CPU temperature");
    component.def("cpu_model", &getCPUModel, "cpu", "Get CPU model name");
    component.def("cpu_identifier", &getProcessorIdentifier, "cpu",
                  "Get CPU identifier");
    component.def("cpu_frequency", &getProcessorFrequency, "cpu",
                  "Get current CPU frequency");
    component.def("physical_packages", &getNumberOfPhysicalPackages, "cpu",
                  "Get number of physical CPU packages");
    component.def("logical_cpus", &getNumberOfPhysicalCPUs, "cpu",
                  "Get number of logical CPUs");
    component.def("cache_sizes", &getCacheSizes, "cpu", "Get CPU cache sizes");

    // ------------------------------------------------------------
    // Memory
    // ------------------------------------------------------------

    component.def("memory_usage", &getMemoryUsage, "memory",
                  "Get current memory usage percentage");
    component.def("total_memory", &getTotalMemorySize, "memory",
                  "Get total memory size");
    component.def("available_memory", &getAvailableMemorySize, "memory",
                  "Get available memory size");
    component.def("physical_memory_info", &getPhysicalMemoryInfo, "memory",
                  "Get physical memory slot info");
    component.def("virtual_memory_max", &getVirtualMemoryMax, "memory",
                  "Get virtual memory max size");
    component.def("virtual_memory_used", &getVirtualMemoryUsed, "memory",
                  "Get virtual memory used size");
    component.def("swap_memory_total", &getSwapMemoryTotal, "memory",
                  "Get swap memory total size");
    component.def("swap_memory_used", &getSwapMemoryUsed, "memory",
                  "Get swap memory used size");
    component.def("committed_memory", &getCommittedMemory, "memory",
                  "Get committed memory");
    component.def("uncommitted_memory", &getUncommittedMemory, "memory",
                  "Get uncommitted memory");

    component.defType<MemoryInfo>("memory_info");
    component.defType<MemoryInfo::MemorySlot>("memory_slot");
    component.def_v("memory_slot_type", &MemoryInfo::MemorySlot::type,
                    "memory_slot", "Get memory slot type");
    component.def("memory_slot_capacity", &MemoryInfo::MemorySlot::capacity,
                  "memory_slot", "Get memory slot capacity");
    component.def("memory_slot_clock_speed",
                  &MemoryInfo::MemorySlot::clockSpeed, "memory_slot",
                  "Get memory slot clock speed");

    // ------------------------------------------------------------
    // Battery
    // ------------------------------------------------------------

    component.def("get_battery_info", &getBatteryInfo, "battery",
                  "Get battery information");
    component.defType<BatteryInfo>("battery_info");

    // ------------------------------------------------------------
    // Disk
    // ------------------------------------------------------------

    component.def("disk_usage", &getDiskUsage, "disk",
                  "Get current disk usage percentage");
    component.def("get_drive_model", &getDriveModel, "disk", "Get drive model");
    component.def("storage_device_models", &getStorageDeviceModels, "disk",
                  "Get storage device models");
    component.def("available_drives", &getAvailableDrives, "disk",
                  "Get available drives");
    component.def("calculate_disk_usage_percentage",
                  &calculateDiskUsagePercentage, "disk",
                  "Calculate disk usage percentage");
    component.def("file_system_type", &getFileSystemType, "disk",
                  "Get file system type");

    // ------------------------------------------------------------
    // OS
    // ------------------------------------------------------------

    component.def("get_os_info", &getOperatingSystemInfo, "os",
                  "Get operating system information");
    component.def("is_wsl", &isWsl, "os", "Check if running in WSL");
    component.defType<OperatingSystemInfo>("os_info");

    // ------------------------------------------------------------
    // SN
    // ------------------------------------------------------------

    component.def("get_bios_serial_number", &HardwareInfo::getBiosSerialNumber,
                  "sn", "Get bios serial number");
    component.def("get_motherboard_serial_number",
                  &HardwareInfo::getMotherboardSerialNumber, "sn",
                  "Get motherboard serial number");
    component.def("get_cpu_serial_number", &HardwareInfo::getCpuSerialNumber,
                  "sn", "Get cpu serial number");
    component.def("get_disk_serial_numbers",
                  &HardwareInfo::getDiskSerialNumbers, "sn",
                  "Get disk serial numbers");

    // ------------------------------------------------------------
    // Wifi
    // ------------------------------------------------------------

    component.def("is_hotspot_connected", &isHotspotConnected, "wifi",
                  "Check if the hotspot is connected");
    component.def("wired_network", &getCurrentWiredNetwork, "wifi",
                  "Get current wired network");
    component.def("wifi_name", &getCurrentWifi, "wifi",
                  "Get current wifi name");
    component.def("current_ip", &getHostIPs, "network",
                  "Get current IP address");
    component.def("ipv4_addresses", &getIPv4Addresses, "network",
                  "Get IPv4 addresses");
    component.def("ipv6_addresses", &getIPv6Addresses, "network",
                  "Get IPv6 addresses");
    component.def("interface_names", &getInterfaceNames, "network",
                  "Get interface names");

    // ------------------------------------------------------------
    // GPU
    // ------------------------------------------------------------
    component.def("gpu_info", &getGPUInfo, "gpu", "Get GPU info");

    DLOG_F(INFO, "Loaded module {}", component.getName());
});
