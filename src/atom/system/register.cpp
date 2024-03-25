/*
 * register.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Some registry functions for Windows

**************************************************/

#ifdef _WIN32

#include "register.hpp"

#include "atom/log/loguru.hpp"

bool getRegistrySubKeys(HKEY hRootKey, const std::string &subKey,
                        std::vector<std::string> &subKeys) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (lRes == ERROR_SUCCESS) {
            subKeys.push_back(achKey);
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

bool getRegistryValues(
    HKEY hRootKey, const std::string &subKey,
    std::vector<std::pair<std::string, std::string>> &values) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    const DWORD MAX_VALUE_NAME = 16383;
    char achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cbData;
    DWORD dwType;
    TCHAR lpData[MAX_PATH];
    DWORD dwDataSize = sizeof(lpData);

    DWORD i = 0;
    while (true) {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, NULL, &dwType,
                            (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (lRes == ERROR_SUCCESS) {
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

            values.push_back(std::make_pair(valueName, valueData));
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

bool modifyRegistryValue(HKEY hRootKey, const std::string &subKey,
                         const std::string &valueName,
                         const std::string &newValue) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    DWORD dwType = REG_SZ;
    const BYTE *data = reinterpret_cast<const BYTE *>(newValue.c_str());
    DWORD dataSize = static_cast<DWORD>(newValue.length());

    lRes = RegSetValueEx(hKey, valueName.c_str(), 0, dwType, data, dataSize);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not set value type: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool deleteRegistrySubKey(HKEY hRootKey, const std::string &subKey) {
    LONG lRes = RegDeleteKey(hRootKey, subKey.c_str());
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not delete subkey: {}", lRes);
        return false;
    }

    return true;
}

bool deleteRegistryValue(HKEY hRootKey, const std::string &subKey,
                         const std::string &valueName) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    lRes = RegDeleteValue(hKey, valueName.c_str());
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not delete value: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

void recursivelyEnumerateRegistrySubKeys(HKEY hRootKey,
                                         const std::string &subKey) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (lRes == ERROR_SUCCESS) {
            DLOG_F(INFO, "Sub Key: {}", achKey);
            std::string newSubKey = subKey + "\\" + achKey;
            RecursivelyEnumerateRegistrySubKeys(hRootKey, newSubKey);
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

bool backupRegistry(HKEY hRootKey, const std::string &subKey,
                    const std::string &backupFilePath) {
    LONG lRes = RegSaveKey(hRootKey, subKey.c_str(), NULL);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        return false;
    }

    bool success = CopyFileA("NTUSER.DAT", backupFilePath.c_str(), FALSE);
    if (!success) {
        LOG_F(ERROR, "Could not create backup file");
        return false;
    }

    return true;
}

void findRegistryKey(HKEY hRootKey, const std::string &subKey,
                     const std::string &searchKey) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true) {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (lRes == ERROR_SUCCESS) {
            if (achKey == searchKey) {
                DLOG_F(INFO, "Found key: {}", achKey);
            }
            std::string newSubKey = subKey + "\\" + achKey;
            FindRegistryKey(hRootKey, newSubKey, searchKey);
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

void findRegistryValue(HKEY hRootKey, const std::string &subKey,
                       const std::string &searchValue) {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_VALUE_NAME = 16383;
    char achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    DWORD cbData;
    DWORD dwType;
    TCHAR lpData[MAX_PATH];
    DWORD dwDataSize = sizeof(lpData);

    DWORD i = 0;
    while (true) {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, NULL, &dwType,
                            (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (lRes == ERROR_SUCCESS) {
            if (achValue == searchValue) {
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

bool exportRegistry(HKEY hRootKey, const std::string &subKey,
                    const std::string &exportFilePath) {
    LONG lRes = RegSaveKey(hRootKey, subKey.c_str(), NULL);
    if (lRes != ERROR_SUCCESS) {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        return false;
    }

    bool success = CopyFileA("NTUSER.DAT", exportFilePath.c_str(), FALSE);
    if (!success) {
        LOG_F(ERROR, "Could not create export file");
        return false;
    }

    return true;
}

#endif
