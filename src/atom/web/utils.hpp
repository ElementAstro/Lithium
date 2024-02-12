/*
 * utils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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