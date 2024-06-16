#ifndef ATOM_SYSTEM_SOFTWARE_HPP
#define ATOM_SYSTEM_SOFTWARE_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace atom::system {
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
bool checkSoftwareInstalled(const std::string& software_name);

/**
 * @brief Get the version of the specified application.
 * 获取指定应用程序的版本
 *
 * @param app_path The path to the application. 应用程序路径
 * @return The version of the application. 应用程序的版本
 */
std::string getAppVersion(const fs::path& app_path);

/**
 * @brief Get the path to the specified application.
 * 获取指定应用程序的路径
 *
 * @param software_name The name of the software. 软件名称
 * @return The path to the application. 应用程序的路径
 */
fs::path getAppPath(const std::string& software_name);

/**
 * @brief Get the permissions of the specified application.
 * 获取指定应用程序的权限
 *
 * @param app_path The path to the application. 应用程序路径
 * @return The permissions of the application. 应用程序的权限
 */
std::vector<std::string> getAppPermissions(const fs::path& app_path);
}  // namespace atom::system

#endif