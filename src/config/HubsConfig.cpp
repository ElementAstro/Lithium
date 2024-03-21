/*
 * HubConfig.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Config DTO

**************************************************/

#include "HubsConfig.hpp"

HubsConfig::HubsConfig(const oatpp::String& configFilename)
  : m_configFile(configFilename)
{
  if(configFilename) {
    auto json = oatpp::String::loadFromFile(configFilename->c_str());
    m_hubs = m_mapper.readFromString<oatpp::UnorderedFields<oatpp::Object<HubConfigDto>>>(json);
  }
}

void HubsConfig::putHubConfig(const oatpp::Object<HubConfigDto>& config) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if(m_hubs == nullptr) {
    m_hubs = oatpp::UnorderedFields<oatpp::Object<HubConfigDto>>({});
  }
  m_hubs->insert({config->hubId, config});
}

oatpp::Object<HubConfigDto> HubsConfig::getHubConfig(const oatpp::String& hubId) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if(m_hubs) {
    auto it = m_hubs->find(hubId);
    if(it != m_hubs->end()) {
      return it->second;
    }
  }
  return nullptr;
}

bool HubsConfig::save() {
  oatpp::String json;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    json = m_mapper.writeToString(m_hubs);
  }
  if(m_configFile) {
    json.saveToFile(m_configFile->c_str());
    return true;
  }
  return false;
}
