/*
 * App.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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
#include "controller/AsyncDeviceController.hpp"
#include "controller/AsyncTweakerController.hpp"
#include "controller/AsyncScriptController.hpp"
#include "controller/AsyncModuleController.hpp"
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
#include "atom/server/global_ptr.hpp"

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

#include "atom/log/loguru.hpp"

#define ADD_CONTROLLER(controller, docEndpoints, router, logMessage) \
    auto controller##_ptr = controller::createShared(); \
    docEndpoints.append(controller##_ptr->getEndpoints()); \
    router->addController(controller##_ptr); \
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

    DLOG_F(INFO, "Document endpoints loaded");

    /* Add document */

    ADD_CONTROLLER(SystemController, docEndpoints, router, "System controller");
    ADD_CONTROLLER(IOController, docEndpoints, router, "IO controller");
    ADD_CONTROLLER(ProcessController, docEndpoints, router, "System process controller");
    ADD_CONTROLLER(PHD2Controller, docEndpoints, router, "PHD2 controller");
    ADD_CONTROLLER(TaskController, docEndpoints, router, "Task controller");
    ADD_CONTROLLER(UploadController, docEndpoints, router, "Upload controller");
    ADD_CONTROLLER(WebSocketController, docEndpoints, router, "WebSocket controller");
    ADD_CONTROLLER(TweakerController, docEndpoints, router, "Tweaker controller");
    ADD_CONTROLLER(ScriptController, docEndpoints, router, "Script controller");
    ADD_CONTROLLER(ModuleController, docEndpoints, router, "Module controller");
    ADD_CONTROLLER(StaticController, docEndpoints, router, "Static file controller");
    ADD_CONTROLLER(DeviceController, docEndpoints, router, "Device controller");

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

    DLOG_F(INFO, "Server running on port {}...", connectionProvider->getProperty("port").toString()->c_str());

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

    loguru::set_fatal_handler([](const loguru::Message &message)
                              { 
        Lithium::CrashReport::saveCrashLog(std::string(message.prefix) + message.message); 
        oatpp::base::Environment::destroy(); });
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
#if ENABLE_GETTEXT
    bindtextdomain("lithium", "locale");
    /* Only write the following 2 lines if creating an executable */
    setlocale(LC_ALL, "");
    textdomain("lithium");
#endif

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