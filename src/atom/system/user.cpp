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
// clang-format off
#include <windows.h>
#include <lmcons.h>
#include <tchar.h>
#include <userenv.h>
// clang-format on
#else
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <climits>
#include <codecvt>
#include <locale>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
auto isRoot() -> bool {
#ifdef _WIN32
    HANDLE hToken;
    TOKEN_ELEVATION elevation;
    DWORD dwSize;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0) {
        LOG_F(ERROR, "isRoot error: OpenProcessToken error");
        return false;
    }

    if (GetTokenInformation(hToken, TokenElevation, &elevation,
                            sizeof(elevation), &dwSize) == 0) {
        LOG_F(ERROR, "isRoot error: GetTokenInformation error");
        CloseHandle(hToken);
        return false;
    }

    bool elevated = (elevation.TokenIsElevated != 0);
    CloseHandle(hToken);
    return elevated;
#else
    return (getuid() == 0);
#endif
}

auto getUserGroups() -> std::vector<std::wstring> {
    std::vector<std::wstring> groups;

#ifdef _WIN32
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0) {
        LOG_F(ERROR, "Failed to open process token.");
        return groups;
    }
    DWORD bufferSize = 0;
    GetTokenInformation(hToken, TokenGroups, nullptr, 0, &bufferSize);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        LOG_F(ERROR, "Failed to get token information size.");
        CloseHandle(hToken);
        return groups;
    }

    std::vector<BYTE> buffer(bufferSize);
    if (GetTokenInformation(hToken, TokenGroups, buffer.data(), bufferSize,
                            &bufferSize) == 0) {
        LOG_F(ERROR, "Failed to get token information.");
        CloseHandle(hToken);
        return groups;
    }

    // 解析用户组信息
    auto *pTokenGroups = reinterpret_cast<PTOKEN_GROUPS>(buffer.data());
    for (DWORD i = 0; i < pTokenGroups->GroupCount; i++) {
        SID_NAME_USE sidUse;
        DWORD nameLength = 0;
        DWORD domainLength = 0;
        LookupAccountSid(nullptr, pTokenGroups->Groups[i].Sid, nullptr,
                         &nameLength, nullptr, &domainLength, &sidUse);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            LOG_F(ERROR, "Failed to get account name and domain length.");
            CloseHandle(hToken);
            return groups;
        }

        std::vector<TCHAR> nameBuffer(nameLength);
        std::vector<TCHAR> domainBuffer(domainLength);
        if (!LookupAccountSid(nullptr, pTokenGroups->Groups[i].Sid,
                              nameBuffer.data(), &nameLength,
                              domainBuffer.data(), &domainLength, &sidUse)) {
            LOG_F(ERROR, "Failed to lookup account SID.");
            CloseHandle(hToken);
            return groups;
        }

        std::wstring groupName;
        std::wstring nameStr(nameBuffer.begin(), nameBuffer.end());
        groupName += nameStr;
        groups.push_back(groupName);
    }

    CloseHandle(hToken);
#else
    // 获取用户组信息
    gid_t *groupsArray = nullptr;
    int groupCount = getgroups(0, nullptr);
    if (groupCount == -1) {
        LOG_F(ERROR, "Failed to get user group count.");
        return groups;
    }

    groupsArray = new gid_t[groupCount];
    if (getgroups(groupCount, groupsArray) == -1) {
        LOG_F(ERROR, "Failed to get user groups.");
        delete[] groupsArray;
        return groups;
    }

    // 解析用户组信息
    for (int i = 0; i < groupCount; i++) {
        struct group *grp = getgrgid(groupsArray[i]);
        if (grp != nullptr) {
            std::wstring groupName = L"";
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
            std::wstring nameStr = converter.from_bytes(grp->gr_name);
            groupName += nameStr;
            groups.push_back(groupName);
        }
    }

    delete[] groupsArray;
#endif

    return groups;
}

auto getUsername() -> std::string {
    std::string username;
#ifdef _WIN32
    char buffer[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameA(buffer, &size) != 0) {
        username = buffer;
    }
#else
    char *buffer;
    buffer = getlogin();
    if (buffer != nullptr) {
        username = buffer;
    }
#endif
    return username;
}

auto getHostname() -> std::string {
    std::string hostname;
#ifdef _WIN32
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(buffer, &size) != 0) {
        hostname = buffer;
    }
#else
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        hostname = buffer;
    }
#endif
    return hostname;
}

int getUserId() {
    int userId = 0;
#ifdef _WIN32
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwLengthNeeded;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLengthNeeded);
        TOKEN_USER *pTokenUser = (TOKEN_USER *)malloc(dwLengthNeeded);
        if (GetTokenInformation(hToken, TokenUser, pTokenUser, dwLengthNeeded,
                                &dwLengthNeeded) != 0) {
            PSID sid = pTokenUser->User.Sid;
            DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
            DWORD *subAuthority =
                GetSidSubAuthority(sid, subAuthorityCount - 1);
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

auto getGroupId() -> int {
    int groupId = 0;
#ifdef _WIN32
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwLengthNeeded;
        GetTokenInformation(hToken, TokenPrimaryGroup, nullptr, 0,
                            &dwLengthNeeded);
        auto *pTokenPrimaryGroup =
            (TOKEN_PRIMARY_GROUP *)malloc(dwLengthNeeded);
        if (GetTokenInformation(hToken, TokenPrimaryGroup, pTokenPrimaryGroup,
                                dwLengthNeeded, &dwLengthNeeded) != 0) {
            PSID sid = pTokenPrimaryGroup->PrimaryGroup;
            DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
            DWORD *subAuthority =
                GetSidSubAuthority(sid, subAuthorityCount - 1);
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
auto getUserProfileDirectory() -> std::string {
    std::string userProfileDir;
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwSize = 0;
        GetUserProfileDirectoryA(hToken, nullptr, &dwSize);
        char *buffer = new char[dwSize];
        if (GetUserProfileDirectoryA(hToken, buffer, &dwSize) != 0) {
            userProfileDir = buffer;
        }
        delete[] buffer;
        CloseHandle(hToken);
    }
    return userProfileDir;
}
#endif

auto getHomeDirectory() -> std::string {
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

auto getCurrentWorkingDirectory() -> std::string {
#ifdef _WIN32
    // Windows-specific code
    char cwd[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, cwd)) {
        return std::string(cwd);
    }
    return "Error getting current working directory";
#else
    // POSIX (Linux, macOS) specific code
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return cwd;
    }
    return "Error getting current working directory";
#endif
}

auto getLoginShell() -> std::string {
    std::string loginShell;
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD bufSize = sizeof(buf) / sizeof(buf[0]);
    if (GetEnvironmentVariableA("COMSPEC", buf, bufSize) != 0) {
        loginShell = std::string(buf);
    }
#else
    int userId = getUserId();
    struct passwd *userInfo = getpwuid(userId);
    loginShell = std::string(userInfo->pw_shell);
#endif
    return loginShell;
}

std::string getLogin() {
#ifdef _WIN32
    char buffer[UNLEN + 1];
    DWORD bufferSize = UNLEN + 1;
    if (GetUserNameA(buffer, &bufferSize) != 0) {
        return buffer;
    }
#else
    char *username = ::getlogin();
    if (username != nullptr) {
        return username;
    }
#endif
    return "";
}
}  // namespace atom::system
