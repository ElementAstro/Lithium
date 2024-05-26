/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-18

Description: Lithium Web Interface based on oatpp

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

ServerComponent::ServerComponent(const std::string& name)
    : Component(name) {
    LOG_F(INFO, "ServerComponent Constructed");
}

ServerComponent::~ServerComponent() {
    LOG_F(INFO, "ServerComponent Destructed");
}

bool ServerComponent::initialize() {
    LOG_F(INFO, "ServerComponent Initialized");
    return true;
}

bool ServerComponent::destroy() {
    LOG_F(INFO, "ServerComponent Destroyed");
    return true;
}
