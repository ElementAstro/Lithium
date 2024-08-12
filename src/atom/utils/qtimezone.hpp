#ifndef ATOM_UTILS_QTIMEZONE_HPP
#define ATOM_UTILS_QTIMEZONE_HPP

#include <chrono>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

namespace atom::utils {
class MyDateTime;

class MyTimeZone {
public:
    MyTimeZone();
    MyTimeZone(const std::string& timeZoneId);

    static auto availableTimeZoneIds() -> std::vector<std::string>;
    [[nodiscard]] auto id() const -> std::string;
    [[nodiscard]] auto displayName() const -> std::string;

    [[nodiscard]] auto isValid() const -> bool;

    [[nodiscard]] auto offsetFromUtc(const MyDateTime& dateTime) const
        -> std::chrono::seconds;
    [[nodiscard]] auto standardTimeOffset() const -> std::chrono::seconds;
    [[nodiscard]] auto daylightTimeOffset() const -> std::chrono::seconds;
    [[nodiscard]] auto hasDaylightTime() const -> bool;
    [[nodiscard]] auto isDaylightTime(const MyDateTime& dateTime) const -> bool;

    auto operator<=>(const MyTimeZone& other) const = default;

private:
    std::string timeZoneId;
    std::optional<std::chrono::seconds> offset;
};
}  // namespace atom::utils

#endif
