/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Lithium Image Component for Atom Addon

**************************************************/

#include "_component.hpp"

#include "fitsio.hpp"

#include "atom/log/loguru.hpp"

ImageComponent::ImageComponent(const std::string& name) : Component(name) {
    LOG_F(INFO, "Lithium Image Component Constructed");
}

ImageComponent::~ImageComponent() {
    LOG_F(INFO, "Lithium Image Component Destructed");
}

bool ImageComponent::initialize() {
    LOG_F(INFO, "Lithium Image Component Initialized");
    return true;
}

bool ImageComponent::destroy() {
    LOG_F(INFO, "Lithium Image Component Destroyed");
    return true;
}
