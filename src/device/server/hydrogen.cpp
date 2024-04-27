/*
 * manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Hydrogen Device Manager

**************************************************/

#include "hydrogen.hpp"
#include "../utils/utils.hpp"
#include "config.h"
#include "hydrogen_driver.hpp"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

#include <regex>

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/system.hpp"

namespace Lithium {
HydrogenManager::HydrogenManager(const std::string &hst, int prt,
                                 const std::string &cfg, const std::string &dta,
                                 const std::string &fif) {
    host = hst;
    port = prt;
    config_path = cfg;
    data_path = dta;
    fifo_path = fif;
}

HydrogenManager::~HydrogenManager() {}

std::shared_ptr<HydrogenManager> HydrogenManager::createShared(
    const std::string &hst, int prt, const std::string &cfg,
    const std::string &dta, const std::string &fif) {
    return std::make_shared<HydrogenManager>(hst, prt, cfg, dta, fif);
}

std::unique_ptr<HydrogenManager> HydrogenManager::createUnique(
    const std::string &hst, int prt, const std::string &cfg,
    const std::string &dta, const std::string &fif) {
    return std::make_unique<HydrogenManager>(hst, prt, cfg, dta, fif);
}

std::vector<decorator<std::function<json(const json &)>>> HydrogenManager::getFunctions()
{
    std::vector<decorator<std::function<json(const json &)>>> functions;
    //auto startServer = make_decorator(std::function<json(HydrogenManager*,json)>(
    //    &HydrogenManager::_startServer
    //));
    //functions.push_back(startServer);
    return functions;
}

json HydrogenManager::_startServer(const json &params)
{
    if (!isInstalled()) {
        LOG_F(ERROR, "Hydrogen is not installed");
        return {
            {"error", "Hydrogen is not installed"}
        };
    }
    if (!isRunning())
    {
        LOG_F(INFO, "Starting server");
        if (!startServer())
        {
            LOG_F(ERROR, "Failed to start server");
            return {
                {"error", "Failed to start server"}
            };
        }
    }
    return {
        {"success", true}
    };
}

#ifdef _WIN32
bool HydrogenManager::startServer() {
    // If there is an Hydrogen server running, just kill it
    // Surely, this is not the best way to do this, but it works.
    // If there is no server running, start it.
    if (isRunning()) {
        stopServer();
    }
    DLOG_F(INFO, "Deleting fifo pipe at: {}", fifo_path);
    if (!Atom::IO::removeFile(fifo_path)) {
        LOG_F(ERROR, "Failed to delete fifo pipe at: {}", fifo_path);
        return false;
    }
#if ENABLE_INDI
    std::string cmd = "indiserver -p " + std::to_string(port) +
                      " -m 100 -v -f " + fifo_path +
                      " > C:\\tmp\\indiserver.log 2>&1";
#else
    std::string cmd = "hydrogenserver -p " + std::to_string(port) +
                      " -m 100 -v -f " + fifo_path +
                      " > C:\\tmp\\Hydrogenserver.log 2>&1";
#endif
    try {
        if (executeCommand(cmd) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    // Just start the server without driver
    DLOG_F(INFO, "Started Hydrogen server on port {}", port);
    return true;
}
#else
bool HydrogenManager::startServer() {
    // If there is a Hydrogen server running, just kill it
    if (isRunning()) {
        stopServer();
    }
    // Clear old fifo pipe and create new one
    DLOG_F(INFO, "Deleting fifo pipe at: {}", fifo_path);
    if (!Atom::IO::removeFile(fifo_path)) {
        LOG_F(ERROR, "Failed to delete fifo pipe at: {}", fifo_path);
        return false;
    }
    int res = system(("mkfifo " + fifo_path).c_str());
    if (res != 0) {
        LOG_F(ERROR, "Failed to create fifo pipe at: {}", fifo_path);
        return false;
    }
    // Just start the server without driver
#if ENABLE_INDI
    std::string cmd = "indiserver -p " + std::to_string(port) +
                      " -m 100 -v -f " + fifo_path +
                      " > /tmp/indiserver.log 2>&1 &";
#else
    std::string cmd = "hydrogenserver -p " + std::to_string(port) +
                      " -m 100 -v -f " + fifo_path +
                      " > /tmp/hydrogenserver.log 2>&1 &";
#endif
    res = system(cmd.c_str());
    if (res != 0) {
        LOG_F(ERROR, "Failed to start Hydrogenserver, error code is {}", res);
        return false;
    }
    DLOG_F(INFO, "Started Hydrogen server on port {}", port);
    return true;
}
#endif

bool HydrogenManager::stopServer() {
    if (!isRunning()) {
        DLOG_F(WARNING, "Hydrogen server is not running");
        return true;
    }
#if ENABLE_INDI
#ifdef _WIN32
    std::string cmd = "taskkill /f /im indiserver.exe >nul 2>&1";
#else
    std::string cmd = "killall indiserver >/dev/null 2>&1";
#endif
#else
#ifdef _WIN32
    std::string cmd = "taskkill /f /im hydrogenserver.exe >nul 2>&1";
#else
    std::string cmd = "killall hydrogenserver >/dev/null 2>&1";
#endif
#endif
    DLOG_F(INFO, "Terminating Hydrogen server");
    try {
        if (executeCommand(cmd) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    DLOG_F(INFO, "Hydrogen server terminated successfully");
    return true;
}

bool HydrogenManager::isRunning() {
    // A little bit hacky, but it works. We need a dynamic way to check if the
    // server is running Not stupid like this :P
#if ENABLE_INDI
#ifdef _WIN32
    std::string processName = "indiserver.exe";
    return Atom::System::isProcessRunning(processName);
#else
    std::string processName = "indiserver";
    return Atom::System::isProcessRunning(processName);
#endif
#else
#ifdef _WIN32
    std::string processName = "hydrogenserver.exe";
    return Atom::System::isProcessRunning(processName);
#else
    std::string processName = "hydrogenserver";
    return Atom::System::isProcessRunning(processName);
#endif
#endif
}

bool HydrogenManager::isInstalled() {
#ifdef _WIN32
    if (!Atom::System::checkSoftwareInstalled("hydrogenserver.exe")) {
        return false;
    }
#else
    if (!Atom::System::checkSoftwareInstalled("hydrogenserver")) {
        return false;
    }
#endif
    return true;
}

bool HydrogenManager::startDriver(
    std::shared_ptr<HydrogenDeviceContainer> driver) {
    std::string cmd = "start " + driver->binary;
    if (driver->skeleton != "") {
        cmd += " -s \"" + driver->skeleton + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
    DLOG_F(INFO, "Cmd: {}", full_cmd);
    try {
        if (executeCommand(full_cmd) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", full_cmd);
            return false;
        }
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", full_cmd,
              e.what());
        return false;
    }
    running_drivers.emplace(driver->label, driver);
    return true;
}

bool HydrogenManager::stopDriver(
    std::shared_ptr<HydrogenDeviceContainer> driver) {
    std::string cmd = "stop " + driver->binary;
    if (driver->binary.find("@") == std::string::npos) {
        cmd += " -n \"" + driver->label + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
    DLOG_F(INFO, "Cmd: {}", full_cmd);
    try {
        if (executeCommand(full_cmd) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", full_cmd);
            return false;
        }
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", full_cmd,
              e.what());
        return false;
    }
    DLOG_F(INFO, "Stop running driver: {}", driver->label);
    running_drivers.erase(driver->label);
    return true;
}

bool HydrogenManager::setProp(const std::string &dev, const std::string &prop,
                              const std::string &element,
                              const std::string &value) {
    std::stringstream ss;

    ss << "hydrogen_setprop " << dev << "." << prop << "." << element << "="
       << value;
    std::string cmd = ss.str();
    DLOG_F(INFO, "Cmd: {}", cmd);
    try {
        if (executeCommand(cmd) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    DLOG_F(INFO, "Set property: {}.{} to {}", dev, prop, value);
    return true;
}

std::string HydrogenManager::getProp(const std::string &dev,
                                     const std::string &prop,
                                     const std::string &element) {
    std::stringstream ss;
#if ENABLE_INDI
    ss << "indi_getprop " << dev << "." << prop << "." << element;
#else
    ss << "hydrogen_getprop " << dev << "." << prop << "." << element;
#endif
    std::string cmd = ss.str();
    try {
        std::string output = executeCommand(cmd);
        size_t equalsPos = output.find('=');
        if (equalsPos != std::string::npos && equalsPos + 1 < output.length()) {
            return output.substr(equalsPos + 1,
                                 output.length() - equalsPos - 2);
        }
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return "";
    }
    return "";
}

std::string HydrogenManager::getState(const std::string &dev,
                                      const std::string &prop) {
    return getProp(dev, prop, "_STATE");
}

#if ENABLE_FASTHASH
emhash8::HashMap<std::string, std::shared_ptr<HydrogenDeviceContainer>>
HydrogenManager::getRunningDrivers()
#else
std::unordered_map<std::string, std::shared_ptr<HydrogenDeviceContainer>>
HydrogenManager::getRunningDrivers()
#endif
{
    return running_drivers;
}

#if ENABLE_FASTHASH
std::vector<emhash8::HashMap<std::string, std::string>>
HydrogenManager::getDevices()
#else
std::vector<std::unordered_map<std::string, std::string>>
HydrogenManager::getDevices()
#endif
{
#if ENABLE_FASTHASH
    std::vector<emhash8::HashMap<std::string, std::string>> devices;
#else
    std::vector<std::unordered_map<std::string, std::string>> devices;
#endif
#if ENABLE_INDI
    std::string cmd = "indi_getprop *.CONNECTION.CONNECT";
#else
    std::string cmd = "hydrogen_getprop *.CONNECTION.CONNECT";
#endif
    std::array<char, 128> buffer;
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        LOG_F(ERROR, "Failed to execute command: {}", cmd);
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    std::vector<std::string> lines = {"", ""};
    for (char token : result) {
        if (token == '\n') {
            std::unordered_map<std::string, std::string> device;
            std::stringstream ss(lines[0]);
            std::string item;
            while (getline(ss, item, '.')) {
                device["device"] = item;
            }
            device["connected"] = (lines[1] == "On") ? "true" : "false";
            devices.push_back(device);
            lines = {"", ""};
        } else if (token == '=') {
            lines[1] = lines[1].substr(0, lines[1].length() - 1);
        } else if (token == ' ') {
            continue;
        } else {
            lines[(lines[0] == "") ? 0 : 1] += token;
        }
    }
    return devices;
}
}  // namespace Lithium
