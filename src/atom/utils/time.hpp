/*
 * time.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Some useful functions about time

**************************************************/

#pragma once

#include <string>
#include <ctime>

namespace Atom::Utils
{
    /**
     * @brief 获取当前时间的字符串时间戳
     *
     * @return std::string 返回格式为"%Y-%m-%d %H:%M:%S"的字符串时间戳
     */
    std::string GetTimestampString();

    /**
     * @brief 将UTC时间转换为东八区时间
     *
     * @param utcTimeStr UTC时间字符串，格式为"%Y-%m-%d %H:%M:%S"
     * @return std::string 东八区时间字符串，格式为"%Y-%m-%d %H:%M:%S"
     */
    std::string ConvertToChinaTime(const std::string &utcTimeStr);

    /**
     * @brief 获取当前时间的东八区字符串时间戳
     *
     * @return std::string 返回格式为"%Y-%m-%d %H:%M:%S"的东八区字符串时间戳
     */
    std::string GetChinaTimestampString();

    /**
     * @brief 将时间戳转换为字符串
     *
     * @param timestamp 时间戳
     * @return std::string 时间戳对应的字符串
     */
    std::string TimeStampToString(time_t timestamp);

    /**
     * @brief 将tm结构体转换为字符串
     *
     * @param tm tm结构体
     * @param format 时间格式
     * @return std::string 时间字符串
     */
    std::string toString(const std::tm &tm, const std::string &format);
    
    /**
     * @brief 获取当前UTC时间
     *
     * @return std::string 返回格式为"%Y-%m-%d %H:%M:%S"的UTC时间字符串
     */
    std::string getUtcTime();
}
