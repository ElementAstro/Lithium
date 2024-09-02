#include "qtimezone.hpp"
#include "qdatetime.hpp"

#include <string>

#include "atom/error/exception.hpp"

namespace atom::utils {

QTimeZone::QTimeZone() : timeZoneId_("UTC"), offset_(std::chrono::seconds(0)) {}

QTimeZone::QTimeZone(const std::string& timeZoneId_) {
    auto availableIds = availableTimeZoneIds();
    if (std::find(availableIds.begin(), availableIds.end(), timeZoneId_) ==
        availableIds.end()) {
        throw std::invalid_argument("Invalid time zone ID");
    }
    this->timeZoneId_ = timeZoneId_;

    std::tm localTime = {};
    std::time_t currentTime = std::time(nullptr);
#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    std::tm utcTime = {};
#ifdef _WIN32
    gmtime_s(&utcTime, &currentTime);
#else
    gmtime_r(&currentTime, &utcTime);
#endif

    auto localTimeT = std::mktime(&localTime);
    auto utcTimeT = std::mktime(&utcTime);
    offset_ = std::chrono::seconds(localTimeT - utcTimeT);
}

auto QTimeZone::availableTimeZoneIds() -> std::vector<std::string> {
    return {"UTC", "PST", "EST", "CST", "MST"};
}

std::string QTimeZone::id() const { return timeZoneId_; }

std::string QTimeZone::displayName() const {
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
    // Assuming QDateTime has a method to return time_t representation
    std::time_t currentTime = dateTime.toTimeT();
    std::tm utcTime = *gmtime(&currentTime);

    std::tm localTime = utcTime;
    localTime.tm_sec += offset_.value().count();
    mktime(&localTime);

    // Implement DST check and adjustment if applicable
    if (hasDaylightTime() && isDaylightTime(dateTime)) {
        localTime.tm_sec += daylightTimeOffset().count();
        mktime(&localTime);
    }

    return std::chrono::seconds(mktime(&localTime) - mktime(&utcTime));
}

auto QTimeZone::standardTimeOffset() const -> std::chrono::seconds {
    return offset_.value_or(std::chrono::seconds(0));
}

auto QTimeZone::daylightTimeOffset() const -> std::chrono::seconds {
    if (timeZoneId_ == "PST" || timeZoneId_ == "EST" || timeZoneId_ == "CST" ||
        timeZoneId_ == "MST") {
        return std::chrono::seconds(3600);  // 1 hour for these time zones
    }
    return std::chrono::seconds(0);
}

auto QTimeZone::hasDaylightTime() const -> bool {
    return timeZoneId_ == "PST" || timeZoneId_ == "EST" ||
           timeZoneId_ == "CST" || timeZoneId_ == "MST";
}

auto QTimeZone::isDaylightTime(const QDateTime& dateTime) const -> bool {
    if (!hasDaylightTime())
        return false;

    // Implement logic to check if the given dateTime falls under DST for the
    // timezone.
    std::time_t currentTime = dateTime.toTimeT();
    std::tm localTime = *localtime(&currentTime);

    // For simplicity, assume DST starts on the second Sunday in March and ends
    // on the first Sunday in November.
    std::tm startDST = {0, 0, 2, 8, 2, localTime.tm_year};  // March 8th 2:00 AM
    std::tm endDST = {0, 0,  2,
                      1, 10, localTime.tm_year};  // November 1st 2:00 AM

    // Find the second Sunday in March
    while (startDST.tm_wday != 0) {
        startDST.tm_mday += 1;
        mktime(&startDST);
    }
    startDST.tm_mday += 7;
    mktime(&startDST);

    // Find the first Sunday in November
    while (endDST.tm_wday != 0) {
        endDST.tm_mday += 1;
        mktime(&endDST);
    }

    std::time_t startDST_t = mktime(&startDST);
    std::time_t endDST_t = mktime(&endDST);

    return (currentTime >= startDST_t && currentTime < endDST_t);
}
}  // namespace atom::utils
