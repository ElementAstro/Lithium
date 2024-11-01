#include "system_dependency.hpp"

#include "atom/system/command.hpp"

#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>

#if defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__)
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#else
#error "Unsupported platform"
#endif

#include "atom/type/json.hpp"

namespace lithium {

using json = nlohmann::json;

// 匿名命名空间用于私有变量
namespace {
const std::string CACHE_FILE = "dependency_cache.json";
std::mutex cacheMutex;
}  // namespace

DependencyManager::DependencyManager(std::vector<DependencyInfo> dependencies)
    : dependencies_(std::move(dependencies)) {
    detectPlatform();
    configurePackageManager();
    loadCacheFromFile();
}

void DependencyManager::setLogCallback(
    std::function<void(LogLevel, const std::string&)> callback) {
    logCallback_ = std::move(callback);
}

void DependencyManager::detectPlatform() {
#ifdef PLATFORM_LINUX
    // 检测具体的 Linux 发行版
    std::ifstream osReleaseFile("/etc/os-release");
    std::string line;
    std::regex debianRegex(R"(ID=debian|ID=ubuntu|ID=linuxmint)");
    std::regex fedoraRegex(R"(ID=fedora|ID=rhel|ID=centos)");
    std::regex archRegex(R"(ID=arch|ID=manjaro)");
    std::regex opensuseRegex(R"(ID=opensuse|ID=suse)");
    std::regex gentooRegex(R"(ID=gentoo)");

    if (osReleaseFile.is_open()) {
        while (std::getline(osReleaseFile, line)) {
            if (std::regex_search(line, debianRegex)) {
                distroType_ = DistroType::DEBIAN;
                return;
            }
            if (std::regex_search(line, fedoraRegex)) {
                distroType_ = DistroType::FEDORA;
                return;
            }
            if (std::regex_search(line, archRegex)) {
                distroType_ = DistroType::ARCH;
                return;
            }
            if (std::regex_search(line, opensuseRegex)) {
                distroType_ = DistroType::OPENSUSE;
                return;
            }
            if (std::regex_search(line, gentooRegex)) {
                distroType_ = DistroType::GENTOO;
                return;
            }
        }
    }
    distroType_ = DistroType::UNKNOWN;
#elif defined(PLATFORM_MAC)
    distroType_ = DistroType::MACOS;
#elif defined(PLATFORM_WINDOWS)
    distroType_ = DistroType::WINDOWS;
#else
    distroType_ = DistroType::UNKNOWN;
#endif
}

void DependencyManager::configurePackageManager() {
#ifdef PLATFORM_LINUX
    switch (distroType_) {
        case DistroType::DEBIAN:
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd = "dpkg -s " + dep.name + " > /dev/null 2>&1";
                if (!dep.version.empty()) {
                    cmd += " && dpkg -s " + dep.name +
                           " | grep Version | grep " + dep.version;
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo apt-get install -y " + dep.name +
                           (dep.version.empty() ? "" : "=" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo apt-get remove -y " + dep.name;
            };
            break;
        case DistroType::FEDORA:
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd = "rpm -q " + dep.name + " > /dev/null 2>&1";
                if (!dep.version.empty()) {
                    cmd += " && rpm -q " + dep.name + "-" + dep.version +
                           " > /dev/null 2>&1";
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo dnf install -y " + dep.name +
                           (dep.version.empty() ? "" : "-" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo dnf remove -y " + dep.name;
            };
            break;
        case DistroType::ARCH:
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd =
                    "pacman -Qi " + dep.name + " > /dev/null 2>&1";
                if (!dep.version.empty()) {
                    cmd += " && pacman -Qi " + dep.name +
                           " | grep Version | grep " + dep.version;
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo pacman -S --noconfirm " + dep.name +
                           (dep.version.empty() ? "" : "=" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo pacman -R --noconfirm " + dep.name;
            };
            break;
        case DistroType::OPENSUSE:
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd = "zypper se --installed-only " + dep.name +
                                  " > /dev/null 2>&1";
                if (!dep.version.empty()) {
                    cmd += " && zypper se --installed-only " + dep.name +
                           " | grep " + dep.version;
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo zypper install -y " + dep.name +
                           (dep.version.empty() ? "" : "=" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo zypper remove -y " + dep.name;
            };
            break;
        case DistroType::GENTOO:
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd =
                    "equery list " + dep.name + " > /dev/null 2>&1";
                if (!dep.version.empty()) {
                    cmd += " && equery list " + dep.name + "/" + dep.version +
                           " > /dev/null 2>&1";
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo emerge " + dep.name +
                           (dep.version.empty() ? "" : "/" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo emerge --unmerge " + dep.name;
            };
            break;
        default:
            // 默认使用 apt-get
            packageManager_.getCheckCommand =
                [](const DependencyInfo& dep) -> std::string {
                std::string cmd = "pkg-config --exists " + dep.name;
                if (!dep.version.empty()) {
                    // pkg-config 支持特定版本检查
                    cmd += " && pkg-config --atleast-version=" + dep.version +
                           " " + dep.name;
                }
                return cmd;
            };
            packageManager_.getInstallCommand =
                [this](const DependencyInfo& dep) -> std::string {
                if (!customInstallCommands_.contains(dep.name)) {
                    return "sudo apt-get install -y " + dep.name +
                           (dep.version.empty() ? "" : "=" + dep.version);
                }
                return customInstallCommands_.at(dep.name);
            };
            packageManager_.getUninstallCommand =
                [](const DependencyInfo& dep) -> std::string {
                return "sudo apt-get remove -y " + dep.name;
            };
            break;
    }
#elif defined(PLATFORM_MAC)
    packageManager_.getCheckCommand =
        [this](const DependencyInfo& dep) -> std::string {
        std::string cmd = "brew list " + dep.name + " > /dev/null 2>&1";
        if (!dep.version.empty()) {
            cmd += " && brew info " + dep.name + " | grep " + dep.version;
        }
        return cmd;
    };
    packageManager_.getInstallCommand =
        [this](const DependencyInfo& dep) -> std::string {
        if (!customInstallCommands_.count(dep.name)) {
            return "brew install " + dep.name +
                   (dep.version.empty() ? "" : "@" + dep.version);
        }
        return customInstallCommands_.at(dep.name);
    };
    packageManager_.getUninstallCommand =
        [this](const DependencyInfo& dep) -> std::string {
        return "brew uninstall " + dep.name;
    };
#elif defined(PLATFORM_WINDOWS)
    packageManager_.getCheckCommand =
        [this](const DependencyInfo& dep) -> std::string {
        std::string cmd;
        if (!dep.version.empty()) {
            cmd = "choco list --local-only " + dep.name + " | findstr " +
                  dep.version + " > nul 2>&1";
        } else {
            cmd = "choco list --local-only " + dep.name + " > nul 2>&1";
        }
        return cmd;
    };
    packageManager_.getInstallCommand =
        [this](const DependencyInfo& dep) -> std::string {
        if (customInstallCommands_.count(dep.name)) {
            return customInstallCommands_.at(dep.name);
        }
        // 优先使用 Chocolatey，其次是 winget 和 scoop
        if (isCommandAvailable("choco")) {
            return "choco install " + dep.name + " -y" +
                   (dep.version.empty() ? "" : " --version " + dep.version);
        } else if (isCommandAvailable("winget")) {
            return "winget install --id " + dep.name + " -e --silent" +
                   (dep.version.empty() ? "" : " --version " + dep.version);
        } else if (isCommandAvailable("scoop")) {
            return "scoop install " + dep.name +
                   (dep.version.empty() ? "" : "@" + dep.version);
        } else {
            return "echo 'No supported package manager found for installing " +
                   dep.name + "'";
        }
    };
    packageManager_.getUninstallCommand =
        [this](const DependencyInfo& dep) -> std::string {
        if (customInstallCommands_.count(dep.name)) {
            // 假设自定义命令也适用于卸载
            return customInstallCommands_.at(dep.name);
        }
        if (isCommandAvailable("choco")) {
            return "choco uninstall " + dep.name + " -y";
        } else if (isCommandAvailable("winget")) {
            return "winget uninstall --id " + dep.name + " -e --silent";
        } else if (isCommandAvailable("scoop")) {
            return "scoop uninstall " + dep.name;
        } else {
            return "echo 'No supported package manager found for "
                   "uninstalling " +
                   dep.name + "'";
        }
    };
#endif
}

void DependencyManager::checkAndInstallDependencies() {
    std::vector<std::thread> threads;
    threads.reserve(dependencies_.size());
    for (const auto& dep : dependencies_) {
        threads.emplace_back([this, dep]() {
            try {
                if (!isDependencyInstalled(dep)) {
                    log(LogLevel::INFO,
                        "Dependency " + dep.name +
                            " not found, attempting to install...");
                    installDependency(dep);
                    log(LogLevel::INFO,
                        "Successfully installed dependency: " + dep.name);
                } else {
                    log(LogLevel::INFO,
                        "Dependency " + dep.name + " is already installed.");
                }
            } catch (const DependencyException& ex) {
                log(LogLevel::ERROR, "Error installing dependency " + dep.name +
                                         ": " + ex.what());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    saveCacheToFile();
}

auto DependencyManager::isDependencyInstalled(const DependencyInfo& dep)
    -> bool {
    std::lock_guard<std::mutex> lock(cacheMutex);
    if (installedCache_.find(dep.name) != installedCache_.end()) {
        return installedCache_[dep.name];
    }

    std::string checkCommand = packageManager_.getCheckCommand(dep);
    bool isInstalled = false;
    try {
        isInstalled = atom::system::executeCommandSimple(checkCommand);
    } catch (...) {
        isInstalled = false;
    }
    installedCache_[dep.name] = isInstalled;
    return isInstalled;
}

void DependencyManager::installDependency(const DependencyInfo& dep) {
    std::string installCommand = packageManager_.getInstallCommand(dep);
    bool success = atom::system::executeCommandSimple(installCommand);
    if (!success) {
        throw DependencyException("Failed to install " + dep.name);
    }
    // 更新缓存
    std::lock_guard<std::mutex> lock(cacheMutex);
    installedCache_[dep.name] = true;
}

void DependencyManager::uninstallDependency(const std::string& depName) {
    // 查找依赖项
    auto it = std::find_if(dependencies_.begin(), dependencies_.end(),
                           [&depName](const DependencyInfo& info) {
                               return info.name == depName;
                           });
    if (it == dependencies_.end()) {
        log(LogLevel::WARNING, "Dependency " + depName + " not managed.");
        return;
    }

    if (!isDependencyInstalled(*it)) {
        log(LogLevel::INFO, "Dependency " + depName + " is not installed.");
        return;
    }

    try {
        log(LogLevel::INFO, "Uninstalling dependency: " + depName);
        uninstallDependencyInternal(depName);
        // 更新缓存
        std::lock_guard<std::mutex> lock(cacheMutex);
        installedCache_[depName] = false;
        log(LogLevel::INFO, "Successfully uninstalled dependency: " + depName);
    } catch (const DependencyException& ex) {
        log(LogLevel::ERROR,
            "Error uninstalling dependency " + depName + ": " + ex.what());
    }

    saveCacheToFile();
}

void DependencyManager::uninstallDependencyInternal(
    const std::string& depName) {
    // 查找依赖项
    auto it = std::find_if(dependencies_.begin(), dependencies_.end(),
                           [&depName](const DependencyInfo& info) {
                               return info.name == depName;
                           });
    if (it == dependencies_.end()) {
        throw DependencyException("Dependency " + depName + " not found.");
    }

    std::string uninstallCommand = packageManager_.getUninstallCommand(*it);
    bool success = atom::system::executeCommandSimple(uninstallCommand);
    if (!success) {
        throw DependencyException("Failed to uninstall " + depName);
    }
}

auto DependencyManager::getCheckCommand(const DependencyInfo& /*dep*/) const
    -> std::string {
    // 已通过包管理器配置
    return "";
}

auto DependencyManager::getInstallCommand(const DependencyInfo& /*dep*/) const
    -> std::string {
    // 已通过包管理器配置
    return "";
}

auto DependencyManager::getUninstallCommand(const DependencyInfo& /*dep*/) const
    -> std::string {
    // 已通过包管理器配置
    return "";
}

auto DependencyManager::isCommandAvailable(const std::string& command) const
    -> bool {
    std::string checkCommand;
#ifdef PLATFORM_WINDOWS
    checkCommand = "where " + command + " > nul 2>&1";
#else
    checkCommand = "command -v " + command + " > /dev/null 2>&1";
#endif
    return atom::system::executeCommandSimple(checkCommand);
}

void DependencyManager::setCustomInstallCommand(const std::string& dep,
                                                const std::string& command) {
    customInstallCommands_[dep] = command;
}

auto DependencyManager::generateDependencyReport() const -> std::string {
    std::ostringstream report;
    for (const auto& dep : dependencies_) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        report << "Dependency: " << dep.name;
        if (!dep.version.empty()) {
            report << " (" << dep.version << ")";
        }
        report << " - "
               << (installedCache_.at(dep.name) ? "Installed" : "Not Installed")
               << "\n";
    }
    return report.str();
}

void DependencyManager::loadCacheFromFile() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    std::ifstream cacheFile(CACHE_FILE);
    if (!cacheFile.is_open()) {
        return;
    }

    try {
        json j;
        cacheFile >> j;
        for (auto& [key, value] : j.items()) {
            installedCache_[key] = value.get<bool>();
        }
    } catch (const json::parse_error& ex) {
        log(LogLevel::ERROR,
            "Failed to parse cache file: " + std::string(ex.what()));
    }
}

void DependencyManager::saveCacheToFile() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    std::ofstream cacheFile(CACHE_FILE);
    if (!cacheFile.is_open()) {
        log(LogLevel::WARNING, "Failed to open cache file for writing.");
        return;
    }

    json j;
    for (const auto& [dep, status] : installedCache_) {
        j[dep] = status;
    }
    cacheFile << j.dump(4);
}

void DependencyManager::log(LogLevel level, const std::string& message) const {
    if (logCallback_) {
        logCallback_(level, message);
    } else {
        // 默认输出到标准输出
        switch (level) {
            case LogLevel::INFO:
                std::cout << "[INFO] " << message << "\n";
                break;
            case LogLevel::WARNING:
                std::cout << "[WARNING] " << message << "\n";
                break;
            case LogLevel::ERROR:
                std::cerr << "[ERROR] " << message << "\n";
                break;
        }
    }
}

auto DependencyManager::getCurrentPlatform() const -> std::string {
    switch (distroType_) {
        case DistroType::DEBIAN:
            return "Debian-based Linux";
        case DistroType::FEDORA:
            return "Fedora-based Linux";
        case DistroType::ARCH:
            return "Arch-based Linux";
        case DistroType::OPENSUSE:
            return "openSUSE";
        case DistroType::GENTOO:
            return "Gentoo";
        case DistroType::MACOS:
            return "macOS";
        case DistroType::WINDOWS:
            return "Windows";
        default:
            return "Unknown";
    }
}

}  // namespace lithium
