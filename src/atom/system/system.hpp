/*
 * system.hpp
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

Description: System

**************************************************/

#pragma once

#include <string>
#include <vector>

namespace Atom::System
{
    /**
     * @brief Check whether the specified software is installed.
     * 检查指定软件是否已安装
     *
     * @param software_name The name of the software. 软件名称
     * @return true if the software is installed.
     *         如果软件已安装，则返回 true
     * @return false if the software is not installed or an error occurred.
     *         如果软件未安装或发生错误，则返回 false
     */
    bool CheckSoftwareInstalled(const std::string &software_name);

    /**
     * @brief Get the CPU usage percentage.
     * 获取 CPU 使用率百分比
     *
     * @return The CPU usage percentage.
     *         CPU 使用率百分比
     */
    float GetCpuUsage();

    /**
     * @brief Get the CPU temperature.
     * 获取 CPU 温度
     *
     * @return The CPU temperature in degrees Celsius.
     *         CPU 温度（摄氏度）
     */
    float GetCpuTemperature();

    /**
     * @brief Get the memory usage percentage.
     * 获取内存使用率百分比
     *
     * @return The memory usage percentage.
     *         内存使用率百分比
     */
    float GetMemoryUsage();

    /**
     * @brief Get the disk usage for all disks.
     * 获取所有磁盘的使用情况
     *
     * @return A vector of pairs containing the disk name and its usage percentage.
     *         包含磁盘名称和使用率百分比的一对对的向量
     */
    std::vector<std::pair<std::string, float>> GetDiskUsage();

    /**
     * @brief Check whether the current user has root/administrator privileges.
     * 检查当前用户是否具有根/管理员权限
     *
     * @return true if the current user has root/administrator privileges.
     *         如果当前用户具有根/管理员权限，则返回 true
     * @return false if the current user does not have root/administrator privileges.
     *         如果当前用户没有根/管理员权限，则返回 false
     */
    bool IsRoot();

    /**
     * @brief Get the current user name.
     * 获取当前用户名
     *
     * @return The current user name.
     *         当前用户名
     */
    std::string GetCurrentUsername();

    // Max: Surely, we will not recieve any signal after we call this function.

    /**
     * @brief Shutdown the system.
     * 关闭系统
     *
     * @return true if the system is successfully shutdown.
     *         如果系统成功关闭，则返回 true
     * @return false if an error occurred.
     *         如果发生错误，则返回 false
     */
    bool Shutdown();

    /**
     * @brief Reboot the system.
     * 重启系统
     *
     * @return true if the system is successfully rebooted.
     *         如果系统成功重启，则返回 true
     * @return false if an error occurred.
     *         如果发生错误，则返回 false
     */
    bool Reboot();

    /**
     * @brief Get the process information and file address.
     * 获取进程信息以及对应文件地址
     *
     * @return A vector of pairs containing the process information (name and process ID) and its file address.
     *         包含进程信息（名称和进程ID）以及对应文件地址的一对对的向量
     */
    std::vector<std::pair<std::string, std::string>> GetProcessInfo();

    /**
     * @brief Check if there are duplicate processes with the same program name.
     * 检查是否存在同一程序名的重复进程。
     *
     * This function checks if there are multiple instances of a process running with the same program name.
     * It compares the program name of each running process with the specified program name.
     *
     * @param program_name The name of the program to check for duplicates. 要检查重复项的程序名称。
     */
    void CheckDuplicateProcess(const std::string &program_name);

    /**
     * @brief Check if the specified process is running.
     * 检查指定的进程是否正在运行
     *
     * @param processName The name of the process to check. 要检查的进程名称。
     * @return true if the process is running.
     *         如果进程正在运行，则返回 true
     * @return false if the process is not running.
     *         如果进程没有运行，则返回 false
     */
    bool isProcessRunning(const std::string &processName);
}
