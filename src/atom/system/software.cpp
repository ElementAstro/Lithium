#include "software.hpp"

#include <array>
#include <memory>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <aclapi.h>
#include <shlobj.h>
// clang-format on
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <sys/stat.h>
#elif defined(__ANDROID__)
#include <android/native_activity.h>
#else
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#endif

#include "atom/utils/string.hpp"

namespace atom::system {
auto getAppVersion(const fs::path& app_path) -> std::string {
#ifdef _WIN32
    DWORD handle;
    auto wappPath = atom::utils::stringToWString(app_path.string());
    DWORD size = GetFileVersionInfoSizeW(wappPath.c_str(), &handle);
    if (size != 0) {
        LPVOID buffer = malloc(size);
        if (GetFileVersionInfoW(wappPath.c_str(), handle, size, buffer) != 0) {
            LPVOID value;
            UINT length;
            if (VerQueryValue(buffer,
                              TEXT("\\StringFileInfo\\040904b0\\FileVersion"),
                              &value, &length)) {
                std::string version(static_cast<char*>(value), length);
                free(buffer);
                return version;
            }
        }
        free(buffer);
    }
#elif defined(__APPLE__)
    CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, app_path.c_str(),
                                                 kCFURLPOSIXPathStyle, true);
    if (url != nullptr) {
        CFBundleRef bundle = CFBundleCreate(nullptr, url);
        if (bundle != nullptr) {
            CFStringRef version =
                static_cast<CFStringRef>(CFBundleGetValueForInfoDictionaryKey(
                    bundle, kCFBundleVersionKey));
            if (version != nullptr) {
                char buffer[256];
                if (CFStringGetCString(version, buffer, sizeof(buffer),
                                       kCFStringEncodingUTF8)) {
                    CFRelease(bundle);
                    CFRelease(url);
                    return std::string(buffer);
                }
            }
            CFRelease(bundle);
        }
        CFRelease(url);
    }
#elif defined(__ANDROID__)
    ANativeActivity* activity = ANativeActivity_getActivity();
    if (activity != nullptr && activity->callbacks != nullptr &&
        activity->callbacks->onGetPackageVersion != nullptr) {
        JNIEnv* env;
        activity->vm->AttachCurrentThread(&env, nullptr);
        jstring package_name = env->NewStringUTF(app_path.c_str());
        jstring version =
            static_cast<jstring>(activity->callbacks->onGetPackageVersion(
                activity->instance, package_name));
        env->DeleteLocalRef(package_name);
        if (version != nullptr) {
            const char* utf8_version = env->GetStringUTFChars(version, nullptr);
            std::string result(utf8_version);
            env->ReleaseStringUTFChars(version, utf8_version);
            env->DeleteLocalRef(version);
            return result;
        }
        activity->vm->DetachCurrentThread();
    }
#else
    FILE* file = fopen(app_path.c_str(), "rb");
    if (file != nullptr) {
        char buffer[256];
        std::string version;
        while (fgets(buffer, sizeof(buffer), file) != nullptr) {
            if (strncmp(buffer, "@(#)", 4) == 0) {
                char* start = strchr(buffer, ' ');
                if (start != nullptr) {
                    char* end = strchr(start + 1, ' ');
                    if (end != nullptr) {
                        version = std::string(start + 1, end - start - 1);
                        break;
                    }
                }
            }
        }
        fclose(file);
        if (!version.empty()) {
            return version;
        }
    }
#endif

    return "";
}

