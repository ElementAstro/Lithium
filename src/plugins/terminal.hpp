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

Date: 2023-4-8

Description: Terminal

**************************************************/

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <vector>
#include <future>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <thread>
#include <iomanip>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

namespace OpenAPT::Terminal
{
    class CommandManager
    {
    public:
        CommandManager();

        // 注册指令函数
        void registerCommand(const std::string &cmd, std::function<std::string(const std::string &)> func);

        // 运行指令函数，同步调用
        const std::string &runCommand(const std::string &cmd, const std::string &arg);

        // 运行指令函数，异步调用
        std::future<std::string> runCommandAsync(const std::string &cmd, const std::string &arg);

        // 添加 std::future 对象到 futures_ 中
        void addFuture(std::future<std::string> &&future);

        // 等待所有异步调用结束
        void join();

        // 获取注册的指令函数名称列表
        const std::vector<std::string> &getRegisteredCommands() const;

        // 获取历史记录中上一个命令
        const std::string &getPrevCommand();

        // 获取历史记录中下一个命令
        const std::string &getNextCommand();

        // 添加命令到历史记录中
        void addCommandHistory(const std::string &cmd);

        // 检查是否存在下一个历史命令
        bool hasNextCommand() const;

        // 检查是否存在上一个历史命令
        bool hasPrevCommand() const;

    private:
        std::map<std::string, std::function<std::string(const std::string &)>> commands_;
        std::mutex mutex_;
        std::vector<std::future<std::string>> futures_;

        std::vector<std::string> command_history_;           // 历史命令
        int history_index_ = 0;                              // 当前历史命令的索引位置
        std::vector<std::string>::const_iterator hist_iter_; // 历史命令遍历迭代器
    };

    std::string getCursorLocation();

    bool isColorSupported();

    std::string getTerminalInput(CommandManager &manager);

    // 实现 ls 指令，用于显示当前目录下的文件和文件夹
    std::string lsCommand(const std::string &arg);

    // 实现 pwd 指令，用于显示当前工作目录路径名
    std::string pwdCommand(const std::string &arg);

    void printHeader();

    // 实现 mkdir 指令，用于创建目录
    std::string mkdirCommand(const std::string &arg);

    // 实现 cp 指令，用于复制文件或目录
    std::string cpCommand(const std::string &arg);

    // 实现 help 指令，列出所有可用的命令
    std::string helpCommand(CommandManager &manager, const std::string &arg);

    std::string systemCommand(const std::string &arg);
}
