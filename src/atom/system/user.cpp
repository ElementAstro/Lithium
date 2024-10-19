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

#include <array>
#include <memory>

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
    LOG_F(INFO, "isRoot called");
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
    LOG_F(INFO, "isRoot completed with result: {}", elevated);
    return elevated;
#else
    bool result = (getuid() == 0);
    LOG_F(INFO, "isRoot completed with result: {}", result);
    return result;
#endif
}

auto getUserGroups() -> std::vector<std::wstring> {
    LOG_F(INFO, "getUserGroups called");
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
#pragma unroll
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
        LOG_F(INFO, "Found group: {}", nameStr);
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
            LOG_F(INFO, "Found group: {}", nameStr);
        }
    }

    delete[] groupsArray;
#endif

    LOG_F(INFO, "getUserGroups completed with {} groups found", groups.size());
    return groups;
}

auto getUsername() -> std::string {
    LOG_F(INFO, "getUsername called");
    std::string username;
#ifdef _WIN32
    std::array<char, UNLEN + 1> buffer;
    DWORD size = UNLEN + 1;
    if (GetUserNameA(buffer.data(), &size) != 0) {
        username = std::string(buffer.data(), size - 1);
    }
#else
    char *buffer;
    buffer = getlogin();
    if (buffer != nullptr) {
        username = buffer;
    }
#endif
    LOG_F(INFO, "getUsername completed with result: {}", username);
    return username;
}

auto getHostname() -> std::string {
    LOG_F(INFO, "getHostname called");
    std::string hostname;
#ifdef _WIN32
    std::array<char, MAX_COMPUTERNAME_LENGTH + 1> buffer;
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(buffer.data(), &size) != 0) {
        hostname = std::string(buffer.data(), size);
    }
#else
    std::array<char, 256> buffer;
    if (gethostname(buffer.data(), buffer.size()) == 0) {
        hostname = buffer.data();
    }
#endif
    LOG_F(INFO, "getHostname completed with result: {}", hostname);
    return hostname;
}

auto getUserId() -> int {
    LOG_F(INFO, "getUserId called");
    int userId = 0;
#ifdef _WIN32
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwLengthNeeded;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &dwLengthNeeded);
        auto pTokenUser = std::unique_ptr<TOKEN_USER, decltype(&free)>(
            static_cast<TOKEN_USER *>(malloc(dwLengthNeeded)), free);
        if (GetTokenInformation(hToken, TokenUser, pTokenUser.get(),
                                dwLengthNeeded, &dwLengthNeeded) != 0) {
            PSID sid = pTokenUser->User.Sid;
            DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
            DWORD *subAuthority =
                GetSidSubAuthority(sid, subAuthorityCount - 1);
            userId = static_cast<int>(*subAuthority);
        }
        CloseHandle(hToken);
    }
#else
    userId = getuid();
#endif
    LOG_F(INFO, "getUserId completed with result: {}", userId);
    return userId;
}

auto getGroupId() -> int {
    LOG_F(INFO, "getGroupId called");
    int groupId = 0;
#ifdef _WIN32
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwLengthNeeded;
        GetTokenInformation(hToken, TokenPrimaryGroup, nullptr, 0,
                            &dwLengthNeeded);
        auto pTokenPrimaryGroup =
            std::unique_ptr<TOKEN_PRIMARY_GROUP, decltype(&free)>(
                static_cast<TOKEN_PRIMARY_GROUP *>(malloc(dwLengthNeeded)),
                free);
        if (GetTokenInformation(hToken, TokenPrimaryGroup,
                                pTokenPrimaryGroup.get(), dwLengthNeeded,
                                &dwLengthNeeded) != 0) {
            PSID sid = pTokenPrimaryGroup->PrimaryGroup;
            DWORD subAuthorityCount = *GetSidSubAuthorityCount(sid);
            DWORD *subAuthority =
                GetSidSubAuthority(sid, subAuthorityCount - 1);
            groupId = static_cast<int>(*subAuthority);
        }
        CloseHandle(hToken);
    }
#else
    groupId = getgid();
#endif
    LOG_F(INFO, "getGroupId completed with result: {}", groupId);
    return groupId;
}

#ifdef _WIN32
auto getUserProfileDirectory() -> std::string {
    LOG_F(INFO, "getUserProfileDirectory called");
    std::string userProfileDir;
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0) {
        DWORD dwSize = 0;
        GetUserProfileDirectoryA(hToken, nullptr, &dwSize);
        auto buffer = std::make_unique<char[]>(dwSize);
        if (GetUserProfileDirectoryA(hToken, buffer.get(), &dwSize) != 0) {
            userProfileDir = buffer.get();
        }
        CloseHandle(hToken);
    }
    LOG_F(INFO, "getUserProfileDirectory completed with result: {}",
          userProfileDir);
    return userProfileDir;
}
#endif

auto getHomeDirectory() -> std::string {
    LOG_F(INFO, "getHomeDirectory called");
    std::string homeDir;
#ifdef _WIN32
    homeDir = getUserProfileDirectory();
#else
    int userId = getUserId();
    struct passwd *userInfo = getpwuid(userId);
    homeDir = std::string(userInfo->pw_dir);
#endif
    LOG_F(INFO, "getHomeDirectory completed with result: {}", homeDir);
    return homeDir;
}

auto getCurrentWorkingDirectory() -> std::string {
    LOG_F(INFO, "getCurrentWorkingDirectory called");
#ifdef _WIN32
    // Windows-specific code
    std::array<char, MAX_PATH> cwd;
    if (GetCurrentDirectory(cwd.size(), cwd.data())) {
        std::string result = cwd.data();
        LOG_F(INFO, "getCurrentWorkingDirectory completed with result: {}",
              result);
        return result;
    }
    LOG_F(ERROR, "Error getting current working directory");
    return "Error getting current working directory";
#else
    // POSIX (Linux, macOS) specific code
    std::array<char, PATH_MAX> cwd;
    if (getcwd(cwd.data(), cwd.size()) != nullptr) {
        std::string result = cwd.data();
        LOG_F(INFO, "getCurrentWorkingDirectory completed with result: {}",
              result);
        return result;
    }
    LOG_F(ERROR, "Error getting current working directory");
    return "Error getting current working directory";
#endif
}

auto getLoginShell() -> std::string {
    LOG_F(INFO, "getLoginShell called");
    std::string loginShell;
#ifdef _WIN32
    std::array<char, MAX_PATH> buf;
    DWORD bufSize = buf.size();
    if (GetEnvironmentVariableA("COMSPEC", buf.data(), bufSize) != 0) {
        loginShell = std::string(buf.data());
    }
#else
    int userId = getUserId();
    struct passwd *userInfo = getpwuid(userId);
    loginShell = std::string(userInfo->pw_shell);
#endif
    LOG_F(INFO, "getLoginShell completed with result: {}", loginShell);
    return loginShell;
}

auto getLogin() -> std::string {
    LOG_F(INFO, "getLogin called");
#ifdef _WIN32
    std::array<char, UNLEN + 1> buffer;
    DWORD bufferSize = buffer.size();
    if (GetUserNameA(buffer.data(), &bufferSize) != 0) {
        std::string result = buffer.data();
        LOG_F(INFO, "getLogin completed with result: {}", result);
        return result;
    }
#else
    char *username = ::getlogin();
    if (username != nullptr) {
        std::string result = username;
        LOG_F(INFO, "getLogin completed with result: {}", result);
        return result;
    }
#endif
    LOG_F(ERROR, "Error getting login name");
    return "";
}
}  // namespace atom::system
