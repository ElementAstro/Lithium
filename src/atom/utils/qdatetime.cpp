#include "qdatetime.hpp"
#include "qtimezone.hpp"

#include <iomanip>
#include <sstream>

#include "atom/log/loguru.hpp"

namespace atom::utils {

QDateTime::QDateTime() : dateTime_(std::nullopt) {
    LOG_F(INFO, "QDateTime default constructor called");
}

QDateTime::QDateTime(const std::string& dateTimeString,
                     const std::string& format) {
    LOG_F(INFO,
          "QDateTime constructor called with dateTimeString: {}, format: {}",
          dateTimeString, format);
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        dateTime_ = Clock::from_time_t(std::mktime(&t));
        LOG_F(INFO, "QDateTime successfully parsed: {}", dateTimeString);
    } else {
        LOG_F(WARNING, "QDateTime failed to parse: {}", dateTimeString);
    }
}

QDateTime::QDateTime(const std::string& dateTimeString,
                     const std::string& format, const QTimeZone& timeZone) {
    LOG_F(INFO,
          "QDateTime constructor called with dateTimeString: {}, format: {}, "
          "timeZone offset: {}",
          dateTimeString, format, timeZone.offsetFromUtc(*this).count());
    std::istringstream ss(dateTimeString);
    std::tm t = {};
    ss >> std::get_time(&t, format.c_str());
    if (!ss.fail()) {
        auto time = std::mktime(&t) - timeZone.offsetFromUtc(*this).count();
        dateTime_ = Clock::from_time_t(time);
        LOG_F(INFO, "QDateTime successfully parsed with timezone: {}",
              dateTimeString);
    } else {
        LOG_F(WARNING, "QDateTime failed to parse with timezone: {}",
              dateTimeString);
    }
}

auto QDateTime::currentDateTime() -> QDateTime {
    LOG_F(INFO, "QDateTime::currentDateTime called");
    QDateTime dt;
    dt.dateTime_ = Clock::now();
    LOG_F(INFO, "QDateTime::currentDateTime returning current time");
    return dt;
}

auto QDateTime::currentDateTime(const QTimeZone& timeZone) -> QDateTime {
    LOG_F(INFO, "QDateTime::currentDateTime called with timeZone offset: {}",
          timeZone.offsetFromUtc(QDateTime()).count());
    QDateTime dt;
    dt.dateTime_ = Clock::now() + timeZone.offsetFromUtc(dt);
    LOG_F(INFO,
          "QDateTime::currentDateTime returning current time with timezone");
    return dt;
}

auto QDateTime::fromString(const std::string& dateTimeString,
                           const std::string& format) -> QDateTime {
    LOG_F(INFO,
          "QDateTime::fromString called with dateTimeString: {}, format: {}",
          dateTimeString, format);
    return QDateTime(dateTimeString, format);
}

auto QDateTime::fromString(const std::string& dateTimeString,
                           const std::string& format,
                           const QTimeZone& timeZone) -> QDateTime {
    LOG_F(INFO,
          "QDateTime::fromString called with dateTimeString: {}, format: {}, "
          "timeZone offset: {}",
          dateTimeString, format, timeZone.offsetFromUtc(QDateTime()).count());
    return QDateTime(dateTimeString, format, timeZone);
}

auto QDateTime::toString(const std::string& format) const -> std::string {
    LOG_F(INFO, "QDateTime::toString called with format: {}", format);
    if (!dateTime_) {
        LOG_F(WARNING, "QDateTime::toString called on invalid QDateTime");
        return "";
    }
    std::time_t tt = Clock::to_time_t(dateTime_.value());
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    LOG_F(INFO, "QDateTime::toString returning: {}", ss.str());
    return ss.str();
}

auto QDateTime::toString(const std::string& format,
                         const QTimeZone& timeZone) const -> std::string {
    LOG_F(INFO,
          "QDateTime::toString called with format: {}, timeZone offset: {}",
          format, timeZone.offsetFromUtc(*this).count());
    if (!dateTime_) {
        LOG_F(WARNING, "QDateTime::toString called on invalid QDateTime");
        return "";
    }
    auto adjustedTime = dateTime_.value() + timeZone.offsetFromUtc(*this);
    std::time_t tt = Clock::to_time_t(adjustedTime);
    std::tm tm = *std::localtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, format.c_str());
    LOG_F(INFO, "QDateTime::toString returning: {}", ss.str());
    return ss.str();
}

auto QDateTime::toTimeT() const -> std::time_t {
    LOG_F(INFO, "QDateTime::toTimeT called");
    if (!dateTime_) {
        LOG_F(WARNING, "QDateTime::toTimeT called on invalid QDateTime");
        return 0;
    }
    std::time_t result = Clock::to_time_t(dateTime_.value());
    LOG_F(INFO, "QDateTime::toTimeT returning: {}", result);
    return result;
}

auto QDateTime::isValid() const -> bool {
    LOG_F(INFO, "QDateTime::isValid called");
    bool result = dateTime_.has_value();
    LOG_F(INFO, "QDateTime::isValid returning: {}", result ? "true" : "false");
    return result;
}

auto QDateTime::addDays(int days) const -> QDateTime {
    LOG_F(INFO, "QDateTime::addDays called with days: {}", days);
    if (!dateTime_) {
        LOG_F(WARNING, "QDateTime::addDays called on invalid QDateTime");
        return {};
    }
    QDateTime dt;
    dt.dateTime_ = dateTime_.value() + std::chrono::hours(days * 24);
    LOG_F(INFO, "QDateTime::addDays returning new QDateTime");
    return dt;
}

auto QDateTime::addSecs(int seconds) const -> QDateTime {
    LOG_F(INFO, "QDateTime::addSecs called with seconds: {}", seconds);
    if (!dateTime_) {
        LOG_F(WARNING, "QDateTime::addSecs called on invalid QDateTime");
        return {};
    }
    QDateTime dt;
    dt.dateTime_ = dateTime_.value() + std::chrono::seconds(seconds);
    LOG_F(INFO, "QDateTime::addSecs returning new QDateTime");
    return dt;
}

auto QDateTime::daysTo(const QDateTime& other) const -> int {
    LOG_F(INFO, "QDateTime::daysTo called");
    if (!dateTime_ || !other.dateTime_) {
        LOG_F(WARNING, "QDateTime::daysTo called on invalid QDateTime");
        return 0;
    }
    int result = std::chrono::duration_cast<std::chrono::hours>(
                     other.dateTime_.value() - dateTime_.value())
                     .count() /
                 24;
    LOG_F(INFO, "QDateTime::daysTo returning: {}", result);
    return result;
}

auto QDateTime::secsTo(const QDateTime& other) const -> int {
    LOG_F(INFO, "QDateTime::secsTo called");
    if (!dateTime_ || !other.dateTime_) {
        LOG_F(WARNING, "QDateTime::secsTo called on invalid QDateTime");
        return 0;
    }
    int result = std::chrono::duration_cast<std::chrono::seconds>(
                     other.dateTime_.value() - dateTime_.value())
                     .count();
    LOG_F(INFO, "QDateTime::secsTo returning: {}", result);
    return result;
}

}  // namespace atom::utils
