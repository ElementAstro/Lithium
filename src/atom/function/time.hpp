/*!
 * \file time.hpp
 * \brief Record compile time
 * \author Max Qian <lightapt.com>
 * \date 2024-05-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_TIME_HPP
#define ATOM_META_TIME_HPP

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include "atom/macro.hpp"

namespace atom::meta {
ATOM_INLINE auto getCompileTime() -> std::string {
    std::string date = __DATE__;
    std::string time = __TIME__;
    std::istringstream dateStream(date);
    std::tm tm{};
    dateStream >> std::get_time(&tm, "%b %d %Y");
    std::istringstream timeStream(time);
    timeStream >> std::get_time(&tm, "%H:%M:%S");
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
}  // namespace atom::meta

#endif
