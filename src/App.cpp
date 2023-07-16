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

#include "oatpp-swagger/Controller.hpp"

#include "oatpp/network/Server.hpp"

#include <argparse/argparse.hpp>

#include "LithiumApp.hpp"

void run()
{

    AppComponent components; // Create scope Environment components

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    oatpp::web::server::api::Endpoints docEndpoints;

    router->addController(oatpp::swagger::Controller::createShared(docEndpoints));
    router->addController(StaticController::createShared());
    router->addController(SystemController::createShared());
    router->addController(WebSocketController::createShared());
    router->addController(IOController::createShared());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* create server */
    oatpp::network::Server server(connectionProvider,
                                  connectionHandler);

    if (Lithium::MyApp.GetConfig("server/port") == nullptr)
    {
        Lithium::MyApp.SetConfig("server/port", 8000);
    }

    OATPP_LOGD("Server", "Running on port %s...", connectionProvider->getProperty("port").toString()->c_str());

    server.run();
}

/**
 *  main
 */
int main(int argc, const char *argv[])
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
        OATPP_LOGE("PreLaunch", "Failed to parser command line : %s", e.what());
        std::exit(1);
    }

    auto cmd_port = program.get<int>("--port");
    if (cmd_port != 8000)
    {
        OATPP_LOGD("PreLaunch", "Command line server port : %d", cmd_port);
        auto port = Lithium::MyApp.GetConfig("server/port");
        if (port != cmd_port)
        {
            Lithium::MyApp.SetConfig("server/port", cmd_port);
            OATPP_LOGI("PreLaunch", "Set server port to %d", cmd_port);
        }
    }

    oatpp::base::Environment::init();
    run();
    oatpp::base::Environment::destroy();
    return 0;
}