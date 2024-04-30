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

#include "configor.hpp"

#include "atom/log/loguru.hpp"

ConfigComponent::ConfigComponent(const std::string& name)
    : Component(name), m_configManager(ConfigManager::createShared()) {
    LOG_F(INFO, "Config Component Constructed");
    registerCommand("getConfig", &ConfigManager::getValue, m_configManager);
    registerCommand("setConfig", &ConfigManager::setValue, m_configManager);
    registerCommand("hasConfig", &ConfigManager::hasValue, m_configManager);
    registerCommand("deleteConfig", &ConfigManager::deleteValue,
                    m_configManager);
    registerCommand("loadConfig", &ConfigManager::loadFromFile,
                    m_configManager);
    registerCommand("loadConfigs", &ConfigManager::loadFromDir,
                    m_configManager);
    registerCommand("saveConfig", &ConfigManager::saveToFile, m_configManager);
    registerCommand("tidyConfig", &ConfigManager::tidyConfig, m_configManager);
    registerCommand("clearConfig", &ConfigManager::clearConfig,
                    m_configManager);

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