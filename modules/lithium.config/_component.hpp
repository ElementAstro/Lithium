/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Config Component for Atom Addon

**************************************************/

#ifndef LITHIUM_CONFIG_COMPONENT_HPP
#define LITHIUM_CONFIG_COMPONENT_HPP

#include "atom/components/component.hpp"

namespace lithium {
class ConfigManager;
}

class ConfigComponent : public Component {
public:
    explicit ConfigComponent(const std::string& name);
    virtual ~ConfigComponent();

    bool initialize() override;
    bool destroy() override;

private:
    std::shared_ptr<lithium::ConfigManager> m_configManager;
};

#endif
