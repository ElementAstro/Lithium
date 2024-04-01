#ifndef LITHIUM_CONFIG_COMPONENT_HPP
#define LITHIUM_CONFIG_COMPONENT_HPP

#include "atom/components/templates/shared_component.hpp"

namespace Lithium
{
    class ConfigManager;
}

class ConfigComponent : public SharedComponent
{
public:
    explicit ConfigComponent(const std::string &name);
    ~ConfigComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool initialize() override;
    bool destroy() override;

    // -------------------------------------------------------------------
    // Config methods
    // -------------------------------------------------------------------

    json getConfig(const json &m_params);
    json setConfig(const json &m_params);
    json deleteConfig(const json &m_params);
    json hasConfig(const json &m_params);

    json loadConfig(const json &m_params);
    json loadConfigs(const json &m_params);
    json saveConfig(const json &m_params);

private:
    std::unique_ptr<Lithium::ConfigManager> m_configManager;
};

#endif
