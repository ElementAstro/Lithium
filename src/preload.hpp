#ifndef LITHIUM_PRELOAD_HPP
#define LITHIUM_PRELOAD_HPP

#include <memory>
#include <string>
#include <vector>

namespace lithium {
class Preloader {
public:
    Preloader();
    ~Preloader();

    // 禁止拷贝
    Preloader(const Preloader&) = delete;
    Preloader& operator=(const Preloader&) = delete;

    // 允许移动
    Preloader(Preloader&&) noexcept;
    Preloader& operator=(Preloader&&) noexcept;

    // 检查资源文件
    bool checkResources();

    // 下载缺失的资源文件
    void downloadResources();

    // 新增功能：获取下载进度
    double getDownloadProgress() const;

    // 新增功能：设置资源服务器地址
    void setResourceServer(const std::string& server);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
}  // namespace lithium

#endif  // LITHIUM_PRELOAD_HPP