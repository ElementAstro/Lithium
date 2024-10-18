#include "os.hpp"

#include <array>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <fstream>
#elif __APPLE__
#include <sys/utsname.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::system {

auto OperatingSystemInfo::toJson() const -> std::string {
    LOG_F(INFO, "Converting OperatingSystemInfo to JSON");
    std::stringstream stringstream;
    stringstream << "{\n";
    stringstream << R"(  "osName": ")" << osName << "\",\n";
    stringstream << R"(  "osVersion": ")" << osVersion << "\",\n";
    stringstream << R"(  "kernelVersion": ")" << kernelVersion << "\",\n";
    stringstream << R"(  "architecture": ")" << architecture << "\",\n";
    stringstream << R"(  "compiler": ")" << compiler << "\",\n";
    stringstream << R"(  "computerName": ")" << computerName << "\"\n";
    stringstream << "}\n";
    return stringstream.str();
}

auto getComputerName() -> std::optional<std::string> {
    LOG_F(INFO, "Getting computer name");
    constexpr size_t bufferSize = 256;
    std::array<char, bufferSize> buffer;

#if defined(_WIN32)
    auto size = static_cast<DWORD>(buffer.size());
    if (BOOL result = GetComputerNameA(buffer.data(), &size); result) {
        LOG_F(INFO, "Successfully retrieved computer name: {}", buffer.data());
        return std::string(buffer.data());
    } else {
        LOG_F(ERROR, "Failed to get computer name");
    }
#elif defined(__APPLE__)
    CFStringRef name = SCDynamicStoreCopyComputerName(NULL, NULL);
    if (name != NULL) {
        CFStringGetCString(name, buffer.data(), buffer.size(),
                           kCFStringEncodingUTF8);
        CFRelease(name);
        LOG_F(INFO, "Successfully retrieved computer name: {}", buffer.data());
        return std::string(buffer.data());
    } else {
        LOG_F(ERROR, "Failed to get computer name");
    }
#elif defined(__linux__) || defined(__linux)
    if (gethostname(buffer.data(), buffer.size()) == 0) {
        LOG_F(INFO, "Successfully retrieved computer name: {}", buffer.data());
        return std::string(buffer.data());
    } else {
        LOG_F(ERROR, "Failed to get computer name");
    }
#elif defined(__ANDROID__)
    LOG_F(WARNING, "Getting computer name is not supported on Android");
    return std::nullopt;
#endif

    return std::nullopt;
}

auto parseFile(const std::string& filePath)
    -> std::pair<std::string, std::string> {
    LOG_F(INFO, "Parsing file: {}", filePath.c_str());
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_F(ERROR, "Cannot open file: {}", filePath.c_str());
        THROW_FAIL_TO_OPEN_FILE("Cannot open file: " + filePath);
    }

    std::pair<std::string, std::string> osInfo;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;  // Skip empty lines and comments
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // Remove double quotes from the value
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            if (key == "PRETTY_NAME") {
                osInfo.first = value;
                LOG_F(INFO, "Found PRETTY_NAME: {}", value.c_str());
            } else if (key == "VERSION") {
                osInfo.second = value;
                LOG_F(INFO, "Found VERSION: {}", value.c_str());
            }
        }
    }

    return osInfo;
}

auto getOperatingSystemInfo() -> OperatingSystemInfo {
    LOG_F(INFO, "Starting getOperatingSystemInfo function");
    OperatingSystemInfo osInfo;

#ifdef _WIN32
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO)&osvi) != 0) {
        osInfo.osName = "Windows";
        osInfo.osVersion = std::format("{}.{} (Build {})", osvi.dwMajorVersion,
                                       osvi.dwMinorVersion, osvi.dwBuildNumber);
        LOG_F(INFO, "Retrieved OS info: {} {}", osInfo.osName.c_str(),
              osInfo.osVersion.c_str());
    } else {
        LOG_F(ERROR, "Failed to get OS version");
    }
