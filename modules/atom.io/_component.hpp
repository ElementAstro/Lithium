/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-IO

**************************************************/

#ifndef ATOM_IO_COMPONENT_HPP
#define ATOM_IO_COMPONENT_HPP

#include "atom/components/component.hpp"

class IOComponent : public Component {
public:
    explicit IOComponent(const std::string &name);
    ~IOComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;
};

#endif
