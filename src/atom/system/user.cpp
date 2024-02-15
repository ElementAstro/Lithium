/*
 * user.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Some system functions to get user information.

**************************************************/

#include "user.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#include <lmcons.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#endif

#include "atom/log/loguru.hpp"

namespace Atom::System
{
    std::vector<std::wstring> getUserGroups()
    {
        std::vector<std::wstring> groups;

#ifdef _WIN32
        // 获取当前用户的令牌句柄
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            LOG_F(ERROR, "Failed to open process token.");
            return groups;
        }

        // 获取用户组信息
        DWORD bufferSize = 0;
        GetTokenInformation(hToken, TokenGroups, NULL, 0, &bufferSize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            LOG_F(ERROR, "Failed to get token information size.");
            CloseHandle(hToken);
            return groups;
        }

        std::vector<BYTE> buffer(bufferSize);
        if (!GetTokenInformation(hToken, TokenGroups, buffer.data(), bufferSize, &bufferSize))
        {
            LOG_F(ERROR, "Failed to get token information.");
            CloseHandle(hToken);
            return groups;
        }

        // 解析用户组信息
        PTOKEN_GROUPS pTokenGroups = reinterpret_cast<PTOKEN_GROUPS>(buffer.data());
        for (DWORD i = 0; i < pTokenGroups->GroupCount; i++)
        {
            SID_NAME_USE sidUse;
            DWORD nameLength = 0, domainLength = 0;
            LookupAccountSid(NULL, pTokenGroups->Groups[i].Sid, NULL, &nameLength, NULL, &domainLength, &sidUse);
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            {
                LOG_F(ERROR, "Failed to get account name and domain length.");
                CloseHandle(hToken);
                return groups;
            }

            std::vector<TCHAR> nameBuffer(nameLength);
            std::vector<TCHAR> domainBuffer(domainLength);
            if (!LookupAccountSid(NULL, pTokenGroups->Groups[i].Sid, nameBuffer.data(), &nameLength, domainBuffer.data(), &domainLength, &sidUse))
            {
                LOG_F(ERROR, "Failed to lookup account SID.");
                CloseHandle(hToken);
                return groups;
            }

            std::wstring groupName = L"";
            std::wstring nameStr(nameBuffer.begin(), nameBuffer.end());
            groupName += nameStr;
            groups.push_back(groupName);
        }

        CloseHandle(hToken);
#else
        // 获取用户组信息
        gid_t *groupsArray = nullptr;
        int groupCount = getgroups(0, NULL);
        if (groupCount == -1)
        {
            LOG_F(ERROR, "Failed to get user group count.");
            return groups;
        }

        groupsArray = new gid_t[groupCount];
        if (getgroups(groupCount, groupsArray) == -1)
        {
            LOG_F(ERROR, "Failed to get user groups.");
            delete[] groupsArray;
            return groups;
        }

        // 解析用户组信息
        for (int i = 0; i < groupCount; i++)
        {
            struct group *grp = getgrgid(groupsArray[i]);
            if (grp != nullptr)
            {
                std::wstring groupName = L"";
                std::wstring nameStr(grp->gr_name);
                groupName += nameStr;
                groups.push_back(groupName);
            }
        }

        delete[] groupsArray;
#endif

        return groups;
    }

    std::string getUsername()
    {
        std::string username;
#ifdef _WIN32
        char buffer[UNLEN + 1];
        DWORD size = UNLEN + 1;
        if (GetUserNameA(buffer, &size))
        {
            username = buffer;
        }
#else
        char *buffer;
        buffer = getlogin();
        if (buffer != nullptr)
        {
            username = buffer;
        }
#endif
        return username;
    }

    std::string getHostname()
    {
        std::string hostname;
#ifdef _WIN32
        char buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerNameA(buffer, &size))
        {
            hostname = buffer;
        }
#else
        char buffer[256];
        if (gethostname(buffer, sizeof(buffer)) == 0)
        {
            hostname = buffer;
        }
#endif
        return hostname;
    }
}
