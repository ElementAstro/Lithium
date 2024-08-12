
/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-26

Description: Some useful tools written in c++

**************************************************/

#ifndef LITHIUM_CXXTOOLS_COMPONENT_HPP
#define LITHIUM_CXXTOOLS_COMPONENT_HPP

#include "atom/components/component.hpp"

class ToolsComponent : public Component {
public:
    explicit ToolsComponent(const std::string& name);
    ~ToolsComponent() override;

    auto initialize() -> bool override;
    auto destroy() -> bool override;
};
#endif
