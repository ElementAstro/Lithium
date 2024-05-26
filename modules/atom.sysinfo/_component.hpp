
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-18

Description: A collector for system information, not the same as atom.system

**************************************************/

#ifndef ATOM_SYSINFO_COMPONENT_HPP
#define ATOM_SYSINFO_COMPONENT_HPP

#include "atom/components/component.hpp"

class SysInfoComponent : public Component {
public:
    explicit SysInfoComponent(const std::string& name);
    virtual ~SysInfoComponent();

    bool initialize() override;
    bool destroy() override;

protected:
    double getCurrentBatteryLevel();
    bool isBatteryCharging();

    std::string getOSName();
    std::string getOSVersion();
    std::string getKernelVersion();
    std::string getArchitecture();
};
#endif
