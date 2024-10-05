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

/**
 * @class TimeManagerImpl
 * @brief Forward declaration of the implementation class for TimeManager.
 */
class TimeManagerImpl;

/**
 * @class TimeManager
 * @brief A class for managing system time and synchronization.
 */
class TimeManager {
public:
    /**
     * @brief Constructs a TimeManager.
     */
    TimeManager();

    /**
     * @brief Destructor to release resources.
     */
    ~TimeManager();

    /**
     * @brief Gets the current system time.
     * @return The current system time as std::time_t.
     */
    auto getSystemTime() -> std::time_t;

    /**
     * @brief Sets the system time.
     * @param year The year to set.
     * @param month The month to set.
     * @param day The day to set.
     * @param hour The hour to set.
     * @param minute The minute to set.
     * @param second The second to set.
     */
    void setSystemTime(int year, int month, int day, int hour, int minute,
                       int second);

    /**
     * @brief Sets the system timezone.
     * @param timezone The timezone to set (e.g., "UTC", "PST").
     * @return True if the timezone was successfully set, false otherwise.
     */
    auto setSystemTimezone(const std::string &timezone) -> bool;

    /**
     * @brief Synchronizes the system time from the Real-Time Clock (RTC).
     * @return True if the time was successfully synchronized, false otherwise.
     */
    auto syncTimeFromRTC() -> bool;

    /**
     * @brief Gets the Network Time Protocol (NTP) time from a specified
     * hostname.
     * @param hostname The NTP server hostname.
     * @return The NTP time as std::time_t.
     */
    auto getNtpTime(const std::string &hostname) -> std::time_t;

private:
    std::unique_ptr<TimeManagerImpl>
        impl_;  ///< Pointer to the implementation, using Pimpl idiom to hide
                ///< details.
};

}  // namespace atom::web

#endif  // ATOM_WEB_TIME_HPP