/**
 * @file platform.hpp
 * @brief Platform and compiler detection macros.
 *
 * This file defines macros to detect and provide information about the
 * platform, architecture, operating system version, and compiler being
 * used. It includes platform-specific checks for Windows, macOS, Linux,
 * Android, and various BSD systems. It also provides macros to determine
 * if a GUI is available on the platform.
 *
 * @date 2024-02-10
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_SYSTEM_PLATFORM_HPP
#define ATOM_SYSTEM_PLATFORM_HPP

#if defined(_WIN32)
#if defined(__MINGW32__) || defined(__MINGW64__)
#define ATOM_PLATFORM "Windows MinGW"
#else
#define ATOM_PLATFORM "Windows MSVC"
#endif
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
#define ATOM_PLATFORM "iOS Simulator"
#elif TARGET_OS_IPHONE
#define ATOM_PLATFORM "iOS"
#elif TARGET_OS_MAC
#define ATOM_PLATFORM "macOS"
#else
#define ATOM_PLATFORM "Unknown Apple platform"
#endif
#elif defined(__ANDROID__)
#define ATOM_PLATFORM "Android"
#elif defined(__linux__)
#define ATOM_PLATFORM "Linux"
#elif defined(__FreeBSD__)
#define ATOM_PLATFORM "FreeBSD"
#elif defined(__OpenBSD__)
#define ATOM_PLATFORM "OpenBSD"
#elif defined(__NetBSD__)
#define ATOM_PLATFORM "NetBSD"
#elif defined(__DragonFly__)
#define ATOM_PLATFORM "DragonFly BSD"
#elif defined(__sun)
#define ATOM_PLATFORM "Solaris"
#elif defined(__hpux)
#define ATOM_PLATFORM "HP-UX"
#elif defined(__osf__)
#define ATOM_PLATFORM "OSF/1"
#elif defined(__arm__) || defined(__aarch64__)
#define ATOM_PLATFORM "ARM"
#elif defined(__xtensa__) || defined(__XTENSA__)
#define ATOM_PLATFORM "Xtensa"
#elif defined(SWIG)
#define ATOM_PLATFORM "SWIG"
#else
#define ATOM_PLATFORM "Unknown platform"
#endif

#if defined(__i386__) || defined(__i386)
#define ATOM_ARCHITECTURE "x86"
#elif defined(__x86_64__)
#define ATOM_ARCHITECTURE "x86_64"
#elif defined(__arm__)
#define ATOM_ARCHITECTURE "ARM"
#elif defined(__aarch64__)
#define ATOM_ARCHITECTURE "ARM64"
#else
#define ATOM_ARCHITECTURE "Unknown architecture"
#endif

#ifdef _WIN32
#if _WIN32_WINNT >= 0x0A00  // Windows 10 or newer
#define ATOM_OS_VERSION "Windows 10 or newer"
#elif _WIN32_WINNT >= 0x0603  // Windows 8.1
#define ATOM_OS_VERSION "Windows 8.1"
#elif _WIN32_WINNT >= 0x0602  // Windows 8
#define ATOM_OS_VERSION "Windows 8"
#elif _WIN32_WINNT >= 0x0601  // Windows 7
#define ATOM_OS_VERSION "Windows 7"
#elif _WIN32_WINNT >= 0x0600  // Windows Vista
#define ATOM_OS_VERSION "Windows Vista"
#elif _WIN32_WINNT >= 0x0501  // Windows XP
#define ATOM_OS_VERSION "Windows XP"
#else
#define ATOM_OS_VERSION "Unknown Windows version"
#endif
#elif defined(__APPLE__)
#define ATOM_OS_VERSION "macOS"
#elif defined(__ANDROID__)
#include <android/api-level.h>
#if __ANDROID_API__ >= 21
#define ATOM_OS_VERSION "Android " __ANDROID_API__
#else
#define ATOM_OS_VERSION "Android"
#endif
#elif defined(__linux__)
#define ATOM_OS_VERSION "Linux"
#else
#define ATOM_OS_VERSION "Unknown OS version"
#endif

#if defined(__clang__)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ATOM_COMPILER                                \
    "Clang " TOSTRING(__clang_major__) "." TOSTRING( \
        __clang_minor__) "." TOSTRING(__clang_patchlevel__)
#elif defined(__GNUC__)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ATOM_COMPILER                                                    \
    "GCC " TOSTRING(__GNUC__) "." TOSTRING(__GNUC_MINOR__) "." TOSTRING( \
        __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ATOM_COMPILER "MSVC " TOSTRING(_MSC_FULL_VER)
#else
#define ATOM_COMPILER "Unknown compiler"
#endif

#if defined(_WIN32)
#include <windows.h>
#define ATOM_HAS_GUI() (GetSystemMetrics(SM_CXSCREEN) > 0)

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC == 1 || TARGET_OS_IPHONE == 1
#define ATOM_HAS_GUI() (true)
#else
#define ATOM_HAS_GUI() (false)
#endif

#elif defined(__ANDROID__)
#include <android/api-level.h>
#define ATOM_HAS_GUI() (__ANDROID_API__ >= 24)

#elif defined(__linux__)
#if defined(HAVE_X11)
#include <X11/Xlib.h>
#define HAS_X11_GUI()                          \
    ({                                         \
        Display *display = XOpenDisplay(NULL); \
        bool has_gui = (display != NULL);      \
        if (display != NULL)                   \
            XCloseDisplay(display);            \
        has_gui;                               \
    })
#else
#define HAS_X11_GUI() (false)
#endif

#if defined(HAVE_WAYLAND)
#include <wayland-client.h>
#define HAS_WAYLAND_GUI()                                      \
    ({                                                         \
        struct wl_display *display = wl_display_connect(NULL); \
        bool has_gui = (display != NULL);                      \
        if (display != NULL)                                   \
            wl_display_disconnect(display);                    \
        has_gui;                                               \
    })
#else
#define HAS_WAYLAND_GUI() (false)
#endif

#define ATOM_HAS_GUI() (HAS_X11_GUI() || HAS_WAYLAND_GUI())

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#if defined(HAVE_X11)
#include <X11/Xlib.h>
#define ATOM_HAS_GUI()                         \
    ({                                         \
        Display *display = XOpenDisplay(NULL); \
        bool has_gui = (display != NULL);      \
        if (display != NULL)                   \
            XCloseDisplay(display);            \
        has_gui;                               \
    })
#else
#define ATOM_HAS_GUI() (false)
#endif

#else
#define ATOM_HAS_GUI() (false)
#endif

#ifdef __ORDER_LITTLE_ENDIAN__
#define ATOM_EL __ORDER_LITTLE_ENDIAN__
#else
#define ATOM_EL 1234
#endif

#ifdef __ORDER_BIG_ENDIAN__
#define ATOM_EB __ORDER_BIG_ENDIAN__
#else
#define ATOM_EB 4321
#endif

// mixed byte order (eg, PowerPC or ia64)
#define ATOM_EM 1111

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || \
    defined(_M_X64)
#define ATOM_CPU_X86_64
#define ATOM_WORDSIZE 8
#define ATOM_BYTE_ORDER _ATOM_EL

#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define ATOM_CPU_X86
#define ATOM_WORDSIZE 4
#define ATOM_BYTE_ORDER _ATOM_EL

#elif defined(__arm__) || defined(_M_ARM) || defined(__TARGET_ARCH_ARM) || \
    defined(__aarch64__) || defined(_M_ARM64)
#if defined(__aarch64__) || defined(_M_ARM64)
#define ATOM_CPU_ARM64
#define ATOM_CPU_ARMV8
#define ATOM_WORDSIZE 8
#else
#define ATOM_CPU_ARM
#define ATOM_WORDSIZE 4
#if defined(__ARM_ARCH_8__) || defined(__ARM_ARCH_8A__) || \
    (defined(__ARCH_ARM) && __ARCH_ARM >= 8) ||            \
    (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM >= 8)
#define ATOM_CPU_ARMV8
#elif defined(__ARM_ARCH_7__) || defined(_ARM_ARCH_7) ||      \
    defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) ||   \
    defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) ||   \
    defined(__ARM_ARCH_7EM__) ||                              \
    (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM >= 7) || \
    (defined(_M_ARM) && _M_ARM >= 7)
#define ATOM_CPU_ARMV7
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || \
    defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6ZK__) || \
    defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_6KZ__) || \
    (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM >= 6)
#define ATOM_CPU_ARMV6
#elif defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_5TE__) || \
    (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM >= 5)
#define ATOM_CPU_ARMV5
#elif defined(__ARM_ARCH_4T__) || \
    (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM >= 4)
#define ATOM_CPU_ARMV4
#else
#error "unknown CPU architecture: ARM"
#endif
#endif
#if defined(__ARMEL__) || defined(__LITTLE_ENDIAN__) || \
    defined(__AARCH64EL__) ||                           \
    (defined(__BYTE_ORDER__) &&                         \
     (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) ||    \
    defined(                                            \
        _MSC_VER)  // winarm64 does not provide any of the above macros,
                   // but advises little-endianess:
                   // https://docs.microsoft.com/en-us/cpp/build/overview-of-arm-abi-conventions?view=msvc-170
                   // So if it is visual studio compiling, we'll assume little
                   // endian.
#define ATOM_BYTE_ORDER _ATOM_EL
#elif defined(__ARMEB__) || defined(__BIG_ENDIAN__) || \
    defined(__AARCH64EB__) ||                          \
    (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
#define ATOM_BYTE_ORDER _ATOM_EB
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_PDP_ENDIAN__)
#define ATOM_BYTE_ORDER _ATOM_EM
#else
#error "unknown endianness"
#endif

#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
#define ATOM_CPU_IA64
#define ATOM_WORDSIZE 8
#define ATOM_BYTE_ORDER _ATOM_EM
// itanium is bi-endian - check byte order below

#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__) ||   \
    defined(_ARCH_COM) || defined(_ARCH_PWR) || defined(_ARCH_PPC) || \
    defined(_M_MPPC) || defined(_M_PPC)
#if defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
#define ATOM_CPU_PPC64
#define ATOM_WORDSIZE 8
#else
#define ATOM_CPU_PPC
#define ATOM_WORDSIZE 4
#endif
#define ATOM_BYTE_ORDER _ATOM_EM
// ppc is bi-endian - check byte order below

#elif defined(__s390x__) || defined(__zarch__) || defined(__SYSC_ZARCH_)
#define ATOM_CPU_S390_X
#define ATOM_WORDSIZE 8
#define ATOM_BYTE_ORDER _ATOM_EB

#elif defined(__xtensa__) || defined(__XTENSA__)
#define ATOM_CPU_XTENSA
#define ATOM_WORDSIZE 4
// not sure about this...
#if defined(__XTENSA_EL__) || defined(__xtensa_el__)
#define ATOM_BYTE_ORDER _ATOM_EL
#else
#define ATOM_BYTE_ORDER _ATOM_EB
#endif

#elif defined(__riscv)
#if __riscv_xlen == 64
#define ATOM_CPU_RISCV64
#define ATOM_WORDSIZE 8
#else
#define ATOM_CPU_RISCV32
#define ATOM_WORDSIZE 4
#endif
#define ATOM_BYTE_ORDER _ATOM_EL

#elif defined(__EMSCRIPTEN__)
#define ATOM_BYTE_ORDER _ATOM_EL
#define ATOM_WORDSIZE 4

#elif defined(__loongarch__)
#if defined(__loongarch64)
#define ATOM_CPU_LOONGARCH64
#define ATOM_WORDSIZE 8
#else
#define ATOM_CPU_LOONGARCH
#define ATOM_WORDSIZE 4
#endif
#define ATOM_BYTE_ORDER _ATOM_EL

#elif defined(SWIG)
#error "please define CPU architecture macros when compiling with swig"

#else
#error "unknown CPU architecture"
#endif

#define ATOM_LITTLE_ENDIAN (ATOM_BYTE_ORDER == ATOM_EL)
#define ATOM_BIG_ENDIAN (ATOM_BYTE_ORDER == ATOM_EB)
#define ATOM_MIXED_ENDIAN (ATOM_BYTE_ORDER == ATOM_EM)

#endif
