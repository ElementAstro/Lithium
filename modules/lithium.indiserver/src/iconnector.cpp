/*
 * manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: INDI Device Manager

**************************************************/

#include "iconnector.hpp"
#include "container.hpp"

#include <regex>

#include "atom/error/exception.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/system/process.hpp"
#include "atom/system/software.hpp"

INDIConnector::INDIConnector(const std::string &hst, int prt,
                             const std::string &cfg, const std::string &dta,
                             const std::string &fif)
    : host_(hst) {
    host_ = hst;
    port_ = prt;
    config_path_ = cfg;
    data_path_ = dta;
    fifo_path_ = fif;
}

auto INDIConnector::startServer() -> bool {
    // If there is an INDI server running, just kill it
    // Surely, this is not the best way to do this, but it works.
    // If there is no server running, start it.
    if (isRunning()) {
        stopServer();
    }
    DLOG_F(INFO, "Deleting fifo pipe at: {}", fifo_path_);
    if (!atom::io::removeFile(fifo_path_)) {
        LOG_F(ERROR, "Failed to delete fifo pipe at: {}", fifo_path_);
        return false;
    }
    std::string cmd = "indiserver -p " + std::to_string(port_) +
                      " -m 100 -v -f " + fifo_path_ +
                      " > /tmp/indiserver.log 2>&1";
    try {
        if (atom::system::executeCommand(cmd, false) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const atom::error::RuntimeError &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    // Just start the server without driver
    DLOG_F(INFO, "Started INDI server on port {}", port_);
    return true;
}

auto INDIConnector::stopServer() -> bool {
    if (!isRunning()) {
        DLOG_F(WARNING, "INDI server is not running");
        return true;
    }
    std::string cmd = "killall indiserver >/dev/null 2>&1";
    DLOG_F(INFO, "Terminating INDI server");
    try {
        if (atom::system::executeCommand(cmd, false) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    DLOG_F(INFO, "INDI server terminated successfully");
    return true;
}

auto INDIConnector::isRunning() -> bool {
    // A little bit hacky, but it works. We need a dynamic way to check if the
    // server is running Not stupid like this :P
    std::string processName = "indiserver";
    return atom::system::isProcessRunning(processName);
}

auto INDIConnector::isInstalled() -> bool {
    return atom::system::checkSoftwareInstalled("hydrogenserver");
}

auto INDIConnector::startDriver(
    const std::shared_ptr<INDIDeviceContainer> &driver) -> bool {
    std::string cmd = "start " + driver->label;
    if (driver->skeleton != "") {
        cmd += " -s \"" + driver->skeleton + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string fullCmd = "echo \"" + cmd + "\" > " + fifo_path_;
    DLOG_F(INFO, "Cmd: {}", fullCmd);
    try {
        if (atom::system::executeCommand(fullCmd, false) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", fullCmd);
            return false;
        }
    } catch (const atom::error::RuntimeError &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", fullCmd,
              e.what());
        return false;
    }
    running_drivers_.emplace(driver->label, driver);
    return true;
}

auto INDIConnector::stopDriver(
    const std::shared_ptr<INDIDeviceContainer> &driver) -> bool {
    std::string cmd = "stop " + driver->binary;
    if (driver->binary.find('@') == std::string::npos) {
        cmd += " -n \"" + driver->label + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string fullCmd = "echo \"" + cmd + "\" > " + fifo_path_;
    DLOG_F(INFO, "Cmd: {}", fullCmd);
    try {
        if (atom::system::executeCommand(fullCmd, false) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", fullCmd);
            return false;
        }
    } catch (const atom::error::RuntimeError &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", fullCmd,
              e.what());
        return false;
    }
    DLOG_F(INFO, "Stop running driver: {}", driver->label);
    running_drivers_.erase(driver->label);
    return true;
}

auto INDIConnector::setProp(const std::string &dev, const std::string &prop,
                            const std::string &element,
                            const std::string &value) -> bool {
    std::stringstream sss;
    sss << "indi_setprop " << dev << "." << prop << "." << element << "="
        << value;
    std::string cmd = sss.str();
    DLOG_F(INFO, "Cmd: {}", cmd);
    try {
        if (atom::system::executeCommand(cmd, false) != "") {
            LOG_F(ERROR, "Failed to execute command: {}", cmd);
            return false;
        }
    } catch (const atom::error::RuntimeError &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
        return false;
    }
    DLOG_F(INFO, "Set property: {}.{} to {}", dev, prop, value);
    return true;
}

auto INDIConnector::getProp(const std::string &dev, const std::string &prop,
                            const std::string &element) -> std::string {
    std::stringstream sss;
#if ENABLE_INDI
    sss << "indi_getprop " << dev << "." << prop << "." << element;
#else
    sss << "indi_getprop " << dev << "." << prop << "." << element;
#endif
    std::string cmd = sss.str();
    try {
        std::string output = atom::system::executeCommand(cmd, false);
        size_t equalsPos = output.find('=');
        if (equalsPos != std::string::npos && equalsPos + 1 < output.length()) {
            return output.substr(equalsPos + 1,
                                 output.length() - equalsPos - 2);
        }
    } catch (const atom::error::RuntimeError &e) {
        LOG_F(ERROR, "Failed to execute command: {} with {}", cmd, e.what());
    }
    return "";
}

auto INDIConnector::getState(const std::string &dev,
                             const std::string &prop) -> std::string {
    return getProp(dev, prop, "_STATE");
}

#if ENABLE_FASTHASH
emhash8::HashMap<std::string, std::shared_ptr<INDIDeviceContainer>>
INDIConnector::getRunningDrivers()
#else
auto INDIConnector::getRunningDrivers()
    -> std::unordered_map<std::string, std::shared_ptr<INDIDeviceContainer>>
#endif
{
    return running_drivers_;
}

#if ENABLE_FASTHASH
std::vector<emhash8::HashMap<std::string, std::string>>
INDIConnector::getDevices()
#else
auto INDIConnector::getDevices()
    -> std::vector<std::unordered_map<std::string, std::string>>
#endif
{
#if ENABLE_FASTHASH
    std::vector<emhash8::HashMap<std::string, std::string>> devices;
#else
    std::vector<std::unordered_map<std::string, std::string>> devices;
#endif
    std::string cmd = "indi_getprop *.CONNECTION.CONNECT";
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        LOG_F(ERROR, "Failed to execute command: {}", cmd);
        THROW_RUNTIME_ERROR("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    std::vector<std::string> lines = {"", ""};
    for (char token : result) {
        if (token == '\n') {
            std::unordered_map<std::string, std::string> device;
            std::stringstream sss(lines[0]);
            std::string item;
            while (getline(sss, item, '.')) {
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
