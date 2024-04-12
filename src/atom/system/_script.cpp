/*
 * _script.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Carbon Binding of Atom-System

**************************************************/

#include "carbon/carbon.hpp"

#include "module/battery.hpp"
#include "module/cpu.hpp"
#include "module/disk.hpp"
#include "module/memory.hpp"
#include "module/os.hpp"
#include "module/wifi.hpp"

#include <any>
#include "atom/log/loguru.hpp"

using namespace Atom::System;

CARBON_MODULE_EXPORT {
    Carbon::ModulePtr exportModule([[maybe_unused]] const std::any &m_params) {
        Carbon::ModulePtr m_module = std::make_shared<Carbon::Module>();

        m_module->add(Carbon::fun(&getCurrentCpuUsage),
                      "get_current_cpu_usage");
        m_module->add(Carbon::fun(&getCurrentCpuTemperature),
                      "get_current_cpu_temperature");
        m_module->add(Carbon::fun(&getCPUModel), "get_cpu_model");
        m_module->add(Carbon::fun(&getProcessorIdentifier),
                      "get_processor_identifier");
        m_module->add(Carbon::fun(&getProcessorFrequency),
                      "get_processor_frequency");
        m_module->add(Carbon::fun(&getNumberOfPhysicalPackages),
                      "get_number_of_physical_packages");
        m_module->add(Carbon::fun(&getNumberOfPhysicalCPUs),
                      "get_number_of_physical_cpus");

        m_module->add(Carbon::fun(&getMemoryUsage), "get_memory_usage");
        m_module->add(Carbon::fun(&getTotalMemorySize),
                      "get_total_memory_size");
        m_module->add(Carbon::fun(&getAvailableMemorySize),
                      "get_available_memory_size");
        m_module->add(Carbon::fun(&getPhysicalMemoryInfo),
                      "get_physical_memory_info");
        m_module->add(Carbon::fun(&getVirtualMemoryMax),
                      "get_virtual_memory_max");
        m_module->add(Carbon::fun(&getVirtualMemoryUsed),
                      "get_virtual_memory_used");
        m_module->add(Carbon::fun(&getSwapMemoryTotal),
                      "get_swap_memory_total");
        m_module->add(Carbon::fun(&getSwapMemoryUsed), "get_swap_memory_used");
        return m_module;
    }
}
