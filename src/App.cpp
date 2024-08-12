/*
 * app.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-14

Description: Main Entry

**************************************************/

#include "LithiumApp.hpp"

#include "preload.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/crash.hpp"
#include "atom/web/utils.hpp"

// TODO: This is for debug only, please remove it in production
#if !IN_PRODUCTION
#define ENABLE_TERMINAL 1
#endif
#if ENABLE_TERMINAL
#include "debug/terminal.hpp"
using namespace lithium::debug;
#endif

// In release mode, we will disable the debugger
#if IN_PRODUCTION
#include "atom/system/nodebugger.hpp"
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

#include "atom/utils/argsview.hpp"

using namespace std::literals;

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
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&nowTimeT);
    char filename[100];
    std::strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.log", localTime);
    std::filesystem::path logFilePath = logsFolder / filename;
    loguru::add_file(logFilePath.string().c_str(), loguru::Append,
                     loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message) {
        atom::system::saveCrashLog(std::string(message.prefix) +
                                   message.message);
    });
}

/**
 * @brief main function
 * @param argc number of arguments
 * @param argv arguments
 * @return 0 on success
 */
auto main(int argc, char *argv[]) -> int {
#if ENABLE_CPPTRACE
    cpptrace::init();
#endif
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
    atom::utils::ArgumentParser program("Lithium Server"s);

    // NOTE: The command arguments' priority is higher than the config file
    program.addArgument("port", atom::utils::ArgumentParser::ArgType::INTEGER,
                        false, 8000, "Port of the server", {"p"});
    program.addArgument("host", atom::utils::ArgumentParser::ArgType::STRING,
                        false, "0.0.0.0"s, "Host of the server", {"h"});
    program.addArgument("config", atom::utils::ArgumentParser::ArgType::STRING,
                        false, "config.json"s, "Path to the config file",
                        {"c"});
    program.addArgument("module-path",
                        atom::utils::ArgumentParser::ArgType::STRING, false,
                        "modules"s, "Path to the modules directory", {"m"});
    program.addArgument("web-panel",
                        atom::utils::ArgumentParser::ArgType::BOOLEAN, false,
                        true, "Enable web panel", {"w"});
    program.addArgument("debug", atom::utils::ArgumentParser::ArgType::BOOLEAN,
                        false, false, "Enable debug mode", {"d"});
    program.addArgument("log-file",
                        atom::utils::ArgumentParser::ArgType::STRING, false,
                        ""s, "Path to the log file", {"l"});

    program.addDescription("Lithium Command Line Interface:");
    program.addEpilog("End.");

    program.parse(argc, argv);

    lithium::initLithiumApp(argc, argv);
    // Create shared instance
    lithium::myApp = lithium::LithiumApp::createShared();
    // Parse arguments
    try {
        auto cmdHost = program.get<std::string>("host");
        auto cmdPort = program.get<int>("port");
        auto cmdConfigPath = program.get<std::string>("config");
        auto cmdModulePath = program.get<std::string>("module-path");
        auto cmdWebPanel = program.get<bool>("web-panel");
        auto cmdDebug = program.get<bool>("debug");

        // TODO: We need a new way to handle command line arguments.
        // Maybe we will generate a json object or a map and then given to the
        // lithiumapp for initialization.
        /*
        if (!cmd_host.empty()) {
            lithium::MyApp->SetConfig(
                {{"key", "config/server/host"}, {"value", cmd_host}});
            DLOG_F(INFO, "Set server host to {}", cmd_host);
        }
        if (cmd_port != 8000) {
            DLOG_F(INFO, "Command line server port : {}", cmd_port);

            auto port = lithium::MyApp->GetConfig("config/server")
                            .value<int>("port", 8000);
            if (port != cmd_port) {
                lithium::MyApp->SetConfig(
                    {{"key", "config/server/port"}, {"value", cmd_port}});
                DLOG_F(INFO, "Set server port to {}", cmd_port);
            }
        }
        if (!cmd_config_path.empty()) {
            lithium::MyApp->SetConfig({{"key", "config/server/configpath"},
                                       {"value", cmd_config_path}});
            DLOG_F(INFO, "Set server config path to {}", cmd_config_path);
        }
        if (!cmd_module_path.empty()) {
            lithium::MyApp->SetConfig({{"key", "config/server/modulepath"},
                                       {"value", cmd_module_path}});
            DLOG_F(INFO, "Set server module path to {}", cmd_module_path);
        }

        if (!cmd_web_panel) {
            if (lithium::MyApp->GetConfig("config/server/web").get<bool>()) {
                lithium::MyApp->SetConfig(
                    {{"key", "config/server/web"}, {"value", false}});
                DLOG_F(INFO, "Disable web panel");
            }
        }

        if (cmd_debug) {
            if (!lithium::MyApp->GetConfig("config/server/debug").get<bool>()) {
                lithium::MyApp->SetConfig(
                    {{"key", "config/server/debug"}, {"value", true}});
            }
        } else {
            lithium::MyApp->SetConfig(
                {{"key", "config/server/debug"}, {"value", false}});
            DLOG_F(INFO, "Disable debug mode");
        }
        */

    } catch (const std::bad_any_cast &e) {
        LOG_F(ERROR, "Invalid args format! Error: {}", e.what());
        atom::system::saveCrashLog(e.what());
        return 1;
    }

    ConsoleTerminal terminal;
    globalConsoleTerminal = &terminal;
    terminal.run();
    lithium::runServer({argc, const_cast<const char **>(argv)});

    return 0;
}
