#include "qtimezone.hpp"
#include "qdatetime.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

namespace atom::utils {

QTimeZone::QTimeZone() : timeZoneId_("UTC"), offset_(std::chrono::seconds(0)) {}

QTimeZone::QTimeZone(const std::string& timeZoneId_) {
    auto availableIds = availableTimeZoneIds();
    if (std::find(availableIds.begin(), availableIds.end(), timeZoneId_) ==
        availableIds.end()) {
        THROW_INVALID_ARGUMENT("Invalid time zone ID");
    }
    this->timeZoneId_ = timeZoneId_;

    std::tm localTime = {};
    std::time_t currentTime = std::time(nullptr);
#ifdef _WIN32
    if (localtime_s(&localTime, &currentTime) != 0) {
        THROW_GET_TIME_ERROR("Failed to get local time");
    }
#else
    if (localtime_r(&currentTime, &localTime) == nullptr) {
        THROW_GET_TIME_ERROR("Failed to get local time");
    }
#endif

    std::tm utcTime = {};
#ifdef _WIN32
    if (gmtime_s(&utcTime, &currentTime) != 0) {
        THROW_GET_TIME_ERROR("Failed to get UTC time");
    }
#else
    if (gmtime_r(&currentTime, &utcTime) == nullptr) {
        THROW_GET_TIME_ERROR("Failed to get UTC time");
    }
#endif

    auto localTimeT = std::mktime(&localTime);
    auto utcTimeT = std::mktime(&utcTime);
    offset_ = std::chrono::seconds(localTimeT - utcTimeT);
}

auto QTimeZone::availableTimeZoneIds() -> std::vector<std::string> {
    return {"UTC", "PST", "EST", "CST", "MST"};
}

auto QTimeZone::id() const -> std::string { return timeZoneId_; }

auto QTimeZone::displayName() const -> std::string {
    if (timeZoneId_ == "UTC") {
        return "Coordinated Universal Time";
    }
    if (timeZoneId_ == "PST") {
        return "Pacific Standard Time";
    }
    if (timeZoneId_ == "EST") {
        return "Eastern Standard Time";
    }
    if (timeZoneId_ == "CST") {
        return "Central Standard Time";
    }
    if (timeZoneId_ == "MST") {
        return "Mountain Standard Time";
    }
    return "";
}

auto QTimeZone::isValid() const -> bool { return offset_.has_value(); }

auto QTimeZone::offsetFromUtc(const QDateTime& dateTime) const
    -> std::chrono::seconds {
    std::time_t currentTime = dateTime.toTimeT();
    std::tm utcTime = *gmtime(&currentTime);

    std::tm localTime = utcTime;
    localTime.tm_sec += static_cast<int>(offset_.value().count());
    if (mktime(&localTime) == -1) {
        THROW_GET_TIME_ERROR("Failed to convert time");
    }

    if (hasDaylightTime() && isDaylightTime(dateTime)) {
        localTime.tm_sec += static_cast<int>(daylightTimeOffset().count());
        if (mktime(&localTime) == -1) {
            THROW_GET_TIME_ERROR("Failed to convert time");
        }
    }

    return std::chrono::seconds(mktime(&localTime) - mktime(&utcTime));
}

auto QTimeZone::standardTimeOffset() const -> std::chrono::seconds {
    return offset_.value_or(std::chrono::seconds(0));
}

auto QTimeZone::daylightTimeOffset() const -> std::chrono::seconds {
    static constexpr int K_ONE_HOUR_IN_SECONDS = 3600;
    if (timeZoneId_ == "PST" || timeZoneId_ == "EST" || timeZoneId_ == "CST" ||
        timeZoneId_ == "MST") {
        return std::chrono::seconds(K_ONE_HOUR_IN_SECONDS);
    }
    return std::chrono::seconds(0);
}

auto QTimeZone::hasDaylightTime() const -> bool {
    return timeZoneId_ == "PST" || timeZoneId_ == "EST" ||
           timeZoneId_ == "CST" || timeZoneId_ == "MST";
}

auto QTimeZone::isDaylightTime(const QDateTime& dateTime) const -> bool {
    if (!hasDaylightTime()) {
        return false;
    }

    std::time_t currentTime = dateTime.toTimeT();
    std::tm localTime = *localtime(&currentTime);

    static constexpr int K_MARCH = 2;
    static constexpr int K_NOVEMBER = 10;
    static constexpr int K_SECOND_SUNDAY = 8;
    static constexpr int K_FIRST_SUNDAY = 1;
    static constexpr int K_ONE_WEEK = 7;

    std::tm startDST = {0, 0, 2, K_SECOND_SUNDAY, K_MARCH, localTime.tm_year,
                        0, 0, -1};  // March 8th 2:00 AM
    std::tm endDST = {0, 0, 2, K_FIRST_SUNDAY, K_NOVEMBER, localTime.tm_year,
                      0, 0, -1};  // November 1st 2:00 AM

    while (startDST.tm_wday != 0) {
        startDST.tm_mday += 1;
        if (mktime(&startDST) == -1) {
            THROW_GET_TIME_ERROR("Failed to convert time");
        }
    }
    startDST.tm_mday += K_ONE_WEEK;
    if (mktime(&startDST) == -1) {
        THROW_GET_TIME_ERROR("Failed to convert time");
    }

    while (endDST.tm_wday != 0) {
        endDST.tm_mday += 1;
        if (mktime(&endDST) == -1) {
            THROW_GET_TIME_ERROR("Failed to convert time");
        }
    }

    std::time_t startDstT = mktime(&startDST);
    std::time_t endDstT = mktime(&endDST);

    return (currentTime >= startDstT && currentTime < endDstT);
}

}  // namespace atom::utils