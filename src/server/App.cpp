#include "controller/FileController.hpp"
#include "controller/RoomsController.hpp"
#include "controller/StaticController.hpp"
#include "controller/StatisticsController.hpp"

#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

void run(const oatpp::base::CommandLineArguments& args) {
    /* Register Components in scope of run() method */
    AppComponent components(args);

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create RoomsController and add all of its endpoints to router */
    router->addController(std::make_shared<RoomsController>());
    router->addController(std::make_shared<StaticController>());
    router->addController(std::make_shared<FileController>());
    router->addController(std::make_shared<StatisticsController>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                    connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>,
                    connectionProvider);

    /* Create server which takes provided TCP connections and passes them to
     * HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    std::thread serverThread([&server] { server.run(); });

    std::thread pingThread([] {
        OATPP_COMPONENT(std::shared_ptr<Lobby>, lobby);
        lobby->runPingLoop(std::chrono::seconds(30));
    });

    std::thread statThread([] {
        OATPP_COMPONENT(std::shared_ptr<Statistics>, statistics);
        statistics->runStatLoop();
    });

    OATPP_COMPONENT(oatpp::Object<ConfigDto>, appConfig);

    if (appConfig->useTLS) {
        OATPP_LOGi("canchat",
                   "clients are expected to connect at https://{}:{}/",
                   appConfig->host, appConfig->port);
    } else {
        OATPP_LOGi("canchat",
                   "clients are expected to connect at http://{}:{}/",
                   appConfig->host, appConfig->port);
    }

    OATPP_LOGi("canchat", "canonical base URL={}",
               appConfig->getCanonicalBaseUrl())
        OATPP_LOGi("canchat", "statistics URL={}", appConfig->getStatsUrl())

            serverThread.join();
    pingThread.join();
    statThread.join();
}

#ifndef ENABLE_SERVER_STANDALONE
int main(int argc, const char* argv[]) {
    oatpp::Environment::init();

    run(oatpp::base::CommandLineArguments(argc, argv));

    oatpp::Environment::destroy();

    return 0;
}
#else
#include "App.hpp"
namespace lithium {
auto runServer(int argc, const char* argv[]) -> bool {
    oatpp::Environment::init();

    run(oatpp::base::CommandLineArguments(argc, argv));

    oatpp::Environment::destroy();

    return true;
}
}  // namespace lithium

#endif