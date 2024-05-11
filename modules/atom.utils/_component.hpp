/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#ifndef ATOM_UTILS_COMPONENT_HPP
#define ATOM_UTILS_COMPONENT_HPP

#include "atom/components/component.hpp"

class UtilsComponent : public Component {
public:
    explicit UtilsComponent(const std::string &name);
    ~UtilsComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;
};

#endif
