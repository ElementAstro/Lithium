#ifndef LITHIUM_ADDON_VERSION_HPP
#define LITHIUM_ADDON_VERSION_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "macro.hpp"

namespace lithium {

struct Version {
    int major;
    int minor;
    int patch;
    std::string prerelease;  // alpha, beta, rc etc.
    std::string build;       // build metadata

    constexpr Version() : major(0), minor(0), patch(0) {}

    constexpr Version(int maj, int min, int pat, std::string pre = "",
                      std::string bld = "")
        : major(maj),
          minor(min),
          patch(pat),
          prerelease(std::move(pre)),
          build(std::move(bld)) {}

    static constexpr auto parse(std::string_view versionStr) -> Version;
    [[nodiscard]] auto toString() const -> std::string;

    constexpr auto operator<(const Version& other) const -> bool;
    constexpr auto operator>(const Version& other) const -> bool;
    constexpr auto operator==(const Version& other) const -> bool;
    constexpr auto operator<=(const Version& other) const -> bool;
    constexpr auto operator>=(const Version& other) const -> bool;
} ATOM_ALIGNAS(128);

auto operator<<(std::ostream& os, const Version& version) -> std::ostream&;

struct DateVersion {
    int year;
    int month;
    int day;

    constexpr DateVersion(int y, int m, int d) : year(y), month(m), day(d) {}

    static constexpr auto parse(std::string_view dateStr) -> DateVersion;

    constexpr auto operator<(const DateVersion& other) const -> bool;
    constexpr auto operator>(const DateVersion& other) const -> bool;
    constexpr auto operator==(const DateVersion& other) const -> bool;
    constexpr auto operator<=(const DateVersion& other) const -> bool;
    constexpr auto operator>=(const DateVersion& other) const -> bool;
} ATOM_ALIGNAS(16);

auto operator<<(std::ostream& os, const DateVersion& version) -> std::ostream&;

auto checkVersion(const Version& actualVersion,
                  const std::string& requiredVersionStr) -> bool;
auto checkDateVersion(const DateVersion& actualVersion,
                      const std::string& requiredVersionStr) -> bool;

}  // namespace lithium

#endif
