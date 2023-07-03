#pragma once

#include <string>
#include <vector>

namespace OpenAPT::System
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
     * @brief Get the network status information.
     * 获取网络状态信息
     *
     * @return A vector of strings representing the network status.
     *         表示网络状态的字符串向量
     */
    std::vector<std::string> GetNetworkStatus();

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

}
