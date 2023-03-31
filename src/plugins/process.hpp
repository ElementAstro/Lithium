/*
 * process.hpp
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

#include <map>
#include <mutex>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <functional>

namespace OpenAPT {

    class ProcessManager {
        public:
            /**
             * @brief 构造函数
             */
            ProcessManager() : m_stop(false), maxProcesses(10) {}

            /**
             * @brief 析构函数，停止所有进程以及销毁日志记录器
             */
            ~ProcessManager();

            /**
             * @brief 启动一个子进程并运行指定的函数
             * @param name 子进程的名称
             * @param func 子进程要执行的函数
             */
            void startChildProcess(const std::string& name, std::function<void()> func);

            /**
             * @brief 启动一个独立进程并运行指定的命令
             * @param name 进程的名称
             * @param command 要执行的命令
             * @param args 命令参数
             */
            void startIndependentProcess(const std::string& name, const std::string& command, const std::vector<std::string>& args);

            /**
             * @brief 终止指定的子进程
             * @param name 要终止的子进程名称
             */
            void killChildProcess(const std::string& name);

            /**
             * @brief 终止指定的独立进程
             * @param name 要终止的进程名称
             */
            void killIndependentProcess(const std::string& name);

            /**
             * @brief 停止所有进程
             */
            void stopAllProcesses();

            /**
             * @brief 列出所有正在运行的进程名称
             * @return 所有正在运行的进程名称列表
             */
            std::vector<std::string> listProcesses();

            /**
             * @brief 判断指定的进程是否正在运行
             * @param name 要判断的进程名称
             * @return 进程是否正在运行
             */
            bool isProcessRunning(const std::string& name);

        private:
            std::map<std::string, std::unique_ptr<std::thread>> m_processes; /**< 记录所有进程的线程 */
            std::map<std::string, bool> m_processStatus; /**< 记录所有进程的运行状态 */
            std::mutex m_mutex; /**< 用于保护进程列表和状态的互斥锁 */
            bool m_stop; /**< 是否停止所有进程 */
            size_t maxProcesses; /**< 最大进程数量限制 */
        };
}
