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

#include "astap.hpp"

#include "atom/log/loguru.hpp"
#include "function/type_info.hpp"

AstapComponent::AstapComponent(const std::string& name)
    : Component(name), m_solver(std::make_shared<AstapSolver>("astap")) {
    LOG_F(INFO, "AstapComponent Constructed");


}

AstapComponent::~AstapComponent() { LOG_F(INFO, "AstapComponent Destructed"); }

auto AstapComponent::initialize() -> bool {
    LOG_F(INFO, "AstapComponent Initialized");
    return true;
}

auto AstapComponent::destroy() -> bool {
    LOG_F(INFO, "AstapComponent Destroyed");
    return true;
}
