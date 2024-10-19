/*
 * wregistry.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Some registry functions for Windows

**************************************************/

#ifdef _WIN32

#include "wregistry.hpp"

#include <array>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <windows.h>

#include "atom/log/loguru.hpp"

namespace atom::system {
constexpr DWORD MAX_KEY_LENGTH = 255;
constexpr DWORD MAX_VALUE_NAME = 16383;
constexpr DWORD MAX_PATH_LENGTH = MAX_PATH;

auto getRegistrySubKeys(HKEY hRootKey, std::string_view subKey,
                        std::vector<std::string> &subKeys) -> bool {
    LOG_F(INFO, "getRegistrySubKeys called with hRootKey: {}, subKey: {}",
          reinterpret_cast<void *>(hRootKey), subKey);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    std::array<char, MAX_KEY_LENGTH> achKey;
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD index = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, index, achKey.data(), &cchKey, nullptr,
                            nullptr, nullptr, nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            subKeys.emplace_back(achKey.data());
            cchKey = MAX_KEY_LENGTH;
            index++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "getRegistrySubKeys completed with {} subKeys found",
          subKeys.size());
    return true;
}

auto getRegistryValues(HKEY hRootKey, std::string_view subKey,
                       std::vector<std::pair<std::string, std::string>> &values)
    -> bool {
    LOG_F(INFO, "getRegistryValues called with hRootKey: {}, subKey: {}",
          reinterpret_cast<void *>(hRootKey), subKey);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    std::array<char, MAX_VALUE_NAME> achValue;
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD dwType;
    std::array<TCHAR, MAX_PATH_LENGTH> lpData;
    DWORD dwDataSize = sizeof(lpData);

    DWORD index = 0;
    while (true) {
        lRes = RegEnumValue(hKey, index, achValue.data(), &cchValue, nullptr,
                            &dwType, reinterpret_cast<LPBYTE>(lpData.data()),
                            &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            std::string valueName = achValue.data();
            std::string valueData;
            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
                valueData = lpData.data();
            } else if (dwType == REG_DWORD) {
                DWORD data = *reinterpret_cast<DWORD *>(lpData.data());
                valueData = std::to_string(data);
            } else {
                valueData = "<unsupported type>";
            }

            values.emplace_back(valueName, valueData);
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            index++;
        } else {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "getRegistryValues completed with {} values found",
          values.size());
    return true;
}

auto modifyRegistryValue(HKEY hRootKey, std::string_view subKey,
                         std::string_view valueName,
                         std::string_view newValue) -> bool {
    LOG_F(INFO,
          "modifyRegistryValue called with hRootKey: {}, subKey: {}, "
          "valueName: {}, newValue: {}",
          reinterpret_cast<void *>(hRootKey), subKey, valueName, newValue);
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0,
                             KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    DWORD dwType = REG_SZ;
    const BYTE *data = reinterpret_cast<const BYTE *>(newValue.data());
    auto dataSize =
        static_cast<DWORD>(newValue.size() + 1);  // +1 for null terminator

    lRes = RegSetValueEx(hKey, std::string(valueName).c_str(), 0, dwType, data,
                         dataSize);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not set value type: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "modifyRegistryValue completed successfully");
    return true;
}

auto deleteRegistrySubKey(HKEY hRootKey, std::string_view subKey) -> bool {
    LOG_F(INFO, "deleteRegistrySubKey called with hRootKey: {}, subKey: {}",
          reinterpret_cast<void *>(hRootKey), subKey);
    LONG lRes = RegDeleteKey(hRootKey, std::string(subKey).c_str());
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not delete subkey: {}", lRes);
        return false;
    }

    LOG_F(INFO, "deleteRegistrySubKey completed successfully");
    return true;
}

