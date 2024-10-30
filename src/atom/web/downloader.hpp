#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>

#ifdef USE_ASIO
#include <asio.hpp>
#endif

namespace atom::web {

/**
 * @class DownloadManager
 * @brief 管理下载任务的类，使用 Pimpl 习语隐藏实现细节。
 */
class DownloadManager {
public:
    /**
     * @brief 构造函数。
     * @param task_file 保存下载任务列表的文件路径。
     */
    explicit DownloadManager(const std::string& task_file);

    /**
     * @brief 析构函数，释放资源。
     */
    ~DownloadManager();

    // 禁用复制构造和赋值运算符
    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;

    // 启用移动构造和赋值运算符
    DownloadManager(DownloadManager&&) noexcept = default;
    DownloadManager& operator=(DownloadManager&&) noexcept = default;

    /**
     * @brief 添加下载任务。
     * @param url 下载链接。
     * @param filepath 保存下载文件的路径。
     * @param priority 下载任务的优先级，数值越大优先级越高。
     */
    void addTask(const std::string& url, const std::string& filepath,
                 int priority = 0);

    /**
     * @brief 移除下载任务。
     * @param index 任务列表中的索引。
     * @return 如果成功移除，返回 true；否则返回 false。
     */
    bool removeTask(size_t index);

    /**
     * @brief 开始下载任务。
     * @param thread_count 下载线程数，默认为 CPU 核心数。
     * @param download_speed 下载限速（字节每秒），0 表示不限速。
     */
    void start(size_t thread_count = std::thread::hardware_concurrency(),
               size_t download_speed = 0);

    /**
     * @brief 暂停下载任务。
     * @param index 任务列表中的索引。
     */
    void pauseTask(size_t index);

    /**
     * @brief 恢复已暂停的下载任务。
     * @param index 任务列表中的索引。
     */
    void resumeTask(size_t index);

    /**
     * @brief 获取任务已下载的字节数。
     * @param index 任务列表中的索引。
     * @return 已下载的字节数。
     */
    size_t getDownloadedBytes(size_t index);

    /**
     * @brief 取消下载任务。
     * @param index 任务列表中的索引。
     */
    void cancelTask(size_t index);

    /**
     * @brief 动态调整下载线程数。
     * @param thread_count 新的下载线程数。
     */
    void setThreadCount(size_t thread_count);

    /**
     * @brief 设置下载错误的最大重试次数。
     * @param retries 每个任务失败时的最大重试次数。
     */
    void setMaxRetries(size_t retries);

    /**
     * @brief 注册下载完成时的回调函数。
     * @param callback 回调函数，参数为任务索引。
     */
    void onDownloadComplete(const std::function<void(size_t)>& callback);

    /**
     * @brief 注册下载进度更新时的回调函数。
     * @param callback 回调函数，参数为任务索引和下载百分比。
     */
    void onProgressUpdate(const std::function<void(size_t, double)>& callback);

    /**
     * @class Impl
     * @brief DownloadManager 的实现类，用于隐藏实现细节。
     */
    class Impl;

private:
    std::unique_ptr<Impl> impl_;  ///< 实现的指针，使用 Pimpl 习语隐藏细节。
};

}  // namespace atom::web