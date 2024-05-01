/*
 * crash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-4

Description: Crash Report

**************************************************/

#ifndef ATOM_SYSTEM_CRASH_HPP
#define ATOM_SYSTEM_CRASH_HPP

#include <string>

namespace atom::system {
/**
 * @brief 保存崩溃日志
 * @param error_msg 崩溃日志详细信息
 *
 * 该函数用于保存程序崩溃时的日志信息，方便后续调试和分析。
 *
 * @note
 * 调用该函数前需要确保崩溃日志已经被记录下来，否则调用该函数不会有任何效果。
 *
 * @param error_msg The detailed information of the crash log.
 *
 * This function is used to save the log information when the program crashes,
 * which is helpful for further debugging and analysis.
 *
 * @note Make sure the crash log has been recorded before calling this function,
 * otherwise calling this function will have no effect.
 */
void saveCrashLog(const std::string &error_msg);
}  // namespace atom::system

#endif
