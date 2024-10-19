#ifndef ATOM_SYSINFO_WM_HPP
#define ATOM_SYSINFO_WM_HPP

#include <string>

#include "atom/macro.hpp"

namespace atom::system {
struct SystemInfo {
    std::string desktopEnvironment;  // DE: Fluent
    std::string windowManager;       // WM: Desktop Window Manager
    std::string wmTheme;  // WM Theme: Oem - Blue (System: Light, Apps: Light)
    std::string icons;    // Icons: Recycle Bin
    std::string font;     // Font: Microsoft YaHei UI (12pt)
    std::string cursor;   // Cursor: Windows Default (32px)
} ATOM_ALIGNAS(128);

auto getSystemInfo() -> SystemInfo;
}  // namespace atom::system

#endif
