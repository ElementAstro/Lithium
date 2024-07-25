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
#include <memory>
#include "iconnector.hpp"
#include "indiserver.hpp"

#include "atom/log/loguru.hpp"

INDIServerComponent::INDIServerComponent(const std::string& name)
    : Component(name), m_manager(std::make_shared<INDIManager>(std::make_unique<INDIConnector>())) {
    LOG_F(INFO, "INDIServerComponent Constructed");

    def("start", &INDIManager::startServer, m_manager, "astro",
        "start indiserver");
    def("stop", &INDIManager::stopServer, m_manager, "astro", "stop indiserver");
    def("is_running", &INDIManager::isRunning, m_manager, "astro",
        "check if indiserver is running");
    def("is_installed", &INDIManager::isInstalled, m_manager, "astro",
        "check if indiserver is installed");
    def("set_prop", &INDIManager::setProp, m_manager, "astro", "set prop");
    def("get_prop", &INDIManager::getProp, m_manager, "astro", "get prop");
    def("get_state", &INDIManager::getState, m_manager, "astro", "get state");
    def("get_available_device", &INDIManager::getRunningDrivers, m_manager, "astro",
        "get available device");

    addVariable("indi.manager", m_manager, "indi manager");
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
