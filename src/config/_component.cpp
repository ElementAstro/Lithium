#include "_component.hpp"

#include "configor.hpp"

#include "atom/log/loguru.hpp"

ConfigComponent::ConfigComponent(const std::string& name)
    : SharedComponent(name), m_configManager(Lithium::ConfigManager::createShared()) {}

ConfigComponent::~ConfigComponent() {}

bool ConfigComponent::initialize() { return true; }

bool ConfigComponent::destroy() { return true; }

json ConfigComponent::getConfig(const json& m_params) {
    if (!m_configManager) {
        LOG_F(ERROR, "ConfigComponent::getConfig m_configManager is nullptr");
        return createErrorResponse("getConfig", {"error", "key not found"},
                                   "key not found");
    }
    if (!m_params.contains("key") || !m_params["key"].is_string()) {
        LOG_F(ERROR, "ConfigComponent::getConfig: key not found");
        return createErrorResponse("getConfig", {"error", "missing key"},
                                   "missing key");
    }
    std::string key = m_params["key"].get<std::string>();
    if (!m_configManager->hasValue(key))
    {
        LOG_F(ERROR, "ConfigComponent::getConfig: key not found");
        return createErrorResponse("getConfig", {"error", "key not found"},
                                   "key not found");
    }
    if (auto value = m_configManager->getValue(key); value)
    {
        return createSuccessResponse("getConfig", {"value",value});
    }
    return createErrorResponse("getConfig", {"error", {"error", "failed to get config by key"}},
                               "failed to get config by key");
}

json ConfigComponent::setConfig(const json &m_params)
{
    if (!m_configManager)
    {
        LOG_F(ERROR,"ConfigComponent::setConfig: m_configManager is nullptr");
        return createErrorResponse("setConfig", {"error", {"error", "config manager not set"}},
                                   "config manager not set");
    }
    if (!m_params.contains("key") || !m_params.contains("value"))
    {
        LOG_F(ERROR,"ConfigComponent::setConfig: m_params does not contain key");
        return createErrorResponse("setConfig", {"error", {"error", "key not set"}},
                                   "key not set");
    }
    if (!m_params["key"].is_string() || !m_params["value"].is_primitive())
    {
        LOG_F(ERROR,"ConfigComponent::setConfig: m_params is not a string or primitive");
        return createErrorResponse("setConfig", {"error", {"error", "key not set"}},
                                   "key not set");
    }
    if (!setVariable(m_params["key"].get<std::string>(), m_params["value"]))
    {
        LOG_F(ERROR,"ConfigComponent::setConfig: setVariable failed");
        return createErrorResponse("setConfig", {"error", {"error", "key not set"}},
                                   "key not set");
    }
    return createSuccessResponse("setConfig", {});
}