auto getAppPermissions(const fs::path& app_path) -> std::vector<std::string> {
    std::vector<std::string> permissions;

#ifdef _WIN32
    PSECURITY_DESCRIPTOR securityDescriptor;
    PACL dacl = nullptr;

    if (GetNamedSecurityInfoW(
            atom::utils::stringToWString(app_path.string()).c_str(),
            SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &dacl,
            nullptr, &securityDescriptor) == ERROR_SUCCESS) {
        if (dacl != nullptr) {
            LPVOID ace;
            for (DWORD i = 0; i < dacl->AceCount; ++i) {
                if (GetAce(dacl, i, &ace) != 0) {
                    if (static_cast<PACE_HEADER>(ace)->AceType ==
                        ACCESS_ALLOWED_ACE_TYPE) {
                        auto* allowedAce =
                            static_cast<PACCESS_ALLOWED_ACE>(ace);
                        LPTSTR userName = nullptr;
                        DWORD nameSize = 0;
                        LPTSTR domainName = nullptr;
                        DWORD domainSize = 0;
                        SID_NAME_USE sidType;

                        LookupAccountSid(nullptr, &allowedAce->SidStart,
                                         userName, &nameSize, domainName,
                                         &domainSize, &sidType);
                        userName = static_cast<LPTSTR>(
                            malloc(nameSize * sizeof(TCHAR)));
                        domainName = static_cast<LPTSTR>(
                            malloc(domainSize * sizeof(TCHAR)));
                        if (LookupAccountSid(nullptr, &allowedAce->SidStart,
                                             userName, &nameSize, domainName,
                                             &domainSize, &sidType)) {
                            std::string permission = "User: ";
                            std::string permissionStr = permission;
                            std::string userNameStr = userName;
                            std::string domainNameStr = domainName;

                            permissionStr += userNameStr + "\\" + domainNameStr;
                            permission = permissionStr;

                            permissions.push_back(permission);
                        }
                        free(userName);
                        free(domainName);
                    }
                }
            }
        }
        LocalFree(securityDescriptor);
    }
#elif defined(__APPLE__) || defined(__linux__)
    struct stat file_stat;
    if (stat(app_path.c_str(), &file_stat) == 0) {
        if (file_stat.st_mode & S_IRUSR) {
            permissions.push_back("Owner: Read");
        }
        if (file_stat.st_mode & S_IWUSR) {
            permissions.push_back("Owner: Write");
        }
        if (file_stat.st_mode & S_IXUSR) {
            permissions.push_back("Owner: Execute");
        }
        if (file_stat.st_mode & S_IRGRP) {
            permissions.push_back("Group: Read");
        }
        if (file_stat.st_mode & S_IWGRP) {
            permissions.push_back("Group: Write");
        }
        if (file_stat.st_mode & S_IXGRP) {
            permissions.push_back("Group: Execute");
        }
        if (file_stat.st_mode & S_IROTH) {
            permissions.push_back("Others: Read");
        }
        if (file_stat.st_mode & S_IWOTH) {
            permissions.push_back("Others: Write");
        }
        if (file_stat.st_mode & S_IXOTH) {
            permissions.push_back("Others: Execute");
        }
    }
#elif defined(__ANDROID__)
    ANativeActivity* activity = ANativeActivity_getActivity();
    if (activity != nullptr && activity->callbacks != nullptr &&
        activity->callbacks->onGetPackagePermissions != nullptr) {
        JNIEnv* env;
        activity->vm->AttachCurrentThread(&env, nullptr);
        jstring package_name = env->NewStringUTF(app_path.c_str());
        jobjectArray permissions_array = static_cast<jobjectArray>(
            activity->callbacks->onGetPackagePermissions(activity->instance,
                                                         package_name));
        env->DeleteLocalRef(package_name);
        if (permissions_array != nullptr) {
            jsize length = env->GetArrayLength(permissions_array);
            for (jsize i = 0; i < length; ++i) {
                jstring permission = static_cast<jstring>(
                    env->GetObjectArrayElement(permissions_array, i));
                const char* utf8_permission =
                    env->GetStringUTFChars(permission, nullptr);
                permissions.push_back(std::string(utf8_permission));
                env->ReleaseStringUTFChars(permission, utf8_permission);
                env->DeleteLocalRef(permission);
            }
            env->DeleteLocalRef(permissions_array);
        }
        activity->vm->DetachCurrentThread();
    }
#endif

    return permissions;
}

auto getAppPath(const std::string& software_name) -> fs::path {
#ifdef _WIN32
    WCHAR programFilesPath[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0,
                         programFilesPath) == S_OK) {
        fs::path path(programFilesPath);
        path.append(software_name);
        if (fs::exists(path)) {
            return path;
        }
    }
    return "";
#elif defined(__APPLE__)
    fs::path app_path("/Applications");
    app_path.append(software_name);
    if (fs::exists(app_path)) {
        return app_path;
    }
    return "";
#elif defined(__linux__)
    std::string command = "which " + software_name;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    if (!result.empty()) {
        result.pop_back();  // Remove newline
        if (fs::exists(result)) {
            return fs::path(result);
        }
    }
    return "";
#endif
    return fs::current_path();  // Fallback to current path if all
                                // else fails
}

auto checkSoftwareInstalled(const std::string& software_name) -> bool {
    bool isInstalled = false;

#ifdef _WIN32
    HKEY hKey;
    std::string regPath =
        R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall)";
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      atom::utils::stringToWString(regPath).c_str(), 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        wchar_t subKeyName[256];
        DWORD subKeyNameSize = sizeof(subKeyName);
        while (RegEnumKeyExW(hKey, index, subKeyName, &subKeyNameSize, nullptr,
                             nullptr, nullptr,
                             nullptr) != ERROR_NO_MORE_ITEMS) {
            HKEY hSubKey;
            if (RegOpenKeyExW(hKey, subKeyName, 0, KEY_READ, &hSubKey) ==
                ERROR_SUCCESS) {
                char displayName[256];
                DWORD displayNameSize = sizeof(displayName);
                if (RegQueryValueExW(hSubKey, L"DisplayName", nullptr, nullptr,
                                     reinterpret_cast<LPBYTE>(displayName),
                                     &displayNameSize) == ERROR_SUCCESS) {
                    if (software_name == displayName) {
                        isInstalled = true;
                        RegCloseKey(hSubKey);
                        break;
                    }
                }
                RegCloseKey(hSubKey);
            }
            subKeyNameSize = sizeof(subKeyName);
            ++index;
        }
        RegCloseKey(hKey);
    }

#elif defined(__APPLE__)
    std::string command =
        "mdfind \"kMDItemKind == 'Application' && kMDItemFSName == '*" +
        software_name + "*.app'\"";
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        isInstalled = !result.empty();
    }

#elif defined(__linux__)
    std::string command = "which " + software_name + " > /dev/null 2>&1";
    int result = std::system(command.c_str());
    isInstalled = (result == 0);

#endif

    return isInstalled;
}
}  // namespace atom::system
