/*
 * terminal.hpp
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

Date: 2023-6-30

Description: Terminal

**************************************************/

#pragma once

#define HAS_Lithium_TERMINAL 1

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <vector>
#include <future>
#include <thread>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

namespace Lithium::Terminal
{
    /**
     * @brief 命令管理器类，用于注册、运行指令函数以及管理历史记录等功能。
     */
    class CommandManager
    {
    public:
        /**
         * @brief 构造函数，初始化命令管理器。
         */
        CommandManager();

        /**
         * @brief 注册指令函数。
         * @param cmd 指令名称。
         * @param func 指令函数。
         */
        void registerCommand(const std::string &cmd, std::function<std::string(const std::string &)> func);

        /**
         * @brief 运行指令函数，同步调用。
         * @param cmd 指令名称。
         * @param arg 指令参数。
         * @return 指令函数的返回值。
         */
        const std::string &runCommand(const std::string &cmd, const std::string &arg);

        /**
         * @brief 运行指令函数，异步调用。
         * @param cmd 指令名称。
         * @param arg 指令参数。
         * @return std::future 对象，可用于获取异步调用的返回值。
         */
        std::future<std::string> runCommandAsync(const std::string &cmd, const std::string &arg);

        /**
         * @brief 添加 std::future 对象到 futures_ 中。
         * @param future std::future 对象。
         */
        void addFuture(std::future<std::string> &&future);

        /**
         * @brief 等待所有异步调用结束。
         */
        void join();

        /**
         * @brief 获取注册的指令函数名称列表。
         * @return 指令函数名称列表。
         */
        const std::vector<std::string> &getRegisteredCommands() const;

        /**
         * @brief 获取历史记录中上一个命令。
         * @return 上一个命令。
         */
        const std::string &getPrevCommand();

        /**
         * @brief 获取历史记录中下一个命令。
         * @return 下一个命令。
         */
        const std::string &getNextCommand();

        /**
         * @brief 添加命令到历史记录中。
         * @param cmd 命令。
         */
        void addCommandHistory(const std::string &cmd);

        /**
         * @brief 检查是否存在下一个历史命令。
         * @return 若存在下一个历史命令则返回 true，否则返回 false。
         */
        bool hasNextCommand() const;

        /**
         * @brief 检查是否存在上一个历史命令。
         * @return 若存在上一个历史命令则返回 true，否则返回 false。
         */
        bool hasPrevCommand() const;

    private:
        std::map<std::string, std::function<std::string(const std::string &)>> commands_; // 注册的指令函数
        std::mutex mutex_;                                                                // 互斥锁，用于保护 commands_ 和 futures_
        std::vector<std::future<std::string>> futures_;                                   // 异步调用的 std::future 对象

        std::vector<std::string> command_history_;           // 历史命令
        int history_index_ = 0;                              // 当前历史命令的索引位置
        std::vector<std::string>::const_iterator hist_iter_; // 历史命令遍历迭代器
    };

    /**
     * @brief 获取光标位置。
     * @return 光标位置字符串。
     */
    std::string getCursorLocation();

    /**
     * @brief 检查终端是否支持彩色输出。
     * @return 若支持彩色输出则返回 true，否则返回 false。
     */
    bool isColorSupported();

    /**
     * @brief 获取终端输入。
     * @param manager 命令管理器。
     * @return 终端输入字符串。
     */
    std::string getTerminalInput(CommandManager &manager);

    /**
     * @brief ls 指令的实现，用于显示当前目录下的文件和文件夹。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string lsCommand(const std::string &arg);

    /**
     * @brief pwd 指令的实现，用于显示当前工作目录路径名。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string pwdCommand(const std::string &arg);

    /**
     * @brief 打印终端头部信息。
     */
    void printHeader();

    /**
     * @brief mkdir 指令的实现，用于创建目录。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string mkdirCommand(const std::string &arg);

    /**
     * @brief cp 指令的实现，用于复制文件或目录。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string cpCommand(const std::string &arg);

    /**
     * @brief help 指令的实现，列出所有可用的命令。
     * @param manager 命令管理器。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string helpCommand(CommandManager &manager, const std::string &arg);

    /**
     * @brief system 指令的实现，用于执行系统命令。
     * @param arg 指令参数。
     * @return 执行结果字符串。
     */
    std::string systemCommand(const std::string &arg);
}
