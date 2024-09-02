/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-16

Description: Simple wrapper around INDI server with atom component API
compatibility

**************************************************/

#include "atom/components/component.hpp"
#include <memory>
#include "atom/components/registry.hpp"

#include "driverlist.hpp"
#include "iconnector.hpp"
#include "indiserver.hpp"

#include "atom/log/loguru.hpp"

std::shared_ptr<INDIManager> mManager;

ATOM_MODULE(server_starter_indi, [](Component& mod) {
    LOG_F(INFO, "Registering server_starter_indi module...");
    mManager = std::make_shared<INDIManager>(std::make_unique<INDIConnector>());

    // 为模块添加文档说明
    mod.doc(
        "INDI Server Starter: This module allows the user to control the INDI "
        "(Instrument-Neutral Distributed Interface) server, which is designed "
        "for managing astronomical instruments and observatory operations.");

    // 定义模块接口，绑定方法到模块
    mod.def("start", &INDIManager::startServer, mManager, "astro",
            "Start the INDI server. This function initializes and runs the "
            "server, allowing it to accept commands and connect to devices.");
    mod.def("stop", &INDIManager::stopServer, mManager, "astro",
            "Stop the INDI server. This method cleanly shuts down the server, "
            "ensuring all ongoing processes are completed.");
    mod.def("is_running", &INDIManager::isRunning, mManager, "astro",
            "Check if the INDI server is currently running. Returns true if "
            "the server is active, otherwise false.");
    mod.def("is_installed", &INDIManager::isInstalled, mManager, "astro",
            "Check if the INDI server is installed on the system. This checks "
            "the local environment for the server binaries and configuration.");
    mod.def("set_prop", &INDIManager::setProp, mManager, "astro",
            "Set a property on the INDI server. This allows the user to modify "
            "settings for connected devices dynamically.");
    mod.def("get_prop", &INDIManager::getProp, mManager, "astro",
            "Get the current value of a property from the INDI server. Returns "
            "the property value associated with the specified device.");
    mod.def("get_state", &INDIManager::getState, mManager, "astro",
            "Retrieve the current state of the INDI server. This gives insight "
            "into whether the server is operational or if any issues have "
            "occurred.");
    mod.def("get_available_device", &INDIManager::getRunningDrivers, mManager,
            "astro",
            "Fetch the list of currently available devices that the INDI "
            "server manages. Provides information on all connected drivers.");

    mod.def("get_all_drivers", &readDriversListFromFiles, "astro",
            "Reads drivers list from files");

    mod.defType<INDIManager>("INDIManager");
    // 添加变量
    mod.addVariable("server_start.indi", mManager,
                    "indi manager: A reference to the INDI manager that "
                    "handles server operations and device interactions.");

    LOG_F(INFO, "Registered server_starter_indi module.");
});
