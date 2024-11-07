#ifndef LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP
#define LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP

#include <exception>
#include <functional>
#include <memory>
#include <string>
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
struct DependencyInfo {
    std::string name;
    std::string version;  // 可选
};

// 依赖管理器类
class DependencyManager {
public:
    explicit DependencyManager(std::vector<DependencyInfo> dependencies);
    ~DependencyManager();

    // 禁用拷贝和赋值
    DependencyManager(const DependencyManager&) = delete;
    DependencyManager& operator=(const DependencyManager&) = delete;

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

    // 异步安装依赖项
    void installDependencyAsync(const DependencyInfo& dep);

    // 取消安装操作
    void cancelInstallation(const std::string& dep);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_DEPENDENCY_MANAGER_HPP