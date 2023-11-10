/*
 * pid.cpp
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

Date: 2023-4-4

Description: PID Watcher

**************************************************/

#include "pid.hpp"
#include "config.h"

#include <chrono>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#endif

#include "loguru/loguru.hpp"

#ifdef _WIN32
std::wstring charToWchar(const CHAR *str)
{
    int length = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
    std::wstring wideStr(length, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str, -1, &wideStr[0], length);
    return wideStr;
}
#endif

PIDWatcher::PIDWatcher(const std::string &processName)
    : processName_(processName), isRunning_(false), shouldStop_(false)
{
}

PIDWatcher::~PIDWatcher()
{
    stop();
}

void PIDWatcher::start()
{
    if (!isRunning_.exchange(true))
    {
        thread_ = std::jthread(std::bind(&PIDWatcher::watch, this));
    }
}

void PIDWatcher::stop()
{
    if (isRunning_.exchange(false))
    {
        shouldStop_ = true;
        thread_.join();
    }
}

void PIDWatcher::watch()
{
    while (isRunning_ && !shouldStop_)
    {
#ifdef _WIN32
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            LOG_F(ERROR, _("CreateToolhelp32Snapshot failed."));
            return;
        }

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &entry))
        {
            do
            {
#ifdef _WIN32
                if (_wcsicmp(charToWchar(entry.szExeFile).c_str(), getWideString(processName_).c_str()) == 0)
#else
                if (strcasecmp(entry.szExeFile, processName_.c_str()) == 0)
#endif

                {
                    DWORD pid = entry.th32ProcessID;
                    DLOG_F(INFO, _("Watching process with PID: {}"), pid);

                    HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, pid);
                    if (process != NULL)
                    {
                        DWORD waitResult = WaitForSingleObject(process, 0);
                        if (waitResult == WAIT_OBJECT_0)
                        {
                            DWORD exitCode;
                            if (GetExitCodeProcess(process, &exitCode))
                            {
                                DLOG_F(INFO, _("Process exited with code: {}"), exitCode);
                                // 触发回调函数
                                if (callback_)
                                {
                                    callback_(pid, exitCode);
                                }
                            }
                            else
                            {
                                LOG_F(ERROR, _("GetExitCodeProcess failed."));
                            }
                            CloseHandle(process);
                            stop();
                            return;
                        }
                        CloseHandle(process);
                    }
                    else
                    {
                        LOG_F(ERROR, _("OpenProcess failed."));
                    }

                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);

        std::this_thread::sleep_for(std::chrono::seconds(1));
#else
        FILE *fp = popen(("pgrep " + processName_).c_str(), "r");
        if (fp != nullptr)
        {
            char buffer[16];
            memset(buffer, 0, sizeof(buffer));
            if (fgets(buffer, sizeof(buffer), fp) != nullptr)
            {
                int pid = std::stoi(buffer);
                DLOG_F(INFO, "Watching process with PID: {}", pid);

                int status;
                if (waitpid(pid, &status, WNOHANG) != 0)
                {
                    if (WIFEXITED(status))
                    {
                        DLOG_F(INFO, _("Process exited with status: {}"), WEXITSTATUS(status));
                        // 触发回调函数
                        if (callback_)
                        {
                            callback_(pid, WEXITSTATUS(status));
                        }
                    }
                    else if (WIFSIGNALED(status))
                    {
                        DLOG_F(INFO, _("Process terminated by signal: {}"), WTERMSIG(status));
                    }
                    stop();
                    return;
                }
            }
            pclose(fp);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
#endif
    }
}

void PIDWatcher::setCallback(const std::function<void(int, int)> &callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    callback_ = callback;
}

std::wstring PIDWatcher::getWideString(const std::string &str)
{
#ifdef _WIN32
    std::wstring wideBuffer(256, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideBuffer.data(), static_cast<int>(wideBuffer.size()));
    wideBuffer.resize(wcslen(wideBuffer.c_str()));
    return wideBuffer;
#else
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
#endif
}

PIDWatcherManager::~PIDWatcherManager()
{
    stopAll();
}

void PIDWatcherManager::addWatcher(const std::string &processName)
{
    watchers_.emplace_back(std::make_shared<PIDWatcher>(processName));
}

void PIDWatcherManager::startAll()
{
    for (auto &watcher : watchers_)
    {
        watcher->start();
    }
}

void PIDWatcherManager::stopAll()
{
    for (auto &watcher : watchers_)
    {
        watcher->stop();
    }
}

void PIDWatcherManager::setCallbackForAll(const std::function<void(int, int)> &callback)
{
    for (auto &watcher : watchers_)
    {
        watcher->setCallback(callback);
    }
}

/*

    PIDWatcherManager manager;

    // 创建PIDWatcher并添加到管理器中
    manager.addWatcher("msedge.exe");

    // 设置回调函数
    manager.setCallbackForAll([](int pid, int exitCode)
                              { DLOG_F(INFO, "Callback: Process with PID {} exited with code {}", pid, exitCode); });

    // 开始监视所有进程
    manager.startAll();

    // 停止监视
    std::string input;
    std::getline(std::cin, input);
    manager.stopAll();
*/