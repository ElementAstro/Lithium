/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Config Component for Atom Addon

**************************************************/

#include "_component.hpp"

#include "config/configor.hpp"

#include "atom/log/loguru.hpp"

ConfigComponent::ConfigComponent(const std::string& name)
    : Component(name), m_configManager(lithium::ConfigManager::createShared()) {
    LOG_F(INFO, "Config Component Constructed");
    def("getConfig", &lithium::ConfigManager::getValue, m_configManager);
    def("setConfig", &lithium::ConfigManager::setValue, m_configManager);
    def("hasConfig", &lithium::ConfigManager::hasValue, m_configManager);
    def("deleteConfig", &lithium::ConfigManager::deleteValue, m_configManager);
    def("loadConfig", &lithium::ConfigManager::loadFromFile, m_configManager);
    def("loadConfigs", &lithium::ConfigManager::loadFromDir, m_configManager);
    def("saveConfig", &lithium::ConfigManager::saveToFile, m_configManager);
    def("tidyConfig", &lithium::ConfigManager::tidyConfig, m_configManager);
    def("clearConfig", &lithium::ConfigManager::clearConfig, m_configManager);

    addVariable("config.instance", m_configManager, "ConfigManager Instance");
}

ConfigComponent::~ConfigComponent() {
    LOG_F(INFO, "Config Component Destructed");
}

bool ConfigComponent::initialize() {
    LOG_F(INFO, "Config Component Initialized");
    return true;
}

bool ConfigComponent::destroy() {
    LOG_F(INFO, "Config Component Destroyed");
    return true;
}
