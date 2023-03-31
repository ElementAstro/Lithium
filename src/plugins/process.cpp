/*
 * process.cpp
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
 
Date: 2023-3-31
 
Description: Process Manager
 
**************************************************/

#include <spdlog/spdlog.h>

#include "process.hpp"

namespace OpenAPT {

    /**
     * @brief 析构函数，停止所有进程以及销毁日志记录器
     */
    ProcessManager::~ProcessManager() {
        stopAllProcesses();
        spdlog::shutdown();
    }

    /**
     * @brief 启动一个子进程并运行指定的函数
     * @param name 子进程的名称
     * @param func 子进程要执行的函数
     */
    void ProcessManager::startChildProcess(const std::string& name, std::function<void()> func) {
        // 判断当前进程数量是否超过最大进程限制
        while (m_processes.size() >= static_cast<unsigned long>(maxProcesses)) {
            spdlog::warn("Process count exceeds limit. Waiting for a process to finish...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        try {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto newThread = std::make_unique<std::thread>(func);
            m_processes[name] = std::move(newThread);
            m_processStatus[name] = true;
        } catch (const std::exception& ex) {
            spdlog::error("Exception caught when starting child process {}: {}", name, ex.what());
        }
    }

    /**
     * @brief 启动一个独立进程并运行指定的命令
     * @param name 进程的名称
     * @param command 要执行的命令
     * @param args 命令参数
     */
    void ProcessManager::startIndependentProcess(const std::string& name, const std::string& command, const std::vector<std::string>& args) {
        // 判断当前进程数量是否超过最大进程限制
        while (m_processes.size() >= static_cast<unsigned long>(maxProcesses)) {
            spdlog::warn("Process count exceeds limit. Waiting for a process to finish...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // 将命令和参数整合成一个字符串
        std::string cmd;
        for (std::vector<std::string>::size_type i = 0; i < args.size(); ++i) {
            cmd += args[i];
            if (i != args.size() - 1) {
                cmd += " ";
            }
        }
        cmd = command + " " + cmd;

        try {
            // 创建新线程并启动新进程
            std::lock_guard<std::mutex> lock(m_mutex);
            auto newThread = std::make_unique<std::thread>([this, name, cmd]() {
                int status = std::system(cmd.c_str());
                spdlog::info("Process {} exited with status {}", name, status);
                std::lock_guard<std::mutex> lock(m_mutex);
                m_processStatus[name] = false;
            });
            m_processes[name] = std::move(newThread);
            m_processStatus[name] = true;
        } catch (const std::exception& ex) {
            spdlog::error("Exception caught when starting independent process {}: {}", name, ex.what());
        }
    }

    /**
     * @brief 终止指定的子进程
     * @param name 要终止的子进程名称
     */
    void ProcessManager::killChildProcess(const std::string& name) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_processes.find(name) == m_processes.end()) {
            spdlog::error("Process {} not found.", name);
            return;
        }
        lock.unlock();
        m_processes[name]->detach();
        m_processes[name].reset();
        lock.lock();
        m_processes.erase(name);
        m_processStatus.erase(name);
        spdlog::info("Child process {} killed.", name);
    }

    /**
     * @brief 终止指定的独立进程
     * @param name 要终止的进程名称
     */
    void ProcessManager::killIndependentProcess(const std::string& name) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_processes.find(name) == m_processes.end()) {
            spdlog::error("Process {} not found.", name);
            return;
        }
        lock.unlock();
        std::string cmd = "killall -9 " + name;
        int status = std::system(cmd.c_str());
        lock.lock();
        m_processStatus[name] = false;
        spdlog::info("Independent process {} killed with status {}", name, status);
    }

    /**
     * @brief 停止所有进程
     */
    void ProcessManager::stopAllProcesses() {
        if (m_processes.empty()) {
            return;
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
        for (auto& process : m_processes) {
            if (process.second && process.second->joinable()) {
                process.second->join();
                process.second.reset();
            }
        }
        m_processes.clear();
        m_processStatus.clear();
        m_stop = false;
    }

    /**
     * @brief 列出所有正在运行的进程名称
     * @return 所有正在运行的进程名称列表
     */
    std::vector<std::string> ProcessManager::listProcesses() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> processList;
        for (const auto& process : m_processes) {
            processList.push_back(process.first);
        }
        return processList;
    }

    /**
     * @brief 判断指定的进程是否正在运行
     * @param name 要判断的进程名称
     * @return 进程是否正在运行
     */
    bool ProcessManager::isProcessRunning(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_processStatus.find(name) == m_processStatus.end()) {
            spdlog::error("Process {} not found.", name);
            return false;
        }
        return m_processStatus[name];
    }
}

/*
int main() {
	// 初始化 spdlog

	// 创建进程管理器
	ProcessManager pm;

	// 启动一个子进程
	pm.startChildProcess("child_process", []() {
		spdlog::info("Child process started.");
		int count = 0;
		while (count < 5) {
			spdlog::info("Child process running... count = {}.", count);
			std::this_thread::sleep_for(std::chrono::seconds(1));
			count++;
		}
	});

	// 启动一个独立进程
	pm.startIndependentProcess("independent_process", "ls", {"-a"});

	// 列出当前正在运行的进程
	auto processList = pm.listProcesses();
	spdlog::debug("Current running processes:");
	for (const auto& process : processList) {
		spdlog::debug("- {}", process);
	}

	// 等待子进程执行完毕
	while (pm.isProcessRunning("child_process")) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

    sleep(10);

	// 终止独立进程

	// 终止所有进程
	pm.stopAllProcesses();

    sleep(10);

	return 0;
}
*/
