/*
 * utils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-6-17

Description: Network Utils

**************************************************/

#pragma once

#include <string>
#include <vector>

/**
 * @brief Check whether the device is connected to the internet.
 * 检查设备是否连接到互联网
 *
 * @return true if the device is connected to the internet.
 *         如果设备已连接到互联网，则返回 true
 * @return false if the device is not connected to the internet.
 *         如果设备未连接到互联网，则返回 false
 */
bool IsConnectedToInternet();

/**
 * @brief Get the network status information.
 * 获取网络状态信息
 *
 * @return A vector of strings representing the network status.
 *         表示网络状态的字符串向量
 */
std::vector<std::string> GetNetworkStatus();

/**
 * @brief Check if there is any program running on the specified port and kill it if found.
 * 检查指定端口上是否有程序正在运行，如果找到则终止该程序。
 *
 * This function checks if there is any program running on the specified port by querying the system.
 * If a program is found, it will be terminated.
 *
 * @param port The port number to check. 要检查的端口号。
 * @return `true` if a program was found and terminated, `false` otherwise. 如果找到并终止了程序，则返回true；否则返回false。
 */
bool CheckAndKillProgramOnPort(int port);

bool isIPv6Format(const std::string& str);

bool isIPv4Format(const std::string& str);