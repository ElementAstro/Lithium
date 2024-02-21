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
#include <locale>
#include <codecvt>
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
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::wstring nameStr = converter.from_bytes(grp->gr_name);
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

    int getUserId()
    {
        int userId = 0;
#ifdef _WIN32
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            DWORD dwLengthNeeded;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLengthNeeded);
            TOKEN_USER *pTokenUser = (TOKEN_USER *)malloc(dwLengthNeeded);
            if (GetTokenInformation(hToken, TokenUser, pTokenUser, dwLengthNeeded, &dwLengthNeeded))
            {
                PSID sid = pTokenUser->User.Sid;
                DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
                DWORD *subAuthority = GetSidSubAuthority(sid, subAuthorityCount - 1);
                userId = *subAuthority;
            }
            CloseHandle(hToken);
            free(pTokenUser);
        }
#else
        userId = getuid();
#endif
        return userId;
    }

    int getGroupId()
    {
        int groupId = 0;
#ifdef _WIN32
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            DWORD dwLengthNeeded;
            GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &dwLengthNeeded);
            TOKEN_PRIMARY_GROUP *pTokenPrimaryGroup = (TOKEN_PRIMARY_GROUP *)malloc(dwLengthNeeded);
            if (GetTokenInformation(hToken, TokenPrimaryGroup, pTokenPrimaryGroup, dwLengthNeeded, &dwLengthNeeded))
            {
                PSID sid = pTokenPrimaryGroup->PrimaryGroup;
                DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
                DWORD *subAuthority = GetSidSubAuthority(sid, subAuthorityCount - 1);
                groupId = *subAuthority;
            }
            CloseHandle(hToken);
            free(pTokenPrimaryGroup);
        }
#else
        groupId = getgid();
#endif
        return groupId;
    }

#ifdef _WIN32
    std::string getUserProfileDirectory()
    {
        std::string userProfileDir;
        HANDLE hToken;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            DWORD dwSize = 0;
            GetUserProfileDirectoryA(hToken, NULL, &dwSize);
            char *buffer = new char[dwSize];
            if (GetUserProfileDirectoryA(hToken, buffer, &dwSize))
            {
                userProfileDir = buffer;
            }
            delete[] buffer;
            CloseHandle(hToken);
        }
        return userProfileDir;
    }
#endif

    std::string getHomeDirectory()
    {
        std::string homeDir;
#ifdef _WIN32
        homeDir = getUserProfileDirectory();
#else
        int userId = getUserId();
        struct passwd *userInfo = getpwuid(userId);
        homeDir = std::string(userInfo->pw_dir);
#endif
        return homeDir;
    }

    std::string getLoginShell()
    {
        std::string loginShell;
#ifdef _WIN32
        char buf[MAX_PATH];
        DWORD bufSize = sizeof(buf) / sizeof(buf[0]);
        if (GetEnvironmentVariableA("COMSPEC", buf, bufSize) != 0)
        {
            loginShell = std::string(buf);
        }
#else
        int userId = getUserId();
        struct passwd *userInfo = getpwuid(userId);
        loginShell = std::string(userInfo->pw_shell);
#endif
        return loginShell;
    }
}
