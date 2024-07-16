/*
 * platform.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A platform information collection.

**************************************************/

#include "platform.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#if __has_include(<X11/Xlib.h>)
#include <X11/Xlib.h>
#endif
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <TargetConditionals.h>
#elif defined(__ANDROID__)
#include <android/api-level.h>
#endif

namespace atom::system {
#ifdef _WIN32
auto getWindowsVersion() -> std::string {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *)&osvi);

    if (osvi.dwMajorVersion == 11) [[likely]] {
        return "Windows 11";
    } else if (osvi.dwMajorVersion == 10) [[likely]] {
        return "Windows 10";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
        [[unlikely]] {
        return "Windows 8.1";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
        [[unlikely]] {
        return "Windows 8";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
        [[likely]] {
        return "Windows 7";
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
        [[unlikely]] {
        return "Windows Vista";
    } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
        [[unlikely]] {
        return "Windows XP";
    } else {
        return "Unknown Windows version";
    }
}
#endif

auto hasGUI() -> bool {
#if defined(_WIN32)
    return GetSystemMetrics(SM_CXSCREEN) > 0;
#elif defined(__APPLE__)

#if TARGET_OS_MAC == 1
    return true;
#elif TARGET_OS_IPHONE == 1
    return true;
#else
    return false;
#endif
#elif defined(__ANDROID__)

    return __ANDROID_API__ >= 24;
#elif defined(__linux__)
#if defined(HAVE_X11)
    Display *display = XOpenDisplay(NULL);
    if (display != NULL) {
        XCloseDisplay(display);
        return true;
    }
#endif
#if defined(HAVE_WAYLAND)
    struct wl_display *display = wl_display_connect(NULL);
    if (display != NULL) {
        wl_display_disconnect(display);
        return true;
    }
#endif
    return false;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#if defined(HAVE_X11)
    Display *display = XOpenDisplay(NULL);
    if (display != NULL) {
        XCloseDisplay(display);
        return true;
    }
#endif
    return false;
#else
    return false;
#endif
}
}  // namespace atom::system
