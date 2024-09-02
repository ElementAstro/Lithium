/*
 * time.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Some useful functions about time

**************************************************/

#ifndef ATOM_UTILS_TIME_HPP
#define ATOM_UTILS_TIME_HPP

#include <ctime>
#include <string>

namespace atom::utils {
/**
 * @brief Retrieves the current timestamp as a formatted string.
 *
 * This function returns the current local time formatted as a string with the
 * pattern "%Y-%m-%d %H:%M:%S".
 *
 * @return std::string The current timestamp formatted as "%Y-%m-%d %H:%M:%S".
 */
[[nodiscard]] std::string getTimestampString();

/**
 * @brief Converts a UTC time string to China Standard Time (CST, UTC+8).
 *
 * This function takes a UTC time string formatted as "%Y-%m-%d %H:%M:%S" and
 * converts it to China Standard Time (CST), returning the time as a string with
 * the same format.
 *
 * @param utcTimeStr A string representing the UTC time in the format "%Y-%m-%d
 * %H:%M:%S".
 *
 * @return std::string The corresponding time in China Standard Time, formatted
 * as "%Y-%m-%d %H:%M:%S".
 */
[[nodiscard]] std::string convertToChinaTime(const std::string &utcTimeStr);

/**
 * @brief Retrieves the current China Standard Time (CST) as a formatted
 * timestamp string.
 *
 * This function returns the current local time in China Standard Time (CST),
 * formatted as a string with the pattern
 * "%Y-%m-%d %H:%M:%S".
 *
 * @return std::string The current China Standard Time formatted as "%Y-%m-%d
 * %H:%M:%S".
 */
[[nodiscard]] std::string getChinaTimestampString();

/**
 * @brief Converts a timestamp to a formatted string.
 *
 * This function takes a timestamp (in seconds since the Unix epoch) and
 * converts it to a string representation. The default format is "%Y-%m-%d
 * %H:%M:%S", but it may be adapted based on implementation details.
 *
 * @param timestamp The timestamp to be converted, typically expressed in
 * seconds since the Unix epoch.
 *
 * @return std::string The string representation of the timestamp.
 */
[[nodiscard]] std::string timeStampToString(time_t timestamp);

/**
 * @brief Converts a `tm` structure to a formatted string.
 *
 * This function takes a `std::tm` structure representing a date and time and
 * converts it to a formatted string according to the specified format.
 *
 * @param tm The `std::tm` structure to be converted to a string.
 * @param format A string representing the desired format for the output.
 *
 * @return std::string The formatted time string based on the `tm` structure and
 * format.
 */
[[nodiscard]] std::string toString(const std::tm &tm,
                                   const std::string &format);

/**
 * @brief Retrieves the current UTC time as a formatted string.
 *
 * This function returns the current UTC time formatted as a string with the
 * pattern "%Y-%m-%d %H:%M:%S".
 *
 * @return std::string The current UTC time formatted as "%Y-%m-%d %H:%M:%S".
 */
[[nodiscard]] std::string getUtcTime();

/**
 * @brief Converts a timestamp to a `tm` structure.
 *
 * This function takes a timestamp (in seconds since the Unix epoch) and
 * converts it to a `std::tm` structure, which represents a calendar date and
 * time.
 *
 * @param timestamp The timestamp to be converted, typically expressed in
 * seconds since the Unix epoch.
 *
 * @return std::tm The corresponding `std::tm` structure representing the
 * timestamp.
 */
[[nodiscard]] std::tm timestampToTime(long long timestamp);
}  // namespace atom::utils

#endif
