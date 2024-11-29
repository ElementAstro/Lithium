#include "controller/ComponentController.hpp"
#include "controller/ConfigController.hpp"
#include "controller/FileController.hpp"
#include "controller/RoomsController.hpp"
// #include "controller/ScriptController.hpp"
#include "controller/StaticController.hpp"
#include "controller/StatisticsController.hpp"

#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include "addon/loader.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"

#include "controller/INDIController.hpp"

void run(const oatpp::base::CommandLineArguments& args) {
    /* Register Components in scope of run() method */
    AppComponent components(args);

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create RoomsController and add all of its endpoints to router */
    router->addController(std::make_shared<ComponentController>());
    router->addController(ConfigController::createShared());
    router->addController(std::make_shared<RoomsController>());
    router->addController(std::make_shared<StaticController>());
    router->addController(std::make_shared<FileController>());
    // router->addController(std::make_shared<ScriptController>());
    router->addController(std::make_shared<StatisticsController>());

    router->addController(INDIController::createShared());

    // router->addController(createInstance());

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
        LOG_F(INFO, "clients are expected to connect at https://{}:{}",
              *appConfig->host, *appConfig->port);
    } else {
        LOG_F(INFO, "Canonical base URL={}", *appConfig->getCanonicalBaseUrl());
    }

    LOG_F(INFO, "Canonical base URL={}", *appConfig->getCanonicalBaseUrl());
    LOG_F(INFO, "Statistics URL={}", *appConfig->getStatsUrl());

    serverThread.join();
    pingThread.join();
    statThread.join();
}

#include "App.hpp"
namespace lithium {
auto runServer(CommandLineArgs args) -> bool {
    oatpp::Environment::init();

    run(oatpp::base::CommandLineArguments(args.argc, args.argv));

    oatpp::Environment::destroy();

    return true;
}
}  // namespace lithium
