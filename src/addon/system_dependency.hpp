#ifndef LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP
#define LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lithium {

class DependencyException : public std::exception {
public:
    explicit DependencyException(std::string message)
        : message_(std::move(message)) {}
    [[nodiscard]] auto what() const noexcept -> const char* override {
        return message_.c_str();
    }

private:
    std::string message_;
};

// 依赖项信息结构
struct DependencyInfo {
    std::string name;
    std::string version;         // 可选
    std::string packageManager;  // 指定的包管理器
};

// 包管理器信息结构
struct PackageManagerInfo {
    std::string name;
    std::function<std::string(const DependencyInfo&)> getCheckCommand;
    std::function<std::string(const DependencyInfo&)> getInstallCommand;
    std::function<std::string(const DependencyInfo&)> getUninstallCommand;
    std::function<std::string(const std::string&)> getSearchCommand;
};

// 依赖管理器类
class DependencyManager {
public:
    DependencyManager();
    ~DependencyManager();

    DependencyManager(const DependencyManager&) = delete;
    DependencyManager& operator=(const DependencyManager&) = delete;

    void checkAndInstallDependencies();
    void setCustomInstallCommand(const std::string& dep,
                                 const std::string& command);
    auto generateDependencyReport() const -> std::string;
    void uninstallDependency(const std::string& dep);
    auto getCurrentPlatform() const -> std::string;
    void installDependencyAsync(const DependencyInfo& dep);
    void cancelInstallation(const std::string& dep);
    void addDependency(const DependencyInfo& dep);
    void removeDependency(const std::string& depName);
    auto searchDependency(const std::string& depName)
        -> std::vector<std::string>;
    void loadSystemPackageManagers();
    auto getPackageManagers() const -> std::vector<PackageManagerInfo>;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP