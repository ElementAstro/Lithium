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

#include "config/configor.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/function/invoke.hpp"
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
#include <csignal>
#endif

#include "atom/utils/argsview.hpp"

using namespace std::literals;

/**
 * @brief setup log file
 * @note This is called in main function
 */
void setupLogFile() {
    using namespace std::chrono;
    using namespace std::filesystem;

    path logsFolder = current_path() / "logs";
    if (!exists(logsFolder)) {
        create_directory(logsFolder);
    }

    auto now = system_clock::now();
    auto nowTimeT = system_clock::to_time_t(now);
    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &nowTimeT);
#else
    localtime_r(&nowTimeT, &localTime);
#endif

    std::ostringstream filenameStream;
    filenameStream << std::put_time(&localTime, "%Y%m%d_%H%M%S.log");
    path logFilePath = logsFolder / filenameStream.str();

    loguru::add_file(logFilePath.string().c_str(), loguru::Append,
                     loguru::Verbosity_MAX);

    loguru::set_fatal_handler([](const loguru::Message &message) {
        atom::system::saveCrashLog(std::string(message.prefix) +
                                   message.message);
    });
}

auto convertArgs(int argc, char **argv) -> std::vector<std::string> {
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return args;
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

    program.parse(argc, convertArgs(argc, argv));

    lithium::initLithiumApp(argc, argv);
    // Create shared instance
    lithium::myApp = lithium::LithiumApp::createShared();
    // Parse arguments
    try {
        auto setConfig = []<typename T>(const std::string &key, T value)
            requires IsBuiltIn<T>
        {
            if (!key.empty()) {
                lithium::myApp->setValue(key, value);
            } else {
                THROW_INVALID_ARGUMENT("Invalid key: " + key);
            }
        };

        setConfig("/lithium/server/host"_valid,
                  program.get<std::string>("host").value());
        setConfig("/lithium/server/port"_valid,
                  program.get<int>("port").value());
        setConfig("/lithium/server/configpath"_valid,
                  program.get<std::string>("config").value());
        setConfig("/lithium/server/modulepath"_valid,
                  program.get<std::string>("module-path").value());
        setConfig("/lithium/server/web"_valid,
                  program.get<bool>("web-panel").value());
        setConfig("/lithium/server/debug"_valid,
                  program.get<bool>("debug").value());
        setConfig("/lithium/server/logfile"_valid,
                  program.get<std::string>("log-file").value());

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
