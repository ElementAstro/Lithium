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

#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <windows.h>

#include "atom/log/loguru.hpp"

namespace atom::system {
auto getRegistrySubKeys(HKEY hRootKey, std::string_view subKey,
                        std::vector<std::string> &subKeys) -> bool {
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, nullptr, nullptr, nullptr,
                            nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            subKeys.emplace_back(achKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}

auto getRegistryValues(HKEY hRootKey, std::string_view subKey,
                       std::vector<std::pair<std::string, std::string>> &values)
    -> bool {
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    const DWORD MAX_VALUE_NAME = 16383;
    char achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD dwType;
    TCHAR lpData[MAX_PATH];
    DWORD dwDataSize = sizeof(lpData);

    DWORD i = 0;
    while (true) {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, nullptr, &dwType,
                            (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            std::string valueName = achValue;
            std::string valueData;
            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
                valueData = lpData;
            } else if (dwType == REG_DWORD) {
                DWORD data = *(DWORD *)lpData;
                valueData = std::to_string(data);
            } else {
                valueData = "<unsupported type>";
            }

            values.emplace_back(valueName, valueData);
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            i++;
        } else {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}

auto modifyRegistryValue(HKEY hRootKey, std::string_view subKey,
                         std::string_view valueName,
                         std::string_view newValue) -> bool {
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
    return true;
}

auto deleteRegistrySubKey(HKEY hRootKey, std::string_view subKey) -> bool {
    LONG lRes = RegDeleteKey(hRootKey, std::string(subKey).c_str());
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not delete subkey: {}", lRes);
        return false;
    }

    return true;
}

auto deleteRegistryValue(HKEY hRootKey, std::string_view subKey,
                         std::string_view valueName) -> bool {
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
    return true;
}

void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey,
                                         std::string_view subKey) {
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, nullptr, nullptr, nullptr,
                            nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            DLOG_F(INFO, "Sub Key: {}", achKey);
            std::string newSubKey = std::format("{}\\{}", subKey, achKey);
            recursivelyEnumerateRegistrySubKeys(hRootKey, newSubKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

auto backupRegistry(HKEY hRootKey, std::string_view subKey,
                    std::string_view backupFilePath) -> bool {
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
    return true;
}

void findRegistryKey(HKEY hRootKey, std::string_view subKey,
                     std::string_view searchKey) {
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, nullptr, nullptr, nullptr,
                            nullptr);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            if (std::string_view(achKey) == searchKey) {
                DLOG_F(INFO, "Found key: {}", achKey);
            }
            std::string newSubKey = std::format("{}\\{}", subKey, achKey);
            findRegistryKey(hRootKey, newSubKey, searchKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        } else {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

void findRegistryValue(HKEY hRootKey, std::string_view subKey,
                       std::string_view searchValue) {
    HKEY hKey;
    LONG lRes =
        RegOpenKeyEx(hRootKey, std::string(subKey).c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_VALUE_NAME = 16383;
    char achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD dwType;
    TCHAR lpData[MAX_PATH];
    DWORD dwDataSize = sizeof(lpData);

    DWORD i = 0;
    while (true) {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, nullptr, &dwType,
                            (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (lRes == ERROR_SUCCESS) {
            if (std::string_view(achValue) == searchValue) {
                LOG_F(INFO, "Found value: {}", achValue);
            }
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            i++;
        } else {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

auto exportRegistry(HKEY hRootKey, std::string_view subKey,
                    std::string_view exportFilePath) -> bool {
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
    return true;
}
}  // namespace atom::system

#endif
