/*
 * register.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-17

Description: Some registry functions for Windows

**************************************************/

#ifdef _WIN32

#include "register.hpp"

#include "atom/log/loguru.hpp"

bool GetRegistrySubKeys(HKEY hRootKey, const std::string &subKey, std::vector<std::string> &subKeys)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true)
    {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (lRes == ERROR_SUCCESS)
        {
            subKeys.push_back(achKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        }
        else
        {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}

bool GetRegistryValues(HKEY hRootKey, const std::string &subKey, std::vector<std::pair<std::string, std::string>> &values)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
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
    while (true)
    {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, NULL, &dwType, (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (lRes == ERROR_SUCCESS)
        {
            std::string valueName = achValue;
            std::string valueData;
            if (dwType == REG_SZ || dwType == REG_EXPAND_SZ)
            {
                valueData = lpData;
            }
            else if (dwType == REG_DWORD)
            {
                DWORD data = *(DWORD *)lpData;
                valueData = std::to_string(data);
            }
            else
            {
                valueData = "<unsupported type>";
            }

            values.push_back(std::make_pair(valueName, valueData));
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            i++;
        }
        else
        {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return false;
        }
    }

    RegCloseKey(hKey);
    return true;
}

bool ModifyRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &valueName, const std::string &newValue)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    DWORD dwType = REG_SZ;
    const BYTE *data = reinterpret_cast<const BYTE *>(newValue.c_str());
    DWORD dataSize = static_cast<DWORD>(newValue.length());

    lRes = RegSetValueEx(hKey, valueName.c_str(), 0, dwType, data, dataSize);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not set value type: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool DeleteRegistrySubKey(HKEY hRootKey, const std::string &subKey)
{
    LONG lRes = RegDeleteKey(hRootKey, subKey.c_str());
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not delete subkey: {}", lRes);
        return false;
    }

    return true;
}

bool DeleteRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &valueName)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return false;
    }

    lRes = RegDeleteValue(hKey, valueName.c_str());
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not delete value: {}", lRes);
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

void RecursivelyEnumerateRegistrySubKeys(HKEY hRootKey, const std::string &subKey)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true)
    {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (lRes == ERROR_SUCCESS)
        {
            DLOG_F(INFO, "Sub Key: {}", achKey);
            std::string newSubKey = subKey + "\\" + achKey;
            RecursivelyEnumerateRegistrySubKeys(hRootKey, newSubKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        }
        else
        {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

bool BackupRegistry(HKEY hRootKey, const std::string &subKey, const std::string &backupFilePath)
{
    LONG lRes = RegSaveKey(hRootKey, subKey.c_str(), NULL);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        return false;
    }

    bool success = CopyFileA("NTUSER.DAT", backupFilePath.c_str(), FALSE);
    if (!success)
    {
        LOG_F(ERROR, "Could not create backup file");
        return false;
    }

    return true;
}

void FindRegistryKey(HKEY hRootKey, const std::string &subKey, const std::string &searchKey)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not open key: {}", lRes);
        return;
    }

    const DWORD MAX_KEY_LENGTH = 255;
    char achKey[MAX_KEY_LENGTH];
    DWORD cchKey = MAX_KEY_LENGTH;

    DWORD i = 0;
    while (true)
    {
        lRes = RegEnumKeyEx(hKey, i, achKey, &cchKey, NULL, NULL, NULL, NULL);
        if (lRes == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (lRes == ERROR_SUCCESS)
        {
            if (achKey == searchKey)
            {
                DLOG_F(INFO, "Found key: {}", achKey);
            }
            std::string newSubKey = subKey + "\\" + achKey;
            FindRegistryKey(hRootKey, newSubKey, searchKey);
            cchKey = MAX_KEY_LENGTH;
            i++;
        }
        else
        {
            LOG_F(ERROR, "Could not enum key: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

void FindRegistryValue(HKEY hRootKey, const std::string &subKey, const std::string &searchValue)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS)
    {
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
    while (true)
    {
        lRes = RegEnumValue(hKey, i, achValue, &cchValue, NULL, &dwType, (LPBYTE)lpData, &dwDataSize);
        if (lRes == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (lRes == ERROR_SUCCESS)
        {
            if (achValue == searchValue)
            {
                LOG_F(INFO, "Found value: {}", achValue)
            }
            cchValue = MAX_VALUE_NAME;
            dwDataSize = sizeof(lpData);
            i++;
        }
        else
        {
            LOG_F(ERROR, "Could not enum value: {}", lRes);
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}

bool ExportRegistry(HKEY hRootKey, const std::string &subKey, const std::string &exportFilePath)
{
    LONG lRes = RegSaveKey(hRootKey, subKey.c_str(), NULL);
    if (lRes != ERROR_SUCCESS)
    {
        LOG_F(ERROR, "Could not save key: {}", lRes);
        std::cerr << "Could not save key: " << lRes << std::endl;
        return false;
    }

    bool success = CopyFileA("NTUSER.DAT", exportFilePath.c_str(), FALSE);
    if (!success)
    {
        LOG_F(ERROR, "Could not create export file");
        return false;
    }

    return true;
}

/*
int main()
{
    std::vector<std::string> subKeys;
    if (GetRegistrySubKeys(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", subKeys)) {
        std::cout << "Sub Keys:" << std::endl;
        for (const auto& subKey : subKeys) {
            std::cout << "  " << subKey << std::endl;
        }
    }

    std::vector<std::pair<std::string, std::string>> values;
    if (GetRegistryValues(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", values)) {
        std::cout << "Values:" << std::endl;
        for (const auto& value : values) {
            std::cout << "  " << value.first << " = " << value.second << std::endl;
        }
    }

    return 0;
}
*/

#endif