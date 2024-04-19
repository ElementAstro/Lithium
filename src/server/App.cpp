/*
 * App.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Main

**************************************************/

#include "App.hpp"

#include "./Runner.hpp"

#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

void run() {
    /* Register Components in scope of run() method */
    AppComponent components;

    Runner runner(OATPP_GET_COMPONENT(oatpp::Object<ConfigDto>),
                  OATPP_GET_COMPONENT(std::shared_ptr<oatpp::async::Executor>));

    runner.start();

    runner.join();
}

int runServer() {
    oatpp::base::Environment::init();

    run();

    oatpp::base::Environment::destroy();

    return 0;
}
