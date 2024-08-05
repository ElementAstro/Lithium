/*
 * time.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-31

Description: Time

**************************************************/

#ifndef ATOM_WEB_TIME_HPP
#define ATOM_WEB_TIME_HPP

#include <ctime>
#include <memory>
#include <string>

namespace atom::web {

class TimeManagerImpl;

class TimeManager {
public:
    TimeManager();
    ~TimeManager();

    auto getSystemTime() -> std::time_t;

    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second);

    auto setSystemTimezone(const std::string &timezone) -> bool;

    auto syncTimeFromRTC() -> bool;

    auto getNtpTime(const std::string &hostname) -> std::time_t;

private:
    std::unique_ptr<TimeManagerImpl> impl_;
};

}  // namespace atom::web

#endif
