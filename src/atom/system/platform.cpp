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
std::string getWindowsVersion() {
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

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#endif

bool hasGUI() {
#if defined(_WIN32)
    return GetSystemMetrics(SM_CXSCREEN) > 0;
#elif defined(__linux__)
    Display *display = XOpenDisplay(NULL);
    if (display != NULL) {
        XCloseDisplay(display);
        return true;
    }
    return false;
#elif defined(__APPLE__)
    return true;  // macOS 系统默认支持 GUI
#else
    return false;
#endif
}
