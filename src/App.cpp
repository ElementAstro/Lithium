/*
 * App.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Main

**************************************************/

#include "AppComponent.hpp"

#ifdef ENABLE_ASYNC
#include "controller/AsyncConfigController.hpp"
#include "controller/AsyncStaticController.hpp"
#include "controller/AsyncSystemController.hpp"
#include "controller/AsyncWebSocketController.hpp"
#include "controller/AsyncIOController.hpp"
#include "controller/AsyncProcessController.hpp"
#include "controller/AsyncUploadController.hpp"
#include "controller/AsyncDeviceController.hpp"
#else

#endif

#if ENABLE_ASYNC
#include "oatpp-swagger/AsyncController.hpp"
#else
#include "oatpp-swagger/Controller.hpp"
#endif

#include "oatpp/network/Server.hpp"

#include <argparse/argparse.hpp>

#include "LithiumApp.hpp"

#include "atom/server/global_ptr.hpp"
#include "atom/system/crash.hpp"
#include "atom/web/utils.hpp"
#include "atom/log/loguru.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

#define ADD_CONTROLLER(controller, docEndpoints, router, logMessage) \
    auto controller##_ptr = controller::createShared();              \
    docEndpoints.append(controller##_ptr->getEndpoints());           \
    router->addController(controller##_ptr);                         \
    DLOG_F(INFO, logMessage " loaded");

void runServer()
{
    DLOG_F(INFO, "Loading App component ...");
#if ENABLE_IPV6
    AppComponent components(Lithium::MyApp->GetConfig("config/server").value("host", "::"), Lithium::MyApp->GetConfig("config/server").value("port", 8000)); // Create scope Environment components
#else
    AppComponent components(Lithium::MyApp->GetConfig("config/server").value("host", "0.0.0.0"), Lithium::MyApp->GetConfig("config/server").value("port", 8000)); // Create scope Environment components
#endif
    DLOG_F(INFO, "App component loaded");

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
    oatpp::web::server::api::Endpoints docEndpoints;
    /* Add routes & documents */

    ADD_CONTROLLER(ConfigController, docEndpoints, router, "AsyncConfigController");

    DLOG_F(INFO, "Starting to load API doc controller");
#if ENABLE_ASYNC
    router->addController(oatpp::swagger::AsyncController::createShared(docEndpoints));
#else
    router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
#endif
    DLOG_F(INFO, "API doc controller loaded");

    /* Load websocket route */
    router->addController(WebSocketController::createShared());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    DLOG_F(INFO, "Loaded server components ... Prepare for starting ...");
    /* create server */
    oatpp::network::Server server(connectionProvider,
                                  connectionHandler);

    DLOG_F(INFO, "Server running on port {}...", connectionProvider->getProperty("port").toString()->c_str());

    /* This is a block function that will be called when the server is started */
    server.run();
}

/**
 * @brief setup log file
 * @note This is called in main function
 */
void setupLogFile()
{
    std::filesystem::path logsFolder = std::filesystem::current_path() / "logs";
    if (!std::filesystem::exists(logsFolder))
    {
        std::filesystem::create_directory(logsFolder);
    }
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time_t);
    char filename[100];
    std::strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.log", local_time);
    std::filesystem::path logFilePath = logsFolder / filename;
    loguru::add_file(logFilePath.string().c_str(), loguru::Append, loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message)
                              { 
        Lithium::CrashReport::saveCrashLog(std::string(message.prefix) + message.message); 
        oatpp::base::Environment::destroy(); });
}

/**
 * @brief main function
 * @param argc number of arguments
 * @param argv arguments
 * @return 0 on success
 */
int main(int argc, char *argv[])
{
    /* Add gettext */
#if ENABLE_GETTEXT
    bindtextdomain("lithium", "locale");
    /* Only write the following 2 lines if creating an executable */
    setlocale(LC_ALL, "");
    textdomain("lithium");
#endif

    // Init loguru log system
    loguru::init(argc, argv);
    // Set log file
    setupLogFile();

    /* Parse arguments */
    argparse::ArgumentParser program("Lithium Server");

    program.add_argument("-P", "--port").help(_("port the server running on")).default_value(8000);
    program.add_argument("-H", "--host").help(_("host the server running on")).default_value("0.0.0.0");
    program.add_argument("-C", "--config").help(_("path to the config file")).default_value("cpnfig.json");
    program.add_argument("-M", "--module-path").help(_("path to the modules directory")).default_value("modules");
    program.add_argument("-W", "--web-panel").help(_("web panel")).default_value(true);
    program.add_argument("-L", "--log-file").help(_("path to log file"));

    program.add_description(_("Lithium Command Line Interface:"));
    program.add_epilog(_("End."));

    program.parse_args(argc, argv);

    Lithium::InitLithiumApp();
    // Run oatpp server
    Lithium::MyApp = Lithium::LithiumApp::createShared();
    Lithium::MyApp->initMyAppChai();

    auto cmd_port = program.get<int>("--port");
    if (cmd_port != 8000)
    {
        DLOG_F(INFO, _("Command line server port : %d"), cmd_port);

        auto port = Lithium::MyApp->GetConfig("config/server").value<int>("port", 8000);
        if (port != cmd_port)
        {
            Lithium::MyApp->SetConfig("config/server/port", cmd_port);
            DLOG_F(INFO, _("Set server port to %d"), cmd_port);
        }
    }
    try
    {
        auto cmd_host = program.get<std::string>("--host");
        auto cmd_config_path = program.get<std::string>("--config");
        auto cmd_module_path = program.get<std::string>("--module-path");
        auto cmd_web_panel = program.get<bool>("--web-panel");

        if (!cmd_host.empty())
        {
            Lithium::MyApp->SetConfig("config/server/host", cmd_host);
        }
        if (!cmd_config_path.empty())
        {
            Lithium::MyApp->SetConfig("config/server/configpath", cmd_config_path);
        }
        if (!cmd_module_path.empty())
        {
            Lithium::MyApp->SetConfig("config/server/modulepath", cmd_module_path);
        }

        if (!cmd_web_panel)
        {
            if (Lithium::MyApp->GetConfig("config/server/web").get<bool>())
            {
                Lithium::MyApp->SetConfig("config/server/web", false);
            }
        }
    }
    catch (const std::bad_any_cast &e)
    {
        LOG_F(ERROR, "Invalid args format! Error: {}", e.what());
    }

    oatpp::base::Environment::init();
    // Run the main server
    runServer();
    // Clean up all
    oatpp::base::Environment::destroy();

    return 0;
}