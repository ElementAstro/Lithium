/*
 * storage.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-5

Description: Storage Monitor

**************************************************/

#ifndef ATOM_SYSTEM_STORAGE_HPP
#define ATOM_SYSTEM_STORAGE_HPP

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <condition_variable>
#include "atom/macro.hpp"

namespace atom::system {
/**
 * @brief 监控存储空间变化的类。
 *
 * 这个类可以监控所有已挂载设备的存储空间使用情况，并在存储空间发生变化时触发注册的回调函数。
 */
class StorageMonitor {
public:
    /**
     * @brief 默认构造函数。
     */
    StorageMonitor();

    ~StorageMonitor();

    /**
     * @brief 注册回调函数。
     *
     * @param callback 回调函数，当存储空间发生变化时会被触发。
     */
    void registerCallback(std::function<void(const std::string &)> callback);

    /**
     * @brief 启动存储空间监控。
     *
     * @return 成功返回true，否则返回false。
     */
    ATOM_NODISCARD auto startMonitoring() -> bool;

    /**
     * @brief 停止存储空间监控。
     */
    void stopMonitoring();

    /**
     * @brief 检查是否正在运行监控。
     *
     * @return 如果正在运行则返回true，否则返回false。
     */
    ATOM_NODISCARD auto isRunning() const -> bool;

    /**
     * @brief 触发所有注册的回调函数。
     *
     * @param path 存储空间路径。
     */
    void triggerCallbacks(const std::string &path);

    /**
     * @brief 检查指定路径是否有新的存储设备插入。
     *
     * @param path 存储空间路径。
     * @return 如果有新的存储设备插入则返回true，否则返回false。
     */
    ATOM_NODISCARD auto isNewMediaInserted(const std::string &path) -> bool;

    /**
     * @brief 列举所有已挂载的存储空间。
     */
    void listAllStorage();

    /**
     * @brief 列举指定路径下的文件列表。
     *
     * @param path 存储空间路径。
     */
    void listFiles(const std::string &path);

    /**
     * @brief 动态添加存储路径。
     *
     * @param path 要添加的存储路径。
     */
    void addStoragePath(const std::string &path);

    /**
     * @brief 动态移除存储路径。
     *
     * @param path 要移除的存储路径。
     */
    void removeStoragePath(const std::string &path);

    /**
     * @brief 获取当前存储状态。
     *
     * @return 存储状态的字符串表示。
     */
    std::string getStorageStatus();

private:
    std::vector<std::string> m_storagePaths;  ///< 所有已挂载的存储空间路径。
    std::unordered_map<std::string, std::pair<uintmax_t, uintmax_t>>
        m_storageStats;
    std::mutex m_mutex;  ///< 互斥锁，用于保护数据结构的线程安全。
    std::vector<std::function<void(const std::string &)>> m_callbacks; ///< 注册的回调函数列表。
    bool m_isRunning;  ///< 标记是否正在运行监控。
    std::thread m_monitorThread; ///< 监控线程。
    std::condition_variable m_cv; ///< 条件变量用于线程同步。
};

#ifdef _WIN32
static void monitorUdisk();
#else
void monitorUdisk(atom::system::StorageMonitor& monitor);
#endif

}  // namespace atom::system

#endif