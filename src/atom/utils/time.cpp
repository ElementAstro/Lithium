/*
 * time.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Some useful functions about time

**************************************************/

#include "time.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace atom::utils {
std::string getTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << '.'
       << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string convertToChinaTime(const std::string &utcTimeStr) {
    // 解析UTC时间字符串
    std::tm tm = {};
    std::istringstream iss(utcTimeStr);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    // 转换为时间点
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    // 转换为东八区时间
    std::chrono::hours offset(8);
    auto local_tp =
        std::chrono::time_point_cast<std::chrono::hours>(tp) + offset;

    // 格式化为字符串
    auto local_time = std::chrono::system_clock::to_time_t(local_tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

std::string getChinaTimestampString() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为东八区时间点
    std::chrono::hours offset(8);
    auto local_tp =
        std::chrono::time_point_cast<std::chrono::hours>(now) + offset;

    // 格式化为字符串
    auto local_time = std::chrono::system_clock::to_time_t(local_tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

std::string timeStampToString(time_t timestamp) {
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
    return std::string(buffer);
}

// Specially for Astrometry.net
std::string toString(const std::tm &tm, const std::string &format) {
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

std::string getUtcTime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &now_time_t);
#else
    gmtime_r(&now_time_t, &tm);
#endif
    return toString(tm, "%FT%TZ");
}

std::tm timestampToTime(long long timestamp) {
    std::time_t time = static_cast<std::time_t>(timestamp / 1000);
    return *std::localtime(&time);
}
}  // namespace atom::utils
