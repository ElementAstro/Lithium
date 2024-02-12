/*
 * kevconf.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Configuration

**************************************************/

#ifndef ATOM_EVENT_CONF_HPP
#define ATOM_EVENT_CONF_HPP

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__CYGWIN__)
#define ATOM_OS_WIN
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define ATOM_OS_LINUX
#if defined(__ANDROID__)
#define ATOM_OS_ANDROID
#endif
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define ATOM_OS_MAC
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE == 1
#define ATOM_OS_IOS
#endif
#else
#error "Unknown OS, I don't know what to do"
#endif

#ifdef ATOM_OS_WIN
#if defined(_WIN64)
#define ATOM_ENV64
#else
#define ATOM_ENV32
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#if defined(__x86_64__) || defined(__ppc64__)
#define ATOM_ENV64
#else
#define ATOM_ENV32
#endif
#endif

#endif
