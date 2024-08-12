#ifndef ATOM_UTILS_QDATETIME_HPP
#define ATOM_UTILS_QDATETIME_HPP

#include <chrono>
#include <ctime>
#include <optional>
#include <string>

namespace atom::utils {
class MyTimeZone;  // 前向声明

class MyDateTime {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    MyDateTime();
    MyDateTime(const std::string& dateTimeString, const std::string& format);
    MyDateTime(const std::string& dateTimeString, const std::string& format,
               const MyTimeZone& timeZone);

    static auto currentDateTime() -> MyDateTime;
    static auto currentDateTime(const MyTimeZone& timeZone) -> MyDateTime;
    static auto fromString(const std::string& dateTimeString,
                           const std::string& format) -> MyDateTime;
    static auto fromString(const std::string& dateTimeString,
                           const std::string& format,
                           const MyTimeZone& timeZone) -> MyDateTime;
    [[nodiscard]] auto toString(const std::string& format) const -> std::string;
    [[nodiscard]] auto toString(const std::string& format,
                                const MyTimeZone& timeZone) const
        -> std::string;
    [[nodiscard]] auto toTimeT() const -> std::time_t;

    [[nodiscard]] auto isValid() const -> bool;

    [[nodiscard]] auto addDays(int days) const -> MyDateTime;
    [[nodiscard]] auto addSecs(int seconds) const -> MyDateTime;

    [[nodiscard]] auto daysTo(const MyDateTime& other) const -> int;
    [[nodiscard]] auto secsTo(const MyDateTime& other) const -> int;

    auto operator<=>(const MyDateTime& other) const = default;

private:
    std::optional<TimePoint> dateTime;
};
}  // namespace atom::utils

#endif
