/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-16

Description: Simple wrapper around INDI server with atom component API
compatibility

**************************************************/

#include "_component.hpp"
#include "indiserver.hpp"

#include "atom/log/loguru.hpp"

INDIServerComponent::INDIServerComponent(const std::string& name)
    : Component(name), m_manager(std::make_shared<INDIManager>()) {
    LOG_F(INFO, "INDIServerComponent Constructed");

    def("start", &INDIManager::startServer, m_manager, "indi",
        "start indiserver");
    def("stop", &INDIManager::stopServer, m_manager, "indi", "stop indiserver");
}

INDIServerComponent::~INDIServerComponent() {
    LOG_F(INFO, "INDIServerComponent Destructed");
}

bool INDIServerComponent::initialize() {
    LOG_F(INFO, "INDIServerComponent Initialized");
    return true;
}

bool INDIServerComponent::destroy() {
    LOG_F(INFO, "INDIServerComponent Destroyed");
    return true;
}