auto deleteRegistryValue(HKEY hRootKey, std::string_view subKey,
                         std::string_view valueName) -> bool {
    LOG_F(INFO,
          "deleteRegistryValue called with hRootKey: {}, subKey: {}, "
          "valueName: {}",
          reinterpret_cast<void *>(hRootKey), subKey, valueName);
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0,
                             KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    lRes = RegDeleteValue(hKey, std::string(valueName).c_str());
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not delete value: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "deleteRegistryValue completed successfully");
    return true;
}

void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey,
                                         std::string_view subKey) {
    LOG_F(INFO,
          "recursivelyEnumerateRegistrySubKeys called with hRootKey: {}, "
          "subKey: {}",
          reinterpret_cast<void *>(hRootKey), subKey);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    std::array<char, MAX_KEY_LENGTH> achKey;
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD index = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, index, achKey.data(), &cchKey, nullptr,
                            nullptr, nullptr, nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            DLOG_F(INFO, "Sub Key: {}", achKey.data());
            std::string newSubKey =
                std::format("{}\\{}", subKey, achKey.data());
            recursivelyEnumerateRegistrySubKeys(hRootKey, newSubKey);
            cchKey = MAX_KEY_LENGTH;
            index++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "recursivelyEnumerateRegistrySubKeys completed");
}

auto backupRegistry(HKEY hRootKey, std::string_view subKey,
                    std::string_view backupFilePath) -> bool {
    LOG_F(INFO,
          "backupRegistry called with hRootKey: {}, subKey: {}, "
          "backupFilePath: {}",
          reinterpret_cast<void *>(hRootKey), subKey, backupFilePath);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    lRes = RegSaveKey(hKey, std::string(backupFilePath).c_str(), nullptr);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "backupRegistry completed successfully");
    return true;
}

void findRegistryKey(HKEY hRootKey, std::string_view subKey,
                     std::string_view searchKey) {
    LOG_F(INFO,
          "findRegistryKey called with hRootKey: {}, subKey: {}, searchKey: {}",
          reinterpret_cast<void *>(hRootKey), subKey, searchKey);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    std::array<char, MAX_KEY_LENGTH> achKey;
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD index = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, index, achKey.data(), &cchKey, nullptr,
                            nullptr, nullptr, nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            if (std::string_view(achKey.data()) == searchKey) {
                DLOG_F(INFO, "Found key: {}", achKey.data());
            }
            std::string newSubKey =
                std::format("{}\\{}", subKey, achKey.data());
            findRegistryKey(hRootKey, newSubKey, searchKey);
            cchKey = MAX_KEY_LENGTH;
            index++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "findRegistryKey completed");
}

void findRegistryValue(HKEY hRootKey, std::string_view subKey,
                       std::string_view searchValue) {
    LOG_F(INFO,
          "findRegistryValue called with hRootKey: {}, subKey: {}, "
          "searchValue: {}",
          reinterpret_cast<void *>(hRootKey), subKey, searchValue);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    std::array<char, MAX_VALUE_NAME> achValue;
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD dwType;
    std::array<TCHAR, MAX_PATH_LENGTH> lpData;
    DWORD dwDataSize = sizeof(lpData);

    DWORD index = 0;
    while (true) {
        lRes = RegEnumValue(hKey, index, achValue.data(), &cchValue, nullptr,
                            &dwType, reinterpret_cast<LPBYTE>(lpData.data()),
                            &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            if (std::string_view(achValue.data()) == searchValue) {
                LOG_F(INFO, "Found value: {}", achValue.data());
            }
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            index++;
        } else {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "findRegistryValue completed");
}

auto exportRegistry(HKEY hRootKey, std::string_view subKey,
                    std::string_view exportFilePath) -> bool {
    LOG_F(INFO,
          "exportRegistry called with hRootKey: {}, subKey: {}, "
          "exportFilePath: {}",
          reinterpret_cast<void *>(hRootKey), subKey, exportFilePath);
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    lRes = RegSaveKey(hKey, std::string(exportFilePath).c_str(), nullptr);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    LOG_F(INFO, "exportRegistry completed successfully");
    return true;
}
}  // namespace atom::system

#endif