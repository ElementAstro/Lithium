/*
 * indidevice_manager.cpp
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

Date: 2023-3-29

Description: INDI Device Manager

**************************************************/

#include "indidevice_manager.hpp"
#include "indi_device.hpp"
#include "device_utils.hpp"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

#include <regex>

#include "loguru/loguru.hpp"

INDIManager::INDIManager(const std::string &hst, int prt, const std::string &cfg, const std::string &dta, const std::string &fif)
{
    host = hst;
    port = prt;
    config_path = cfg;
    data_path = dta;
    fifo_path = fif;
}

#ifdef _WIN32
void INDIManager::start_server()
{
    // If there is an INDI server running, just kill it
    if (is_running())
    {
        stop_server();
    }
    LOG_F(INFO, "Deleting fifo pipe at: {}", fifo_path);
    DeleteFileA(fifo_path.c_str());
    CreateNamedPipeA(fifo_path.c_str(), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 0, NULL);
    // Just start the server without driver
    std::string cmd = "indiserver -p " + std::to_string(port) + " -m 100 -v -f " + fifo_path + " > C:\\tmp\\indiserver.log 2>&1";
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessA(NULL, const_cast<char *>(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        throw std::runtime_error("Failed to execute command!");
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    LOG_F(INFO, "Started INDI server on port {}", port);
}
#else
void INDIManager::start_server()
{
    // If there is a INDI server running, just kill it
    if (is_running())
    {
        stop_server();
    }
    // Clear old fifo pipe and create new one
    LOG_F(INFO, "Deleting fifo pipe at: {}", fifo_path);
    system(("rm -f " + fifo_path).c_str());
    system(("mkfifo " + fifo_path).c_str());
    // Just start the server without driver
    std::string cmd = "indiserver -p " + std::to_string(port) + " -m 100 -v -f " + fifo_path + " > /tmp/indiserver.log 2>&1 &";

    std::cout << cmd << std::endl;
    std::cout << "Started INDI server on port " << port << std::endl;
    system(cmd.c_str());
}
#endif

void INDIManager::stop_server()
{
#ifdef _WIN32
    std::string cmd = "taskkill /f /im indiserver.exe >nul 2>&1";
#else
    std::string cmd = "killall indiserver >/dev/null 2>&1";
#endif
    int res = system(cmd.c_str());
    if (res == 0)
    {
        LOG_F(INFO, "INDI server terminated successfully");
    }
    else
    {
        LOG_F(ERROR, "Failed to terminate indiserver, error code is {}", res);
    }
}

#ifdef _WIN32
bool INDIManager::is_running()
{
    std::string processName = "indiserver.exe";
    bool isRunning = false;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32))
        {
            do
            {
                std::string foundProcess(pe32.szExeFile);
                if (foundProcess.find(processName) != std::string::npos)
                {
                    isRunning = true;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    return isRunning;
}
#else
bool INDIManager::is_running()
{
    std::string output = "";
    FILE *pipe = popen(("ps -ef | grep indiserver | grep -v grep | wc -l").c_str(), "r");
    if (!pipe)
        return false;
    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
            output += buffer;
    }
    pclose(pipe);
    return (output != "0");
}
#endif

void INDIManager::start_driver(std::shared_ptr<INDIDeviceContainer> driver)
{
    std::string cmd = "start " + driver->binary;

    if (driver->skeleton != "")
    {
        cmd += " -s \"" + driver->skeleton + "\"";
    }

    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (CreateProcess(NULL, const_cast<char *>(full_cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        LOG_F(INFO, "Started driver : {}", driver->name);

        running_drivers.emplace(driver->label, driver);
    }
    else
    {
        LOG_F(ERROR, "Failed to start driver: {}", driver->name);
    }
#else
    system(full_cmd.c_str());
    LOG_F(INFO, "Started driver : {}", driver->name);
#endif

    running_drivers.emplace(driver->label, driver);
}

void INDIManager::stop_driver(std::shared_ptr<INDIDeviceContainer> driver)
{
    std::string cmd = "stop " + driver->binary;
    if (driver->binary.find("@") == std::string::npos)
    {
        cmd += " -n \"" + driver->label + "\"";
    }
    cmd = std::regex_replace(cmd, std::regex("\""), "\\\"");
    std::string full_cmd = "echo \"" + cmd + "\" > " + fifo_path;
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    if (CreateProcess(NULL, const_cast<char *>(full_cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        LOG_F(INFO,"Stop running driver: {}",driver->label );
        running_drivers.erase(driver->label);
    }
    else
    {
        LOG_F(ERROR,"Failed to stop driver: {}", driver->label);
    }
#else
    system(full_cmd.c_str());
    LOG_F(INFO,"Stop running driver: {}",driver->label );
    running_drivers.erase(driver->label);
#endif
}

#ifdef _WIN32
void INDIManager::set_prop(const std::string &dev, const std::string &prop, const std::string &element, const std::string &value)
{
    std::stringstream ss;
    ss << "indi_setprop " << dev << "." << prop << "." << element << "=" << value;
    std::string cmd = ss.str();
    execute_command(cmd);
}

std::string INDIManager::get_prop(const std::string &dev, const std::string &prop, const std::string &element)
{
    std::stringstream ss;
    ss << "indi_getprop " << dev << "." << prop << "." << element;
    std::string cmd = ss.str();
    std::string output = execute_command(cmd);
    size_t equalsPos = output.find('=');
    if (equalsPos != std::string::npos && equalsPos + 1 < output.length())
    {
        return output.substr(equalsPos + 1, output.length() - equalsPos - 2);
    }
    return "";
}
#else
void INDIManager::set_prop(const std::string &dev, const std::string &prop, const std::string &element, const std::string &value)
{
    std::stringstream ss;
    ss << "indi_setprop " << dev << "." << prop << "." << element << "=" << value;
    std::string cmd = ss.str();
    system(cmd.c_str());
}

std::string INDIManager::get_prop(const std::string &dev, const std::string &prop, const std::string &element)
{
    std::stringstream ss;
    ss << "indi_getprop " << dev << "." << prop << "." << element;
    std::string cmd = ss.str();
    std::array<char, 128> buffer;
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result.substr(result.find('=') + 1, result.length()).substr(0, result.length() - 2);
}
#endif

std::string INDIManager::get_state(const std::string &dev, const std::string &prop)
{
    return get_prop(dev, prop, "_STATE");
}

std::map<std::string, std::shared_ptr<INDIDeviceContainer>> INDIManager::get_running_drivers()
{
    return running_drivers;
}

static std::vector<std::map<std::string, std::string>> INDIManager::get_devices()
{
    std::vector<std::map<std::string, std::string>> devices;
    std::string cmd = "indi_getprop *.CONNECTION.CONNECT";
    std::array<char, 128> buffer;
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    std::vector<std::string> lines = {"", ""};
    for (std::string token : result)
    {
        if (token == '\n')
        {
            std::map<std::string, std::string> device;
            std::stringstream ss(lines[0]);
            std::string item;
            while (getline(ss, item, '.'))
            {
                device["device"] = item;
            }
            device["connected"] = (lines[1] == "On") ? "true" : "false";
            devices.push_back(device);
            lines = {"", ""};
        }
        else if (token == '=')
        {
            lines[1] = lines[1].substr(0, lines[1].length() - 1);
        }
        else if (token == ' ')
        {
            continue;
        }
        else
        {
            lines[(lines[0] == "") ? 0 : 1] += token;
        }
    }
    return devices;
}