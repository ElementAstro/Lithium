/*
 * App.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Main

**************************************************/

#include "AppComponent.hpp"

#ifdef ENABLE_ASYNC
#include "controller/AsyncStaticController.hpp"
#include "controller/AsyncSystemController.hpp"
#include "controller/AsyncWebSocketController.hpp"
#include "controller/AsyncIOController.hpp"
#include "controller/AsyncProcessController.hpp"
#include "controller/AsyncPHD2Controller.hpp"
#include "controller/AsyncTaskController.hpp"
#include "controller/AsyncUploadController.hpp"
#else
#include "controller/StaticController.hpp"
#include "controller/SystemController.hpp"
#include "controller/WebSocketController.hpp"
#include "controller/IOController.hpp"
#include "controller/ProcessController.hpp"
#include "controller/PHD2Controller.hpp"
#include "controller/TaskController.hpp"
#include "controller/UploadController.hpp"
#endif

#if ENABLE_ASYNC
#include "oatpp-swagger/AsyncController.hpp"
#else
#include "oatpp-swagger/Controller.hpp"
#endif

#include "oatpp/network/Server.hpp"

#include <argparse/argparse.hpp>

#include "LithiumApp.hpp"

#include "atom/system/crash.hpp"
#include "atom/web/utils.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

#include "loguru/loguru.hpp"

void run()
{
    DLOG_F(INFO, "Loading App component ...");

    std::string host = Lithium::MyApp->GetConfig("config/server").value("host", "0.0.0.0");
    DLOG_F(INFO, "Host: {}", host);
    int port = Lithium::MyApp->GetConfig("config/server").value("port", 8000);
    DLOG_F(INFO, "Port: {}", port);

#if ENABLE_IPV6
    AppComponent components(Lithium::MyApp->GetConfig("config/server").value("host", "::"), Lithium::MyApp->GetConfig("config/server").value("port", 8000)); // Create scope Environment components
#else
    AppComponent components(host, port); // Create scope Environment components
#endif

    DLOG_F(INFO, "App component loaded");
    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    oatpp::web::server::api::Endpoints docEndpoints;

    DLOG_F(INFO, "Document endpoints loaded");

    /* Add document */

    auto static_controller = StaticController::createShared();
    docEndpoints.append(static_controller->getEndpoints());
    router->addController(static_controller);
    DLOG_F(INFO, "Static file controller loaded");

    auto system_controller = SystemController::createShared();
    docEndpoints.append(system_controller->getEndpoints());
    router->addController(system_controller);
    DLOG_F(INFO, "System controller loaded");

    auto io_controller = IOController::createShared();
    docEndpoints.append(io_controller->getEndpoints());
    router->addController(io_controller);
    DLOG_F(INFO, "IO controller loaded");

    auto process_controller = ProcessController::createShared();
    docEndpoints.append(process_controller->getEndpoints());
    router->addController(process_controller);
    DLOG_F(INFO, "System process controller loaded");

    auto phd2_controller = PHD2Controller::createShared();
    docEndpoints.append(phd2_controller->getEndpoints());
    router->addController(phd2_controller);
    DLOG_F(INFO, "PHD2 controller loaded");

    auto task_controller = TaskController::createShared();
    docEndpoints.append(task_controller->getEndpoints());
    router->addController(task_controller);
    DLOG_F(INFO, "Task controller loaded");

    auto upload_controller = UploadController::createShared();
    docEndpoints.append(upload_controller->getEndpoints());
    router->addController(upload_controller);
    DLOG_F(INFO, "Upload controller loaded");

    DLOG_F(INFO, "Starting to load API doc controller");
#if ENABLE_ASYNC
    router->addController(oatpp::swagger::AsyncController::createShared(docEndpoints));
#else
    router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
#endif
    DLOG_F(INFO, "API doc controller loaded");

    router->addController(WebSocketController::createShared());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    DLOG_F(INFO, "Loaded server components ... Prepare for starting ...");
    /* create server */
    oatpp::network::Server server(connectionProvider,
                                  connectionHandler);

    DLOG_F(INFO, "Server running on port %s...", connectionProvider->getProperty("port").toString()->c_str());

    server.run();
}

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
}

#ifdef _WIN32
// Define the signal handler function for Windows platform
BOOL WINAPI interruptHandler(DWORD signalNumber)
{
    if (signalNumber == CTRL_C_EVENT)
    {
        DLOG_F(INFO, "Ctrl+C pressed on Windows. Exiting...");
        throw std::runtime_error("KeyInterruption received");
    }
    return TRUE;
}

#else
// Define the signal handler function
void interruptHandler(int signalNumber, siginfo_t *info, void *context)
{
    Lithium::CrashReport::saveCrashLog("");
    oatpp::base::Environment::destroy();
    loguru::shutdown();
    ::exit(1);
}
#endif

void registerInterruptHandler()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(interruptHandler, TRUE);
#else
    struct sigaction sa;
    sa.sa_sigaction = interruptHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaction(SIGINT, &sa, NULL);
#endif
}

/**
 *  main
 */
int main(int argc, char *argv[])
{
    bindtextdomain("lithium", "locale");
    /* Only write the following 2 lines if creating an executable */
    setlocale(LC_ALL, "");
    textdomain("lithium");

    argparse::ArgumentParser program("Lithium Server");

    program.add_argument("-P", "--port").help(_("port the server running on")).default_value(8000);
    program.add_argument("-H", "--host").help(_("host the server running on")).default_value("0.0.0.0");
    program.add_argument("-C", "--config").help(_("path to the config file")).default_value("cpnfig.json");
    program.add_argument("-M", "--module-path").help(_("path to the modules directory")).default_value("modules");
    program.add_argument("-W", "--web-panel").help(_("web panel")).default_value(true);
    program.add_argument("-L", "--log-file").help(_("path to log file"));

    program.add_description(_("Lithium Command Line Interface:"));
    program.add_epilog(_("End."));

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &e)
    {
        LOG_F(ERROR, _("Failed to parser command line : %s"), e.what());
        std::exit(1);
    }

    try
    {
        // Init loguru log system
        loguru::init(argc, argv);
        // Set log file
        setupLogFile();
#ifdef _WIN32
        // Register ctrl-c handle for better debug
        registerInterruptHandler();
#endif
        // Run oatpp server
        Lithium::MyApp = std::make_shared<Lithium::LithiumApp>();
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
                    Lithium::MyApp->SetConfig("config/server/web",false);
                }
            }
        }
        catch (const std::bad_any_cast &e)
        {
            LOG_F(ERROR, "Invalid args format! Error: {}", e.what());
        }

        oatpp::base::Environment::init();
        // Run the main server
        run();
        // Clean up all
        oatpp::base::Environment::destroy();
    }
    catch (const std::exception &e)
    {
        Lithium::CrashReport::saveCrashLog(e.what());
        oatpp::base::Environment::destroy();
        loguru::shutdown();
        std::exit(EXIT_FAILURE);
    }
    return 0;
}