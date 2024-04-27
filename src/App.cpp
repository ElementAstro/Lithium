/*
 * app.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-14

Description: Main Entry

**************************************************/

#include "lithiumapp.hpp"

#include "preload.hpp"

#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "atom/system/crash.hpp"
#include "atom/web/utils.hpp"

// TODO: This is for debug only, please remove it in production
#define ENABLE_TERMINAL 1
#if ENABLE_TERMINAL
#include "debug/terminal.hpp"
using namespace Lithium::Terminal;
#endif

#include "server/App.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

#include "argparse/argparse.hpp"

/**
 * @brief setup log file
 * @note This is called in main function
 */
void setupLogFile() {
    std::filesystem::path logsFolder = std::filesystem::current_path() / "logs";
    if (!std::filesystem::exists(logsFolder)) {
        std::filesystem::create_directory(logsFolder);
    }
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time_t);
    char filename[100];
    std::strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.log", local_time);
    std::filesystem::path logFilePath = logsFolder / filename;
    loguru::add_file(logFilePath.string().c_str(), loguru::Append,
                     loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message) {
        Atom::System::saveCrashLog(std::string(message.prefix) +
                                   message.message);
    });
}

/**
 * @brief main function
 * @param argc number of arguments
 * @param argv arguments
 * @return 0 on success
 */
int main(int argc, char *argv[]) {
    // NOTE: gettext is not supported yet, it will cause compilation error on
    // Mingw64
    /* Add gettext */
#if ENABLE_GETTEXT
    bindtextdomain("lithium", "locale");
    /* Only write the following 2 lines if creating an executable */
    setlocale(LC_ALL, "");
    textdomain("lithium");
#endif
    // Set log file
    setupLogFile();

    // Init loguru log system
    loguru::init(argc, argv);

    /* Parse arguments */
    argparse::ArgumentParser program("Lithium Server");

    // NOTE: The command arguments' priority is higher than the config file
    program.add_argument("-P", "--port")
        .help("port the server running on")
        .default_value(8000);
    program.add_argument("-H", "--host")
        .help("host the server running on")
        .default_value("0.0.0.0");
    program.add_argument("-C", "--config")
        .help("path to the config file")
        .default_value("config.json");
    program.add_argument("-M", "--module-path")
        .help("path to the modules directory")
        .default_value("./modules");
    program.add_argument("-W", "--web-panel")
        .help("web panel")
        .default_value(true);
    program.add_argument("-D", "--debug")
        .help("debug mode")
        .default_value(false);
    program.add_argument("-L", "--log-file").help("path to log file");

    program.add_description("Lithium Command Line Interface:");
    program.add_epilog("End.");

    program.parse_args(argc, argv);

    Lithium::InitLithiumApp(argc, argv);
    // Create shared instance
    Lithium::MyApp = Lithium::LithiumApp::createShared();
    // Parse arguments
    try {
        auto cmd_host = program.get<std::string>("--host");
        auto cmd_port = program.get<int>("--port");
        auto cmd_config_path = program.get<std::string>("--config");
        auto cmd_module_path = program.get<std::string>("--module-path");
        auto cmd_web_panel = program.get<bool>("--web-panel");
        auto cmd_debug = program.get<bool>("--debug");

        if (!cmd_host.empty()) {
            Lithium::MyApp->SetConfig(
                {{"key", "config/server/host"}, {"value", cmd_host}});
            DLOG_F(INFO, "Set server host to {}", cmd_host);
        }
        if (cmd_port != 8000) {
            DLOG_F(INFO, "Command line server port : {}", cmd_port);

            auto port = Lithium::MyApp->GetConfig("config/server")
                            .value<int>("port", 8000);
            if (port != cmd_port) {
                Lithium::MyApp->SetConfig(
                    {{"key", "config/server/port"}, {"value", cmd_port}});
                DLOG_F(INFO, "Set server port to {}", cmd_port);
            }
        }
        if (!cmd_config_path.empty()) {
            Lithium::MyApp->SetConfig({{"key", "config/server/configpath"},
                                       {"value", cmd_config_path}});
            DLOG_F(INFO, "Set server config path to {}", cmd_config_path);
        }
        if (!cmd_module_path.empty()) {
            Lithium::MyApp->SetConfig({{"key", "config/server/modulepath"},
                                       {"value", cmd_module_path}});
            DLOG_F(INFO, "Set server module path to {}", cmd_module_path);
        }

        if (!cmd_web_panel) {
            if (Lithium::MyApp->GetConfig("config/server/web").get<bool>()) {
                Lithium::MyApp->SetConfig(
                    {{"key", "config/server/web"}, {"value", false}});
                DLOG_F(INFO, "Disable web panel");
            }
        }

        if (cmd_debug) {
            if (!Lithium::MyApp->GetConfig("config/server/debug").get<bool>()) {
                Lithium::MyApp->SetConfig(
                    {{"key", "config/server/debug"}, {"value", true}});
            }
        } else {
            Lithium::MyApp->SetConfig(
                {{"key", "config/server/debug"}, {"value", false}});
            DLOG_F(INFO, "Disable debug mode");
        }
    } catch (const std::bad_any_cast &e) {
        LOG_F(ERROR, "Invalid args format! Error: {}", e.what());
        Atom::System::saveCrashLog(e.what());
        return 1;
    }

    // In debug mode run the terminal first and will not run the server
    if (Lithium::MyApp->GetConfig("config/server/debug").get<bool>()) {
        ConsoleTerminal terminal;
        terminal.run();
    } else {
        runServer();
    }

    return 0;
}