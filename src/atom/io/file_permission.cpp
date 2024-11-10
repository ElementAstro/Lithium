#include "file_permission.hpp"

#ifdef _WIN32
#include <aclapi.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace atom::io {
#ifdef _WIN32
std::string getFilePermissions(const std::string &filePath) {
    DWORD dwRtnCode = 0;
    PACL pDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS *pEA = NULL;
    std::string permissions;

    dwRtnCode = GetNamedSecurityInfoA(filePath.c_str(), SE_FILE_OBJECT,
                                      DACL_SECURITY_INFORMATION, NULL, NULL,
                                      &pDACL, NULL, &pSD);
    if (dwRtnCode != ERROR_SUCCESS) {
        std::cerr << "GetNamedSecurityInfoA error: " << dwRtnCode << std::endl;
        return "";
    }

    if (pDACL != NULL) {
        for (DWORD i = 0; i < pDACL->AceCount; i++) {
            ACE_HEADER *aceHeader;
            if (GetAce(pDACL, i, (LPVOID *)&aceHeader)) {
                ACCESS_ALLOWED_ACE *ace = (ACCESS_ALLOWED_ACE *)aceHeader;
                if (ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {
                    if (ace->Mask & GENERIC_READ)
                        permissions += "r";
                    else
                        permissions += "-";
                    if (ace->Mask & GENERIC_WRITE)
                        permissions += "w";
                    else
                        permissions += "-";
                    if (ace->Mask & GENERIC_EXECUTE)
                        permissions += "x";
                    else
                        permissions += "-";
                }
            }
        }
    }

    if (pSD != NULL)
        LocalFree((HLOCAL)pSD);

    return permissions;
}

std::string getSelfPermissions() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        std::cerr << "GetModuleFileNameA error: " << GetLastError()
                  << std::endl;
        return "";
    }
    return getFilePermissions(path);
}
#else
auto getFilePermissions(const std::string &filePath) -> std::string {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) < 0) {
        perror("stat error");
        return "";
    }

    std::string permissions;
    permissions += (fileStat.st_mode & S_IRUSR) ? "r" : "-";  // User Read
    permissions += (fileStat.st_mode & S_IWUSR) ? "w" : "-";  // User Write
    permissions += (fileStat.st_mode & S_IXUSR) ? "x" : "-";  // User Execute
    permissions += (fileStat.st_mode & S_IRGRP) ? "r" : "-";  // Group Read
    permissions += (fileStat.st_mode & S_IWGRP) ? "w" : "-";  // Group Write
    permissions += (fileStat.st_mode & S_IXGRP) ? "x" : "-";  // Group Execute
    permissions += (fileStat.st_mode & S_IROTH) ? "r" : "-";  // Others Read
    permissions += (fileStat.st_mode & S_IWOTH) ? "w" : "-";  // Others Write
    permissions += (fileStat.st_mode & S_IXOTH) ? "x" : "-";  // Others Execute

    return permissions;
}

auto getSelfPermissions() -> std::string {
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len < 0) {
        perror("readlink error");
        return "";
    }
    path[len] = '\0';  // 确保字符串以'\0'结束
    return getFilePermissions(path);
}
#endif

auto compareFileAndSelfPermissions(const std::string &filePath)
    -> std::optional<bool> {
    std::string filePermissions = getFilePermissions(filePath);
    if (filePermissions.empty()) {
        return std::nullopt;
    }

    std::string selfPermissions = getSelfPermissions();
    if (selfPermissions.empty()) {
        return std::nullopt;
    }

    return filePermissions == selfPermissions;
}
}  // namespace atom::io
