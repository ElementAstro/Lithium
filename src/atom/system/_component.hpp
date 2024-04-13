/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#ifndef ATOM_SYSTEM_COMPONENT_HPP
#define ATOM_SYSTEM_COMPONENT_HPP

#include "atom/components/templates/shared_component.hpp"

class SystemComponent : public SharedComponent
{
public:
    explicit SystemComponent(const std::string &name);
    ~SystemComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;

    // -------------------------------------------------------------------
    // System methods
    // -------------------------------------------------------------------

    json getCPUInfo(const json &m_params);
    json getMemoryInfo(const json &m_params);
    json getDiskInfo(const json &m_params);
    json getNetworkInfo(const json &m_params);
    json getGPUInfo(const json &m_params);
    json getBatteryInfo(const json &m_params);
    json getOSInfo(const json &m_params);

};

#endif