#ifndef LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP
#define LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP

#include <exception>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lithium {

// 日志级别定义
enum class LogLevel { INFO, WARNING, ERROR };

// 自定义异常类
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
struct alignas(64) DependencyInfo {
    std::string name;
    std::string version;  // 可选
};

// 依赖管理器类
class DependencyManager {
public:
    explicit DependencyManager(std::vector<DependencyInfo> dependencies);

    // 设置日志回调函数，包含日志级别
    void setLogCallback(
        std::function<void(LogLevel, const std::string&)> callback);

    // 检查并安装所有依赖项
    void checkAndInstallDependencies();

    // 设置自定义安装命令
    void setCustomInstallCommand(const std::string& dep,
                                 const std::string& command);

    // 生成依赖项报告
    auto generateDependencyReport() const -> std::string;

    // 卸载依赖项
    void uninstallDependency(const std::string& dep);

    // 获取当前支持的平台类型
    auto getCurrentPlatform() const -> std::string;

private:
    std::vector<DependencyInfo> dependencies_;
    std::function<void(LogLevel, const std::string&)> logCallback_;
    std::unordered_map<std::string, bool> installedCache_;
    std::unordered_map<std::string, std::string> customInstallCommands_;

    // 系统发行版类型
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

    // 检测当前的操作系统和发行版
    void detectPlatform();

    // 检查依赖项是否已安装
    auto isDependencyInstalled(const DependencyInfo& dep) -> bool;

    // 安装依赖项
    void installDependency(const DependencyInfo& dep);

    // 卸载依赖项
    void uninstallDependencyInternal(const std::string& dep);

    // 根据平台获取检查、安装和卸载命令
    auto getCheckCommand(const DependencyInfo& dep) const -> std::string;
    auto getInstallCommand(const DependencyInfo& dep) const -> std::string;
    auto getUninstallCommand(const DependencyInfo& dep) const -> std::string;

    // 检查命令是否可用
    auto isCommandAvailable(const std::string& command) const -> bool;

    // 从文件加载缓存
    void loadCacheFromFile();

    // 保存缓存到文件
    void saveCacheToFile() const;

    // 日志记录函数
    void log(LogLevel level, const std::string& message) const;

    // 包管理器接口
    struct alignas(128) PackageManager {
        std::function<std::string(const DependencyInfo&)> getCheckCommand;
        std::function<std::string(const DependencyInfo&)> getInstallCommand;
        std::function<std::string(const DependencyInfo&)> getUninstallCommand;
    };

    PackageManager packageManager_;

    // 根据发行版设置包管理器命令
    void configurePackageManager();
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP