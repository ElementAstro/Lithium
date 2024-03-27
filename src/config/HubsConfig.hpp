/*
 * HubConfig.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Config DTO

**************************************************/

#ifndef LITHIUM_CONFIG_HUBSCONFIG_HPP
#define LITHIUM_CONFIG_HUBSCONFIG_HPP

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include <mutex>

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * Hub config
 */
class HubConfigDto : public oatpp::DTO {

  DTO_INIT(HubConfigDto, DTO)

  /**
   * Hub ID.
   */
  DTO_FIELD(String, hubId);

  /**
   * Host peer can't change.
   * If host peer disconnects the hub is over and all other peers are dropped.
   */
  DTO_FIELD(Boolean, staticHost) = true;

  /**
   * The maximum number of peers connected to hub (including host peer).
   */
  DTO_FIELD(UInt32, maxPeers) = 10;

  /**
   * Max size of the received bytes. (the whole MessageDto structure).
   */
  DTO_FIELD(UInt64, maxMessageSizeBytes) = 4 * 1024; // Default - 4Kb

  /**
   * Max number of messages queued for the peer.
   * If exceeded messages are dropped.
   */
  DTO_FIELD(UInt32, maxQueuedMessages) = 100;

  /**
   * How often should server ping client.
   */
  DTO_FIELD(UInt64, pingIntervalMillis) = 5 * 1000; // 5 seconds

  /**
   * A failed ping is ping to which server receives no response in a 'pingIntervalMillis' interval.
   * If number of failed pings for a peer reaches 'maxFailedPings' in a row then peer is dropped.
   */
  DTO_FIELD(UInt64, maxFailedPings) = 100;

};

class HubsConfig {
private:
  oatpp::String m_configFile;
  oatpp::parser::json::mapping::ObjectMapper m_mapper;
  oatpp::UnorderedFields<oatpp::Object<HubConfigDto>> m_hubs;
  std::mutex m_mutex;
public:

  /**
   * Path to config file containing configs for hubs.
   * @param configFilename
   */
  HubsConfig(const oatpp::String& configFilename);

  /**
   * Put hub config
   * @param hubId
   * @param config
   */
  void putHubConfig(const oatpp::Object<HubConfigDto>& config);

  /**
   * Get hub config
   * @param hubId
   * @return
   */
  oatpp::Object<HubConfigDto> getHubConfig(const oatpp::String& hubId);

  /**
   * Save current state of hubs config to config file.
   */
  bool save();

};

#include OATPP_CODEGEN_END(DTO)

#endif //LITHIUM_CONFIG_HUBSCONFIG_HPP
