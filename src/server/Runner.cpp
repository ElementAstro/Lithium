/*
 * Runner.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Lithium Server Runner

**************************************************/

#include "Runner.hpp"

#include "controller/AsyncClientController.hpp"

#include "oatpp-openssl/server/ConnectionProvider.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"

#include "oatpp/network/Server.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// APIServer

APIServer::APIServer(const oatpp::Object<ServerConfigDto>& config,
                     const std::shared_ptr<oatpp::async::Executor>& executor)
    : m_router(oatpp::web::server::HttpRouter::createShared()) {
    if (config->tls) {
        OATPP_LOGD("APIServer", "key_path='%s'", config->tls->pkFile->c_str());
        OATPP_LOGD("APIServer", "chn_path='%s'",
                   config->tls->chainFile->c_str());

        auto tlcConfig =
            oatpp::openssl::Config::createDefaultServerConfigShared(
                config->tls->pkFile->c_str(), config->tls->chainFile->c_str());

        m_connectionProvider =
            oatpp::openssl::server::ConnectionProvider::createShared(
                tlcConfig,
                {config->host, config->port, oatpp::network::Address::IP_4});

    } else {
        m_connectionProvider =
            oatpp::network::tcp::server::ConnectionProvider::createShared(
                {config->host, config->port, oatpp::network::Address::IP_4});
    }

    m_connectionHandler =
        oatpp::web::server::AsyncHttpConnectionHandler::createShared(m_router,
                                                                     executor);
}

std::shared_ptr<oatpp::web::server::HttpRouter> APIServer::getRouter() {
    return m_router;
}

void APIServer::start() {
    m_serverThread = std::thread([this] {
        oatpp::network::Server server(m_connectionProvider,
                                      m_connectionHandler);
        server.run();
    });
}

void APIServer::join() { m_serverThread.join(); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Runner

Runner::Runner(const oatpp::Object<ConfigDto>& config,
               const std::shared_ptr<oatpp::async::Executor>& executor) {
    /* host API server */
    assertServerConfig(config->hostAPIServer, "hostAPIServer", true);

    auto hostServer =
        std::make_shared<APIServer>(config->hostAPIServer, executor);
    hostServer->getRouter()->addController(std::make_shared<HostController>());
    m_servers.push_back(hostServer);

    /* client API server */
    assertServerConfig(config->clientAPIServer, "clientAPIServer", false);

    if (config->clientAPIServer->host == config->hostAPIServer->host &&
        config->clientAPIServer->port == config->hostAPIServer->port) {
        hostServer->getRouter()->addController(
            std::make_shared<ClientController>());

    } else {
        assertServerConfig(config->clientAPIServer, "clientAPIServer", true);

        auto clientServer =
            std::make_shared<APIServer>(config->clientAPIServer, executor);
        clientServer->getRouter()->addController(
            std::make_shared<ClientController>());
        m_servers.push_back(clientServer);
    }
}

void Runner::assertServerConfig(const oatpp::Object<ServerConfigDto>& config,
                                const oatpp::String& serverName,
                                bool checkTls) {
    if (!config) {
        OATPP_LOGE("Runner", "Error: Missing config value - '%s'",
                   serverName->c_str())
        throw std::runtime_error("Error: Missing config value - '" +
                                 serverName + "'");
    }

    if (!config->host) {
        OATPP_LOGE("Runner", "Error: Missing config value - '%s.host'",
                   serverName->c_str())
        throw std::runtime_error("Error: Missing config value - '" +
                                 serverName + ".host'");
    }

    if (!config->port) {
        OATPP_LOGE("Runner", "Error: Missing config value - '$s.port'",
                   serverName->c_str())
        throw std::runtime_error("Error: Missing config value - '" +
                                 serverName + ".port'");
    }

    if (config->tls && checkTls) {
        if (!config->tls->pkFile) {
            OATPP_LOGE("Runner",
                       "Error: Missing config value - '$s.tls.pkFile'",
                       serverName->c_str())
            throw std::runtime_error("Error: Missing config value - '" +
                                     serverName + ".tls.pkFile'");
        }

        if (!config->tls->chainFile) {
            OATPP_LOGE("Runner",
                       "Error: Missing config value - '$s.tls.chainFile'",
                       serverName->c_str())
            throw std::runtime_error("Error: Missing config value - '" +
                                     serverName + ".tls.chainFile'");
        }
    }
}

void Runner::start() {
    for (auto& server : m_servers) {
        server->start();
    }
}

void Runner::join() {
    for (auto& server : m_servers) {
        server->join();
    }
}