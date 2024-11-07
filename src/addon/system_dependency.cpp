#include "system_dependency.hpp"

#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <unordered_map>

#if defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__)
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#else
#error "Unsupported platform"
#endif

#include "atom/system/command.hpp"
#include "atom/type/json.hpp"

namespace lithium {
using json = nlohmann::json;
class DependencyManager::Impl {
public:
    explicit Impl(std::vector<DependencyInfo> dependencies)
        : dependencies_(std::move(dependencies)) {
        detectPlatform();
        configurePackageManager();
        loadCacheFromFile();
    }

    ~Impl() { saveCacheToFile(); }

    void setLogCallback(
        std::function<void(LogLevel, const std::string&)> callback) {
        logCallback_ = std::move(callback);
    }

    void checkAndInstallDependencies() {
        std::vector<std::future<void>> futures;
        futures.reserve(dependencies_.size());
        for (const auto& dep : dependencies_) {
            futures.emplace_back(std::async(std::launch::async, [&]() {
                try {
                    if (!isDependencyInstalled(dep)) {
                        installDependency(dep);
                        log(LogLevel::INFO,
                            "Installed dependency: " + dep.name);
                    } else {
                        log(LogLevel::INFO,
                            "Dependency already installed: " + dep.name);
                    }
                } catch (const DependencyException& ex) {
                    log(LogLevel::ERROR, ex.what());
                }
            }));
        }

        for (auto& fut : futures) {
            if (fut.valid()) {
                fut.wait();
            }
        }
    }

    void installDependencyAsync(const DependencyInfo& dep) {
        std::lock_guard<std::mutex> lock(asyncMutex_);
        asyncFutures_.emplace_back(std::async(std::launch::async, [&]() {
            try {
                if (!isDependencyInstalled(dep)) {
                    installDependency(dep);
                    log(LogLevel::INFO, "Installed dependency: " + dep.name);
                } else {
                    log(LogLevel::INFO,
                        "Dependency already installed: " + dep.name);
                }
            } catch (const DependencyException& ex) {
                log(LogLevel::ERROR, ex.what());
            }
        }));
    }

    void cancelInstallation(const std::string& depName) {
        // 取消逻辑实现（示例中未具体实现）
        log(LogLevel::WARNING,
            "Cancel installation not implemented for: " + depName);
    }

    void setCustomInstallCommand(const std::string& dep,
                                 const std::string& command) {
        customInstallCommands_[dep] = command;
    }

    auto generateDependencyReport() -> std::string {
        std::ostringstream report;
        for (const auto& dep : dependencies_) {
            report << "Dependency: " << dep.name;
            if (!dep.version.empty()) {
                report << " | Version: " << dep.version;
            }
            report << " | Installed: "
                   << (isDependencyInstalled(dep) ? "Yes" : "No") << "\n";
        }
        return report.str();
    }

    void uninstallDependency(const std::string& depName) {
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
            uninstallDependencyInternal(depName);
            log(LogLevel::INFO, "Uninstalled dependency: " + depName);
        } catch (const DependencyException& ex) {
            log(LogLevel::ERROR, ex.what());
        }
    }

