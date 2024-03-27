/*
 * Config.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Config DTO

**************************************************/

#ifndef LITHIUM_CONFIG_DTO_HPP
#define LITHIUM_CONFIG_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "oatpp/core/data/stream/BufferStream.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * TLS config.
 */
class TLSConfigDto : public oatpp::DTO {
    DTO_INIT(TLSConfigDto, DTO)

    /**
     * Path to private key file.
     */
    DTO_FIELD(String, pkFile);

    /**
     * Path to full chain file.
     */
    DTO_FIELD(String, chainFile);
};

/**
 * Config where to serve controller's endpoints.
 */
class ServerConfigDto : public oatpp::DTO {
    DTO_INIT(ServerConfigDto, DTO)

    /**
     * Host
     */
    DTO_FIELD(String, host);

    /**
     * Port
     */
    DTO_FIELD(UInt16, port);

    /**
     * TLS config. If null - do not use TLS.
     */
    DTO_FIELD(Object<TLSConfigDto>, tls);
};

class ConfigDto : public oatpp::DTO {
public:
    DTO_INIT(ConfigDto, DTO)

    /**
     * Config for Host API Server (create hub functionality).
     */
    DTO_FIELD(Object<ServerConfigDto>, hostAPIServer);

    /**
     * Config for Client API Server (join hub functionality).
     */
    DTO_FIELD(Object<ServerConfigDto>, clientAPIServer);

    /**
     * Path to hubs config file.
     */
    DTO_FIELD(String, hubsConfigFile);
};

#include OATPP_CODEGEN_END(DTO)

#endif  // LITHIUM_CONFIG_DTO_HPP
