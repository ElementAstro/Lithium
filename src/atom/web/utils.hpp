/*
 * utils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Network Utils

**************************************************/

#ifndef ATOM_WEB_UTILS_HPP
#define ATOM_WEB_UTILS_HPP
namespace atom::web {
/**
 * @brief Check if there is any program running on the specified port and kill
 * it if found. 检查指定端口上是否有程序正在运行，如果找到则终止该程序。
 *
 * This function checks if there is any program running on the specified port by
 * querying the system. If a program is found, it will be terminated.
 *
 * @param port The port number to check. 要检查的端口号。
 * @return `true` if a program was found and terminated, `false` otherwise.
 * 如果找到并终止了程序，则返回true；否则返回false。
 */
auto checkAndKillProgramOnPort(int port) -> bool;
}  // namespace atom::web

#endif
