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

#define CONFIG_MANAGER_CHECK                                             \
    if (!m_configManager) {                                              \
        LOG_F(ERROR, "ConfigComponent::{}: configManager is nullptr",    \
              __func__);                                                 \
        return createErrorResponse(__func__, {"error", "key not found"}, \
                                   "key not found");                     \
    }

#define LITHIUM_CONFIG_KEY_CHECK(key)                                        \
    if (!m_params.contains(key) || !m_params[key].is_string()) {             \
        LOG_F(ERROR, "ConfigComponent::{}: {} not found", __func__, key);    \
        return createErrorResponse(__func__,                                 \
                                   {"error", std::string("missing ") + key}, \
                                   std::string("missing ") + key);           \
    }

ConfigComponent::ConfigComponent(const std::string& name)
    : SharedComponent(name),
      m_configManager(Lithium::ConfigManager::createUnique()) {
    DLOG_F(INFO, "ConfigComponent::Constructor");

    DLOG_F(INFO, "Injecting commands");
    registerFunc("getConfig", &ConfigComponent::getConfig, this);
    registerFunc("setConfig", &ConfigComponent::setConfig, this);
    registerFunc("deleteConfig", &ConfigComponent::deleteConfig, this);
    registerFunc("loadConfig", &ConfigComponent::loadConfig, this);
    registerFunc("saveConfig", &ConfigComponent::saveConfig, this);
    registerFunc("hasConfig", &ConfigComponent::hasConfig, this);
    registerFunc("loadConfigs", &ConfigComponent::loadConfigs, this);
}

ConfigComponent::~ConfigComponent() {
    DLOG_F(INFO, "ConfigComponent::Destructor");
}

bool ConfigComponent::initialize() {
    DLOG_F(INFO, "ConfigComponent::initialize");
    return true;
}

bool ConfigComponent::destroy() {
    DLOG_F(INFO, "ConfigComponent::destroy");
    return true;
}

json ConfigComponent::getConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    LITHIUM_CONFIG_KEY_CHECK("key");
    std::string key = m_params["key"].get<std::string>();
    if (!m_configManager->hasValue(key)) {
        LOG_F(ERROR, "ConfigComponent::getConfig: key not found");
        return createErrorResponse("getConfig", {"error", "key not found"},
                                   "key not found");
    }
    if (auto value = m_configManager->getValue(key); value.has_value()) {
        return createSuccessResponse("getConfig", {"value", value.value()});
    }
    return createErrorResponse(
        "getConfig", {"error", {"error", "failed to get config by key"}},
        "failed to get config by key");
}

json ConfigComponent::setConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    if (!m_params.contains("key") || !m_params.contains("value")) {
        LOG_F(ERROR,
              "ConfigComponent::setConfig: m_params does not contain key");
        return createErrorResponse("setConfig", {"error", "key not set"},
                                   "key not set");
    }
    if (!m_params["key"].is_string() || !m_params["value"].is_primitive()) {
        LOG_F(ERROR,
              "ConfigComponent::setConfig: m_params is not a string or "
              "primitive");
        return createErrorResponse("setConfig", {"error", "key not set"},
                                   "key not set");
    }
    if (!setVariable(m_params["key"].get<std::string>(), m_params["value"])) {
        LOG_F(ERROR, "ConfigComponent::setConfig: setVariable failed");
        return createErrorResponse(
            "setConfig", {"error", {"error", "key not set"}}, "key not set");
    }
    return createSuccessResponse("setConfig", {});
}

json ConfigComponent::deleteConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    LITHIUM_CONFIG_KEY_CHECK("key");
    try {
        if (!m_configManager->deleteValue(m_params["key"].get<std::string>())) {
            LOG_F(ERROR, "ConfigComponent::deleteConfig: deleteValue failed");
            return createErrorResponse(__func__, {"error", "key not set"},
                                       "key not set");
        }
    } catch (const json::exception& e) {
        LOG_F(ERROR, "ConfigComponent::deleteConfig: json exception: {}",
              e.what());
        return createErrorResponse(__func__, {"error", e.what()},
                                   "key not set");
    }
    return createSuccessResponse(__func__, {});
}

json ConfigComponent::hasConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    LITHIUM_CONFIG_KEY_CHECK("key");
    try {
        if (!m_configManager->hasValue(m_params["key"].get<std::string>())) {
            LOG_F(ERROR, "ConfigComponent::hasConfig: hasValue failed");
            return createErrorResponse(__func__, {"error", "key not set"},
                                       "key not set");
        }
    } catch (const json::exception& e) {
        LOG_F(ERROR, "ConfigComponent::hasConfig: json exception: {}",
              e.what());
        return createErrorResponse(__func__, {"error", e.what()},
                                   "key not set");
    }
    return createSuccessResponse(__func__, {});
}

json ConfigComponent::loadConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    if (!m_params.contains("path") || !m_params["path"].is_string()) {
        LOG_F(ERROR, "ConfigComponent::loadConfig: path not set");
        return createErrorResponse("loadConfig", {"error", "path not set"},
                                   "path not set");
    }
    if (!m_configManager->loadFromFile(m_params["path"].get<std::string>())) {
        LOG_F(ERROR, "ConfigComponent::loadConfig: loadFromFile failed");
        return createErrorResponse("loadConfig", {"error", "path not set"},
                                   "path not set");
    }
    return createSuccessResponse("loadConfig", {});
}

json ConfigComponent::saveConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    if (!m_params.contains("path") || !m_params["path"].is_string()) {
        LOG_F(ERROR, "ConfigComponent::saveConfig: path not set");
        return createErrorResponse("saveConfig", {"error", "path not set"},
                                   "path not set");
    }
    if (!m_configManager->saveToFile(m_params["path"].get<std::string>())) {
        LOG_F(ERROR, "ConfigComponent::saveConfig: saveToFile failed");
        return createErrorResponse(__func__, {"error", "path not set"},
                                   "path not set");
    }
    return createSuccessResponse(__func__, {});
}

json ConfigComponent::loadConfigs(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    if (!m_params.contains("path") || !m_params["path"].is_string()) {
        LOG_F(ERROR, "ConfigComponent::loadConfigs: path not set");
        return createErrorResponse("loadConfigs", {"error", "path not set"},
                                   "path not set");
    }
    if (!m_configManager->loadFromFile(m_params["path"].get<std::string>())) {
        LOG_F(ERROR, "ConfigComponent::loadConfigs: loadFromFile failed");
        return createErrorResponse(__func__, {"error", "path not set"},
                                   "path not set");
    }
    return createSuccessResponse(__func__, {});
}

json ConfigComponent::tidyConfig(const json& m_params) {
    CONFIG_MANAGER_CHECK;
    m_configManager->tidyConfig();
    return createSuccessResponse(__func__, {});
}