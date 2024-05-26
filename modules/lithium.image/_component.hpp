/*
 * _component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#ifndef LITHIUM_IMAGE_COMPONENT_HPP
#define LITHIUM_IMAGE_COMPONENT_HPP

#include "atom/components/component.hpp"

class ImageComponent : public Component {
public:
    explicit ImageComponent(const std::string &name);
    ~ImageComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;
};

#endif
