/*
 * register.hpp
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

Description: Some registry functions for Windows

**************************************************/

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <string>
#include <vector>

/**
 * @brief 获取指定注册表键下的所有子键名称。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param subKeys 子键名称的字符串向量。
 * @return true 表示成功，false 表示失败。
 */
bool GetRegistrySubKeys(HKEY hRootKey, const std::string &subKey, std::vector<std::string> &subKeys);

/**
 * @brief 获取指定注册表键下的所有值名称和数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param values 名称和数据的字符串对向量。
 * @return true 表示成功，false 表示失败。
 */
bool GetRegistryValues(HKEY hRootKey, const std::string &subKey, std::vector<std::pair<std::string, std::string>> &values);

/**
 * @brief 修改指定注册表键下的指定值的数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param valueName 要修改的值的名称。
 * @param newValue 新的值数据。
 * @return true 表示成功，false 表示失败。
 */
bool ModifyRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &valueName, const std::string &newValue);

/**
 * @brief 删除指定注册表键及其所有子键。
 * @param hRootKey 根键句柄。
 * @param subKey 要删除的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @return true 表示成功，false 表示失败。
 */
bool DeleteRegistrySubKey(HKEY hRootKey, const std::string &subKey);

/**
 * @brief 删除指定注册表键下的指定值。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param valueName 要删除的值的名称。
 * @return true 表示成功，false 表示失败。
 */
bool DeleteRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &valueName);

/**
 * @brief 递归枚举指定注册表键下的所有子键和值。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 */
void RecursivelyEnumerateRegistrySubKeys(HKEY hRootKey, const std::string &subKey);

/**
 * @brief 备份指定注册表键及其所有子键和值。
 * @param hRootKey 根键句柄。
 * @param subKey 要备份的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param backupFilePath 备份文件的完整路径。
 * @return true 表示成功，false 表示失败。
 */
bool BackupRegistry(HKEY hRootKey, const std::string &subKey, const std::string &backupFilePath);

/**
 * @brief 在指定注册表键下递归查找包含指定字符串的子键名称。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param searchKey 要查找的字符串。
 */
void FindRegistryKey(HKEY hRootKey, const std::string &subKey, const std::string &searchKey);

/**
 * @brief 在指定注册表键下递归查找包含指定字符串的值名称和数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param searchValue 要查找的字符串。
 */
void FindRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &searchValue);

/**
 * @brief 导出指定注册表键及其所有子键和值为 REG 文件。
 * @param hRootKey 根键句柄。
 * @param subKey 要导出的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param exportFilePath 导出文件的完整路径。
 * @return true 表示成功，false 表示失败。
 */
bool ExportRegistry(HKEY hRootKey, const std::string &subKey, const std::string &exportFilePath);


#endif