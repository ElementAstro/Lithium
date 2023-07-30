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

#include "controller/StaticController.hpp"
#include "controller/SystemController.hpp"
#include "controller/WebSocketController.hpp"
#include "controller/IOController.hpp"
#include "controller/AuthController.hpp"
#include "controller/ProcessController.hpp"
#include "controller/PHD2Controller.hpp"
#include "controller/TaskController.hpp"

#if ENABLE_ASYNC
#include "oatpp-swagger/AsyncController.hpp"
#else
#include "oatpp-swagger/Controller.hpp"
#endif

#include "oatpp/network/Server.hpp"

#include <argparse/argparse.hpp>

#include "LithiumApp.hpp"

#include "modules/system/crash.hpp"

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
    if (Lithium::MyApp->GetConfig("server/port") == nullptr)
    {
        Lithium::MyApp->SetConfig("server/port", 8000);
    }

    LOG_F(INFO, "Loading App component ...");

    AppComponent components(Lithium::MyApp->GetConfig("server/port").get<int>()); // Create scope Environment components

    LOG_F(INFO, "App component loaded");
    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    oatpp::web::server::api::Endpoints docEndpoints;

    LOG_F(INFO, "Document endpoints loaded");

    /* Add document */

    auto static_controller = StaticController::createShared();
    docEndpoints.append(static_controller->getEndpoints());
    router->addController(static_controller);

    auto system_controller = SystemController::createShared();
    docEndpoints.append(system_controller->getEndpoints());
    router->addController(system_controller);

    auto io_controller = IOController::createShared();
    docEndpoints.append(io_controller->getEndpoints());
    router->addController(io_controller);

    auto process_controller = ProcessController::createShared();
    docEndpoints.append(process_controller->getEndpoints());
    router->addController(process_controller);

    auto phd2_controller = PHD2Controller::createShared();
    docEndpoints.append(phd2_controller->getEndpoints());
    router->addController(phd2_controller);

    auto task_controller = TaskController::createShared();
    docEndpoints.append(task_controller->getEndpoints());
    router->addController(task_controller);

    // auto auth_controller = AuthController::createShared();
    //   docEndpoints.append(auth_controller->getEndpoints());
    // router->addController(auth_controller);

#if ENABLE_ASYNC
    router->addController(oatpp::swagger::AsyncController::createShared(docEndpoints));
#else
    router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
#endif

    router->addController(WebSocketController::createShared());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* create server */
    oatpp::network::Server server(connectionProvider,
                                  connectionHandler);

    LOG_F(INFO, "Running on port %s...", connectionProvider->getProperty("port").toString()->c_str());

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
        LOG_F(INFO, "Ctrl+C pressed on Windows. Exiting...");
        throw std::runtime_error("KeyInterruption received");
    }
    return TRUE;
}

#else
// Define the signal handler function
void interruptHandler(int signalNumber, siginfo_t *info, void *context)
{
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
    argparse::ArgumentParser program("Lithium");

    program.add_argument("-P", "--port").help("port the server running on").default_value(8000);

    program.add_description("Lithium Command Line Interface:");
    program.add_epilog("End.");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &e)
    {
        LOG_F(ERROR, "Failed to parser command line : %s", e.what());
        std::exit(1);
    }

    auto cmd_port = program.get<int>("--port");
    if (cmd_port != 8000)
    {
        LOG_F(INFO, "Command line server port : %d", cmd_port);

        auto port = Lithium::MyApp->GetConfig("server/port");
        if (port != cmd_port)
        {
            Lithium::MyApp->SetConfig("server/port", cmd_port);
            LOG_F(INFO, "Set server port to %d", cmd_port);
        }
    }

    try
    {
        // Init loguru log system
        loguru::init(argc, argv);
        // Set log file
        setupLogFile();
        // Register ctrl-c handle for better debug
        registerInterruptHandler();
        // Run oatpp server
        Lithium::MyApp = std::make_shared<Lithium::LithiumApp>();
        oatpp::base::Environment::init();
        run();
        oatpp::base::Environment::destroy();
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error: %s", e.what());
        Lithium::CrashReport::saveCrashLog(e.what());
        std::exit(EXIT_FAILURE);
    }
    return 0;
}