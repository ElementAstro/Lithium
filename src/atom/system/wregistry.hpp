/*
 * wregistry.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Some registry functions for Windows

**************************************************/

#ifndef ATOM_SYSTEM_WREGISTRY_HPP
#define ATOM_SYSTEM_WREGISTRY_HPP

#ifdef _WIN32
#include <string_view>
#include <utility>
#include <vector>

// Forward declaration to avoid including windows.h in the header
struct HKEY__;

// Alias for HKEY
using HKEY = HKEY__*;

namespace atom::system {

/**
 * @brief 获取指定注册表键下的所有子键名称。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param subKeys 子键名称的字符串向量。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto getRegistrySubKeys(HKEY hRootKey, std::string_view subKey,
                                      std::vector<std::string>& subKeys)
    -> bool;

/**
 * @brief 获取指定注册表键下的所有值名称和数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param values 名称和数据的字符串对向量。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto getRegistryValues(
    HKEY hRootKey, std::string_view subKey,
    std::vector<std::pair<std::string, std::string>>& values) -> bool;

/**
 * @brief 修改指定注册表键下的指定值的数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param valueName 要修改的值的名称。
 * @param newValue 新的值数据。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto modifyRegistryValue(HKEY hRootKey, std::string_view subKey,
                                       std::string_view valueName,
                                       std::string_view newValue) -> bool;

/**
 * @brief 删除指定注册表键及其所有子键。
 * @param hRootKey 根键句柄。
 * @param subKey 要删除的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto deleteRegistrySubKey(HKEY hRootKey,
                                        std::string_view subKey) -> bool;

/**
 * @brief 删除指定注册表键下的指定值。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param valueName 要删除的值的名称。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto deleteRegistryValue(HKEY hRootKey, std::string_view subKey,
                                       std::string_view valueName) -> bool;

/**
 * @brief 递归枚举指定注册表键下的所有子键和值。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 */
void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey,
                                         std::string_view subKey);

/**
 * @brief 备份指定注册表键及其所有子键和值。
 * @param hRootKey 根键句柄。
 * @param subKey 要备份的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param backupFilePath 备份文件的完整路径。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto backupRegistry(HKEY hRootKey, std::string_view subKey,
                                  std::string_view backupFilePath) -> bool;

/**
 * @brief 在指定注册表键下递归查找包含指定字符串的子键名称。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param searchKey 要查找的字符串。
 */
void findRegistryKey(HKEY hRootKey, std::string_view subKey,
                     std::string_view searchKey);

/**
 * @brief 在指定注册表键下递归查找包含指定字符串的值名称和数据。
 * @param hRootKey 根键句柄。
 * @param subKey 指定键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param searchValue 要查找的字符串。
 */
void findRegistryValue(HKEY hRootKey, std::string_view subKey,
                       std::string_view searchValue);

/**
 * @brief 导出指定注册表键及其所有子键和值为 REG 文件。
 * @param hRootKey 根键句柄。
 * @param subKey 要导出的键的名称，可以包括多个嵌套的键，用反斜杠分隔。
 * @param exportFilePath 导出文件的完整路径。
 * @return true 表示成功，false 表示失败。
 */
[[nodiscard]] auto exportRegistry(HKEY hRootKey, std::string_view subKey,
                                  std::string_view exportFilePath) -> bool;

}  // namespace atom::system

#endif

#endif  // ATOM_SYSTEM_WREGISTRY_HPP
