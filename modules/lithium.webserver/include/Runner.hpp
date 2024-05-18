/*
 * Runner.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Lithium Server Runner

**************************************************/

#ifndef LITHIUM_RUNNER_HPP
#define LITHIUM_RUNNER_HPP

#include "config/Config.hpp"

#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/network/ConnectionProvider.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"

#include "oatpp/core/async/Executor.hpp"

class APIServer {
private:
    std::shared_ptr<oatpp::web::server::HttpRouter> m_router;
    std::shared_ptr<oatpp::network::ServerConnectionProvider>
        m_connectionProvider;
    std::shared_ptr<oatpp::web::server::AsyncHttpConnectionHandler>
        m_connectionHandler;

private:
#if __cplusplus >= 202002L
    std::jthread m_serverThread;
#else
    std::thread m_serverThread;
#endif

public:
    APIServer(const oatpp::Object<ServerConfigDto>& config,
              const std::shared_ptr<oatpp::async::Executor>& executor);

    std::shared_ptr<oatpp::web::server::HttpRouter> getRouter();

    void start();

    void join();
};

class Runner {
private:
    std::list<std::shared_ptr<APIServer>> m_servers;

private:
    void assertServerConfig(const oatpp::Object<ServerConfigDto>& config,
                            const oatpp::String& serverName, bool checkTls);

public:
    Runner(const oatpp::Object<ConfigDto>& config,
           const std::shared_ptr<oatpp::async::Executor>& executor);

    void start();

    void join();
};

#endif  // LITHIUM_RUNNER_HPP
