#include "atom/components/component.hpp"
#include <dlgs.h>
#include "atom/components/registry.hpp"

#include "atom/sysinfo/battery.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/gpu.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
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
    component.def_v("memory_slot_type", &MemoryInfo::MemorySlot::type, "memory_slot",
                  "Get memory slot type");
    component.def("memory_slot_capacity", &MemoryInfo::MemorySlot::capacity, "memory_slot",
                  "Get memory slot capacity");
    component.def("memory_slot_clock_speed", &MemoryInfo::MemorySlot::clockSpeed, "memory_slot",
                  "Get memory slot clock speed");


    // ------------------------------------------------------------
    // Disk
    // ------------------------------------------------------------

    component.def("disk_usage", &getDiskUsage, "disk",
                  "Get current disk usage percentage");
    component.def("is_hotspot_connected", &isHotspotConnected, "wifi",
                  "Check if the hotspot is connected");
    component.def("wired_network", &getCurrentWiredNetwork, "wifi",
                  "Get current wired network");
    component.def("wifi_name", &getCurrentWifi, "wifi",
                  "Get current wifi name");
    component.def("current_ip", &getHostIPs, "network",
                  "Get current IP address");
    component.def("gpu_info", &getGPUInfo, "gpu", "Get GPU info");

    DLOG_F(INFO, "Loaded module {}", component.getName());
});