    auto getCurrentPlatform() const -> std::string {
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

private:
    std::vector<DependencyInfo> dependencies_;
    std::function<void(LogLevel, const std::string&)> logCallback_;
    std::unordered_map<std::string, bool> installedCache_;
    std::unordered_map<std::string, std::string> customInstallCommands_;
    mutable std::mutex cacheMutex_;
    std::mutex asyncMutex_;
    std::vector<std::future<void>> asyncFutures_;

    enum class DistroType {
        DEBIAN,
        FEDORA,
        ARCH,
        OPENSUSE,
        GENTOO,
        MACOS,
        WINDOWS,
        UNKNOWN
    };

    DistroType distroType_ = DistroType::UNKNOWN;

    struct PackageManager {
        std::function<std::string(const DependencyInfo&)> getCheckCommand;
        std::function<std::string(const DependencyInfo&)> getInstallCommand;
        std::function<std::string(const DependencyInfo&)> getUninstallCommand;
    };

    PackageManager packageManager_;

    const std::string CACHE_FILE = "dependency_cache.json";

    void detectPlatform() {
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

    void configurePackageManager() {
#ifdef PLATFORM_LINUX
        switch (distroType_) {
            case DistroType::DEBIAN:
                packageManager_.getCheckCommand =
                    [](const DependencyInfo& dep) -> std::string {
                    std::string cmd =
                        "dpkg -s " + dep.name + " > /dev/null 2>&1";
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
                    std::string cmd =
                        "rpm -q " + dep.name + " > /dev/null 2>&1";
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
                        "pacman -Qs " + dep.name + " > /dev/null 2>&1";
                    if (!dep.version.empty()) {
                        // Pacman 不直接支持版本查询，需自定义实现
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
                    return "sudo pacman -Rns --noconfirm " + dep.name;
                };
                break;
            case DistroType::OPENSUSE:
                packageManager_.getCheckCommand =
                    [](const DependencyInfo& dep) -> std::string {
                    std::string cmd =
                        "rpm -q " + dep.name + " > /dev/null 2>&1";
                    if (!dep.version.empty()) {
                        cmd += " && rpm -q " + dep.name + "-" + dep.version +
                               " > /dev/null 2>&1";
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
                        cmd += " && equery list " + dep.name + " | grep " +
                               dep.version;
                    }
                    return cmd;
                };
                packageManager_.getInstallCommand =
                    [this](const DependencyInfo& dep) -> std::string {
                    if (!customInstallCommands_.contains(dep.name)) {
                        return "sudo emerge " + dep.name +
                               (dep.version.empty() ? "" : "-" + dep.version);
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
                    std::string cmd =
                        "dpkg -s " + dep.name + " > /dev/null 2>&1";
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
            if (!dep.version.empty()) {
                return "choco list --local-only " + dep.name + " | findstr " +
                       dep.version;
            } else {
                return "choco list --local-only " + dep.name + " > nul 2>&1";
            }
        };
        packageManager_.getInstallCommand =
            [this](const DependencyInfo& dep) -> std::string {
            if (customInstallCommands_.count(dep.name)) {
                return customInstallCommands_.at(dep.name);
            }
            if (isCommandAvailable("choco")) {
                return "choco install " + dep.name + " -y" +
                       (dep.version.empty() ? "" : " --version " + dep.version);
            } else if (isCommandAvailable("winget")) {
                return "winget install " + dep.name +
                       (dep.version.empty() ? "" : " --version " + dep.version);
            } else if (isCommandAvailable("scoop")) {
                return "scoop install " + dep.name;
            } else {
                throw DependencyException(
                    "No supported package manager found.");
            }
        };
        packageManager_.getUninstallCommand =
            [this](const DependencyInfo& dep) -> std::string {
            if (customInstallCommands_.count(dep.name)) {
                return customInstallCommands_.at(dep.name);
            }
            if (isCommandAvailable("choco")) {
                return "choco uninstall " + dep.name + " -y";
            } else if (isCommandAvailable("winget")) {
                return "winget uninstall " + dep.name;
            } else if (isCommandAvailable("scoop")) {
                return "scoop uninstall " + dep.name;
            } else {
                throw DependencyException(
                    "No supported package manager found.");
            }
        };
#endif
    }

    void checkAndInstallDependenciesOptimized() {
        // 优化后的依赖检查和安装逻辑
    }

    bool isDependencyInstalled(const DependencyInfo& dep) {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = installedCache_.find(dep.name);
        if (it != installedCache_.end()) {
            return it->second;
        }

        std::string checkCommand = packageManager_.getCheckCommand(dep);
        bool isInstalled = false;
        try {
            isInstalled = atom::system::executeCommandSimple(checkCommand);
        } catch (const std::exception& ex) {
            log(LogLevel::ERROR,
                "Error checking dependency " + dep.name + ": " + ex.what());
            isInstalled = false;
        }
        installedCache_[dep.name] = isInstalled;
        return isInstalled;
    }

    void installDependency(const DependencyInfo& dep) {
        std::string installCommand = packageManager_.getInstallCommand(dep);
        bool success = false;
        try {
            success = atom::system::executeCommandSimple(installCommand);
        } catch (const std::exception& ex) {
            throw DependencyException("Failed to install " + dep.name + ": " +
                                      ex.what());
        }

        if (!success) {
            throw DependencyException("Failed to install " + dep.name);
        }

        // 更新缓存
        std::lock_guard<std::mutex> lock(cacheMutex_);
        installedCache_[dep.name] = true;
    }

    void uninstallDependencyInternal(const std::string& depName) {
        auto it = std::find_if(dependencies_.begin(), dependencies_.end(),
                               [&depName](const DependencyInfo& info) {
                                   return info.name == depName;
                               });
        if (it == dependencies_.end()) {
            throw DependencyException("Dependency " + depName + " not found.");
        }

        std::string uninstallCommand = packageManager_.getUninstallCommand(*it);
        bool success = false;
        try {
            success = atom::system::executeCommandSimple(uninstallCommand);
        } catch (const std::exception& ex) {
            throw DependencyException("Failed to uninstall " + depName + ": " +
                                      ex.what());
        }

        if (!success) {
            throw DependencyException("Failed to uninstall " + depName);
        }

        // 更新缓存
        std::lock_guard<std::mutex> lock(cacheMutex_);
        installedCache_[depName] = false;
    }

    static auto isCommandAvailable(const std::string& command) -> bool {
        std::string checkCommand;
#ifdef PLATFORM_WINDOWS
        checkCommand = "where " + command + " > nul 2>&1";
#else
        checkCommand = "command -v " + command + " > /dev/null 2>&1";
#endif
        return atom::system::executeCommandSimple(checkCommand);
    }

    void loadCacheFromFile() {
        std::lock_guard<std::mutex> lock(cacheMutex_);
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
            log(LogLevel::WARNING,
                "Failed to parse cache file: " + std::string(ex.what()));
        }
    }

    void saveCacheToFile() const {
        std::lock_guard<std::mutex> lock(cacheMutex_);
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

    void log(LogLevel level, const std::string& message) const {
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
};

DependencyManager::DependencyManager(std::vector<DependencyInfo> dependencies)
    : pImpl_(std::make_unique<Impl>(std::move(dependencies))) {}

DependencyManager::~DependencyManager() = default;

void DependencyManager::setLogCallback(
    std::function<void(LogLevel, const std::string&)> callback) {
    pImpl_->setLogCallback(std::move(callback));
}

void DependencyManager::checkAndInstallDependencies() {
    pImpl_->checkAndInstallDependencies();
}

void DependencyManager::installDependencyAsync(const DependencyInfo& dep) {
    pImpl_->installDependencyAsync(dep);
}

void DependencyManager::cancelInstallation(const std::string& dep) {
    pImpl_->cancelInstallation(dep);
}

void DependencyManager::setCustomInstallCommand(const std::string& dep,
                                                const std::string& command) {
    pImpl_->setCustomInstallCommand(dep, command);
}

auto DependencyManager::generateDependencyReport() const -> std::string {
    return pImpl_->generateDependencyReport();
}

void DependencyManager::uninstallDependency(const std::string& dep) {
    pImpl_->uninstallDependency(dep);
}

auto DependencyManager::getCurrentPlatform() const -> std::string {
    return pImpl_->getCurrentPlatform();
}

}  // namespace lithium
