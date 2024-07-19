#include "qdatetime.hpp"
#include "qtimezone.hpp"

#include <iomanip>
#include <sstream>

namespace atom::utils {
MyDateTime::MyDateTime() : dateTime(std::nullopt) {}

MyDateTime::MyDateTime(const std::string& dateTimeString,
                       const std::string& format) {
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        dateTime = Clock::from_time_t(std::mktime(&t));
    }
}

MyDateTime::MyDateTime(const std::string& dateTimeString,
                       const std::string& format, const MyTimeZone& timeZone) {
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        auto time = std::mktime(&t) - timeZone.offsetFromUtc(*this).count();
        dateTime = Clock::from_time_t(time);
    }
}

auto MyDateTime::currentDateTime() -> MyDateTime {
    MyDateTime dt;
    dt.dateTime = Clock::now();
    return dt;
}

auto MyDateTime::currentDateTime(const MyTimeZone& timeZone) -> MyDateTime {
    MyDateTime dt;
    dt.dateTime = Clock::now() + timeZone.offsetFromUtc(dt);
    return dt;
}

auto MyDateTime::fromString(const std::string& dateTimeString,
                            const std::string& format) -> MyDateTime {
    return MyDateTime(dateTimeString, format);
}

auto MyDateTime::fromString(const std::string& dateTimeString,
                            const std::string& format,
                            const MyTimeZone& timeZone) -> MyDateTime {
    return MyDateTime(dateTimeString, format, timeZone);
}

auto MyDateTime::toString(const std::string& format) const -> std::string {
    if (!dateTime) {
        return "";
    }
    std::time_t tt = Clock::to_time_t(dateTime.value());
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

auto MyDateTime::toString(const std::string& format,
                          const MyTimeZone& timeZone) const -> std::string {
    if (!dateTime) {
        return "";
    }
    auto adjustedTime = dateTime.value() + timeZone.offsetFromUtc(*this);
    std::time_t tt = Clock::to_time_t(adjustedTime);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

auto MyDateTime::toTimeT() const -> std::time_t {
    if (!dateTime) {
        return 0;
    }
    return Clock::to_time_t(dateTime.value());
}

auto MyDateTime::isValid() const -> bool { return dateTime.has_value(); }

auto MyDateTime::addDays(int days) const -> MyDateTime {
    if (!dateTime) {
        return {};
    }
    MyDateTime dt;
    dt.dateTime = dateTime.value() + std::chrono::hours(days * 24);
    return dt;
}

auto MyDateTime::addSecs(int seconds) const -> MyDateTime {
    if (!dateTime) {
        return {};
    }
    MyDateTime dt;
    dt.dateTime = dateTime.value() + std::chrono::seconds(seconds);
    return dt;
}

auto MyDateTime::daysTo(const MyDateTime& other) const -> int {
    if (!dateTime || !other.dateTime) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::hours>(
               other.dateTime.value() - dateTime.value())
               .count() /
           24;
}

auto MyDateTime::secsTo(const MyDateTime& other) const -> int {
    if (!dateTime || !other.dateTime) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::seconds>(
               other.dateTime.value() - dateTime.value())
        .count();
}

}  // namespace atom::utils
