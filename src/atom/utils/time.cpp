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

#include <array>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace atom::utils {
constexpr int K_MILLISECONDS_IN_SECOND =
    1000;  // Named constant for magic number
constexpr int K_CHINA_TIMEZONE_OFFSET = 8;

auto getTimestampString() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()) %
                        K_MILLISECONDS_IN_SECOND;

    std::tm timeInfo{};
#ifdef _WIN32
    if (localtime_s(&timeInfo, &time) != 0) {
#else
    if (localtime_r(&time, &timeInfo) == nullptr) {
#endif
        THROW_TIME_CONVERT_ERROR("Failed to convert time to local time");
    }

    std::stringstream timestampStream;
    timestampStream << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << '.'
                    << std::setfill('0') << std::setw(3)
                    << milliseconds.count();

    return timestampStream.str();
}

auto convertToChinaTime(const std::string &utcTimeStr) -> std::string {
    // 解析UTC时间字符串
    std::tm timeStruct = {};
    std::istringstream inputStream(utcTimeStr);
    inputStream >> std::get_time(&timeStruct, "%Y-%m-%d %H:%M:%S");

    // 转换为时间点
    auto timePoint =
        std::chrono::system_clock::from_time_t(std::mktime(&timeStruct));

    std::chrono::hours offset(K_CHINA_TIMEZONE_OFFSET);
    auto localTimePoint =
        std::chrono::time_point_cast<std::chrono::hours>(timePoint) + offset;

    // 格式化为字符串
    auto localTime = std::chrono::system_clock::to_time_t(localTimePoint);
    std::tm localTimeStruct{};
#ifdef _WIN32
    if (localtime_s(&localTimeStruct, &localTime) != 0) {
#else
    if (localtime_r(&localTime, &localTimeStruct) == nullptr) {
#endif
        THROW_TIME_CONVERT_ERROR("Failed to convert time to local time");
    }

    std::stringstream outputStream;
    outputStream << std::put_time(&localTimeStruct, "%Y-%m-%d %H:%M:%S");

    return outputStream.str();
}

auto getChinaTimestampString() -> std::string {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为东八区时间点
    std::chrono::hours offset(K_CHINA_TIMEZONE_OFFSET);
    auto localTimePoint =
        std::chrono::time_point_cast<std::chrono::hours>(now) + offset;

    // 格式化为字符串
    auto localTime = std::chrono::system_clock::to_time_t(localTimePoint);
    std::tm localTimeStruct{};
#ifdef _WIN32
    if (localtime_s(&localTimeStruct, &localTime) != 0) {
#else
    if (localtime_r(&localTime, &localTimeStruct) == nullptr) {
#endif
        THROW_TIME_CONVERT_ERROR("Failed to convert time to local time");
    }

    std::stringstream timestampStream;
    timestampStream << std::put_time(&localTimeStruct, "%Y-%m-%d %H:%M:%S");

    return timestampStream.str();
}

auto timeStampToString(time_t timestamp) -> std::string {
    constexpr size_t K_BUFFER_SIZE = 80;  // Named constant for magic number
    std::array<char, K_BUFFER_SIZE> buffer{};
    std::tm timeStruct{};
#ifdef _WIN32
    if (localtime_s(&timeStruct, &timestamp) != 0) {
#else
    if (localtime_r(&timestamp, &timeStruct) == nullptr) {
#endif
        THROW_TIME_CONVERT_ERROR("Failed to convert timestamp to local time");
    }

    if (std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S",
                      &timeStruct) == 0) {
        THROW_TIME_CONVERT_ERROR("strftime failed");
    }

    return std::string(buffer.data());
}

// Specially for Astrometry.net
auto toString(const std::tm &tm, const std::string &format) -> std::string {
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

auto getUtcTime() -> std::string {
    const auto NOW = std::chrono::system_clock::now();
    const std::time_t NOW_TIME_T = std::chrono::system_clock::to_time_t(NOW);
    std::tm utcTime;
#ifdef _WIN32
    if (gmtime_s(&utcTime, &NOW_TIME_T) != 0) {
        THROW_TIME_CONVERT_ERROR("Failed to convert time to UTC");
    }
#else
    gmtime_r(&NOW_TIME_T, &utcTime);
#endif
    return toString(utcTime, "%FT%TZ");
}

auto timestampToTime(long long timestamp) -> std::tm {
    auto time = static_cast<std::time_t>(timestamp / K_MILLISECONDS_IN_SECOND);

    std::tm timeStruct;
#ifdef _WIN32
    if (localtime_s(&timeStruct, &time) != 0) {
#else
    if (localtime_r(&time, &timeStruct) == nullptr) {
#endif
        THROW_TIME_CONVERT_ERROR("Failed to convert timestamp to local time");
    }  // Use localtime_s for thread safety

    return timeStruct;
}
}  // namespace atom::utils
