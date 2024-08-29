#include "qdatetime.hpp"
#include "qtimezone.hpp"

#include <iomanip>
#include <sstream>

namespace atom::utils {
QDateTime::QDateTime() : dateTime_(std::nullopt) {}

QDateTime::QDateTime(const std::string& dateTimeString,
                     const std::string& format) {
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        dateTime_ = Clock::from_time_t(std::mktime(&t));
    }
}

QDateTime::QDateTime(const std::string& dateTimeString,
                     const std::string& format, const QTimeZone& timeZone) {
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        auto time = std::mktime(&t) - timeZone.offsetFromUtc(*this).count();
        dateTime_ = Clock::from_time_t(time);
    }
}

auto QDateTime::currentDateTime() -> QDateTime {
    QDateTime dt;
    dt.dateTime_ = Clock::now();
    return dt;
}

auto QDateTime::currentDateTime(const QTimeZone& timeZone) -> QDateTime {
    QDateTime dt;
    dt.dateTime_ = Clock::now() + timeZone.offsetFromUtc(dt);
    return dt;
}

auto QDateTime::fromString(const std::string& dateTimeString,
                           const std::string& format) -> QDateTime {
    return QDateTime(dateTimeString, format);
}

auto QDateTime::fromString(const std::string& dateTimeString,
                           const std::string& format,
                           const QTimeZone& timeZone) -> QDateTime {
    return QDateTime(dateTimeString, format, timeZone);
}

auto QDateTime::toString(const std::string& format) const -> std::string {
    if (!dateTime_) {
        return "";
    }
    std::time_t tt = Clock::to_time_t(dateTime_.value());
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

auto QDateTime::toString(const std::string& format,
                         const QTimeZone& timeZone) const -> std::string {
    if (!dateTime_) {
        return "";
    }
    auto adjustedTime = dateTime_.value() + timeZone.offsetFromUtc(*this);
    std::time_t tt = Clock::to_time_t(adjustedTime);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    return ss.str();
}

auto QDateTime::toTimeT() const -> std::time_t {
    if (!dateTime_) {
        return 0;
    }
    return Clock::to_time_t(dateTime_.value());
}

auto QDateTime::isValid() const -> bool { return dateTime_.has_value(); }

auto QDateTime::addDays(int days) const -> QDateTime {
    if (!dateTime_) {
        return {};
    }
    QDateTime dt;
    dt.dateTime_ = dateTime_.value() + std::chrono::hours(days * 24);
    return dt;
}

auto QDateTime::addSecs(int seconds) const -> QDateTime {
    if (!dateTime_) {
        return {};
    }
    QDateTime dt;
    dt.dateTime_ = dateTime_.value() + std::chrono::seconds(seconds);
    return dt;
}

auto QDateTime::daysTo(const QDateTime& other) const -> int {
    if (!dateTime_ || !other.dateTime_) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::hours>(
               other.dateTime_.value() - dateTime_.value())
               .count() /
           24;
}

auto QDateTime::secsTo(const QDateTime& other) const -> int {
    if (!dateTime_ || !other.dateTime_) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::seconds>(
               other.dateTime_.value() - dateTime_.value())
        .count();
}

}  // namespace atom::utils
