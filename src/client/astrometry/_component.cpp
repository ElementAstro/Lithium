/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-07-20

Description: Component wrapper for Astap

**************************************************/

#include "_component.hpp"
#include <memory>

#include "astrometry.hpp"

#include "atom/log/loguru.hpp"
#include "function/type_info.hpp"

AstrometryComponent::AstrometryComponent(const std::string& name)
    : Component(name),
      m_solver(std::make_shared<AstrometrySolver>("astrometry")) {
    
}

AstrometryComponent::~AstrometryComponent() {
    LOG_F(INFO, "AstrometryComponent Destructed");
}

auto AstrometryComponent::initialize() -> bool {
    LOG_F(INFO, "AstrometryComponent Initialized");
    return true;
}

auto AstrometryComponent::destroy() -> bool {
    LOG_F(INFO, "AstrometryComponent Destroyed");
    return true;
}
