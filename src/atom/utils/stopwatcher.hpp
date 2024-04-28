/*
 * stopwatcher.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-25

Description: Simple implementation of a stopwatch

**************************************************/

#ifndef ATOM_UTILS_STOPWATCHER_HPP
#define ATOM_UTILS_STOPWATCHER_HPP

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace atom::utils {
/**
 * @brief 用于计时的类。
 *
 * StopWatcher类用于测量代码段的执行时间，并提供了一些功能，如暂停、恢复、重置等。
 */
class StopWatcher {
public:
    /**
     * @brief 构造函数。
     *
     * 创建一个StopWatcher对象。
     */
    StopWatcher();

    /**
     * @brief 启动计时器。
     *
     * 开始计算代码段的执行时间。
     */
    void start();

    /**
     * @brief 停止计时器。
     *
     * 停止计算代码段的执行时间，并记录结束时间点。
     */
    void stop();

    /**
     * @brief 暂停计时器。
     *
     * 暂停计算代码段的执行时间，并记录暂停时间点。
     */
    void pause();

    /**
     * @brief 恢复计时器。
     *
     * 恢复计算代码段的执行时间，继续计时。
     */
    void resume();

    /**
     * @brief 重置计时器。
     *
     * 重置计时器，清空所有记录的时间点和回调函数。
     */
    void reset();

    /**
     * @brief 获取经过的时间（毫秒）。
     *
     * 返回代码段执行的总时间，以毫秒为单位。
     *
     * @return 经过的时间，以毫秒为单位。
     */
    double elapsedMilliseconds() const;

    /**
     * @brief 获取经过的时间（秒）。
     *
     * 返回代码段执行的总时间，以秒为单位。
     *
     * @return 经过的时间，以秒为单位。
     */
    double elapsedSeconds() const;

    /**
     * @brief 获取格式化的经过时间。
     *
     * 返回格式化后的代码段执行时间，以字符串形式表示，如"2 hours 30 minutes 45
     * seconds"。
     *
     * @return 格式化后的经过时间。
     */
    std::string elapsedFormatted() const;

    /**
     * @brief 注册回调函数。
     *
     * 注册一个回调函数，在指定的时间间隔内触发。
     *
     * @param callback 回调函数。
     * @param milliseconds 时间间隔（毫秒）。
     */
    void registerCallback(std::function<void()> callback, int milliseconds);

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end,
        m_pauseTime;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>>
        m_intervals;
    bool m_running, m_paused;
    std::vector<std::pair<std::function<void()>, int>> m_callbacks;

    /**
     * @brief 检查并触发回调函数。
     *
     * 根据当前时间点，检查是否需要触发注册的回调函数。
     *
     * @param currentTime 当前时间点。
     */
    void checkCallbacks(
        const std::chrono::time_point<std::chrono::high_resolution_clock>
            &currentTime);
};
}  // namespace atom::utils

#endif