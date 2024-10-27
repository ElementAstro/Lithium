#ifndef ATOM_SYSINFO_BIOS_HPP
#define ATOM_SYSINFO_BIOS_HPP

#include <string>

#include "atom/atom/macro.hpp"

namespace atom::system {
struct BiosInfoData {
    std::string version;
    std::string manufacturer;
    std::string releaseDate;
} ATOM_ALIGNAS(128);

auto getBiosInfo() -> BiosInfoData;
}  // namespace atom::system

#endif  // ATOM_SYSINFO_BIOS_HPP