#elif __linux__
    auto osReleaseInfo = parseFile("/etc/os-release");
    if (!osReleaseInfo.first.empty()) {
        osInfo.osName = osReleaseInfo.first;
        osInfo.osVersion = osReleaseInfo.second;
    } else {
        auto lsbReleaseInfo = parseFile("/etc/lsb-release");
        if (!lsbReleaseInfo.first.empty()) {
            osInfo.osName = lsbReleaseInfo.first;
            osInfo.osVersion = lsbReleaseInfo.second;
        } else {
            std::ifstream redhatReleaseFile("/etc/redhat-release");
            if (redhatReleaseFile.is_open()) {
                std::string line;
                std::getline(redhatReleaseFile, line);
                osInfo.osName = line;
                redhatReleaseFile.close();
                LOG_F(INFO, "Retrieved OS info from /etc/redhat-release: {}",
                      line.c_str());
            }
        }
    }

    if (osInfo.osName.empty()) {
        LOG_F(ERROR, "Failed to get OS name");
    }

    std::ifstream kernelVersionFile("/proc/version");
    if (kernelVersionFile.is_open()) {
        std::string line;
        std::getline(kernelVersionFile, line);
        osInfo.kernelVersion = line.substr(0, line.find(" "));
        kernelVersionFile.close();
        LOG_F(INFO, "Retrieved kernel version: {}",
              osInfo.kernelVersion.c_str());
    } else {
        LOG_F(ERROR, "Failed to open /proc/version");
    }
#elif __APPLE__
    struct utsname info;
    if (uname(&info) == 0) {
        osInfo.osName = info.sysname;
        osInfo.osVersion = info.release;
        osInfo.kernelVersion = info.version;
        LOG_F(INFO, "Retrieved OS info: {} {} {}", info.sysname, info.release,
              info.version);
    } else {
        LOG_F(ERROR, "Failed to get OS info using uname");
    }
#endif

    // 获取系统架构
#if defined(__i386__) || defined(__i386)
    const std::string ARCHITECTURE = "x86";
#elif defined(__x86_64__)
    const std::string ARCHITECTURE = "x86_64";
#elif defined(__arm__)
    const std::string ARCHITECTURE = "ARM";
#elif defined(__aarch64__)
    const std::string ARCHITECTURE = "ARM64";
#else
    const std::string ARCHITECTURE = "Unknown architecture";
#endif
    osInfo.architecture = ARCHITECTURE;
    LOG_F(INFO, "Detected architecture: {}", ARCHITECTURE.c_str());

    const std::string COMPILER =
#if defined(__clang__)
        std::format("Clang {}.{}.{}", __clang_major__, __clang_minor__,
                    __clang_patchlevel__);
#elif defined(__GNUC__)
        std::format("GCC {}.{}.{}", __GNUC__, __GNUC_MINOR__,
                    __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
        std::format("MSVC {}", _MSC_FULL_VER);
#else
        "Unknown compiler";
#endif
    osInfo.compiler = COMPILER;
    LOG_F(INFO, "Detected compiler: {}", COMPILER.c_str());

    osInfo.computerName = getComputerName().value_or("Unknown computer name");
    LOG_F(INFO, "Detected computer name: {}", osInfo.computerName.c_str());

    LOG_F(INFO, "Finished getOperatingSystemInfo function");
    return osInfo;
}

auto isWsl() -> bool {
    LOG_F(INFO, "Checking if running in WSL");
    std::ifstream procVersion("/proc/version");
    std::string line;
    if (procVersion.is_open()) {
        std::getline(procVersion, line);
        procVersion.close();
        // Check if the line contains "Microsoft" which is a typical indicator
        // of WSL
        bool isWsl = line.find("microsoft") != std::string::npos ||
                     line.find("WSL") != std::string::npos;
        LOG_F(INFO, "WSL check result: %d", isWsl);
        return isWsl;
    } else {
        LOG_F(ERROR, "Failed to open /proc/version");
    }
    return false;
}

}  // namespace atom::system