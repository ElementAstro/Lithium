/*
 * kmtrace.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Trace Destroy Detector

*************************************************/

#include "kmtrace.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

#ifdef ATOM_OS_WIN
#include <Windows.h>
#elif defined(ATOM_OS_LINUX)
#include <sys/types.h>
#include <unistd.h>
#if !defined(ATOM_OS_ANDROID)
#include <sys/syscall.h>
#endif
#endif

#ifdef ATOM_OS_WIN
#define getCurrentThreadId() GetCurrentThreadId()
#define vsnprintf(d, dl, fmt, ...) _vsnprintf_s(d, dl, _TRUNCATE, fmt, ##__VA_ARGS__)
#elif defined(ATOM_OS_MAC)
#define getCurrentThreadId() pthread_mach_thread_np(pthread_self())
#elif defined(ATOM_OS_ANDROID)
#define getCurrentThreadId() gettid()
#elif defined(ATOM_OS_LINUX)
#define getCurrentThreadId() syscall(__NR_gettid)
#else
#define getCurrentThreadId() pthread_self()
#endif

#ifdef ATOM_OS_ANDROID
#include <android/log.h>
#endif

#define ATOM_TRACE_TAG "KEV"

namespace
{
    std::string getDateTimeString(bool utc)
    {
        auto now = std::chrono::system_clock::now();
        auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
        auto itt = std::chrono::system_clock::to_time_t(now);
        struct tm res;
#ifdef ATOM_OS_WIN
        utc ? gmtime_s(&res, &itt) : localtime_s(&res, &itt); // windows and C11
#else
        utc ? gmtime_r(&itt, &res) : localtime_r(&itt, &res);
#endif
        std::ostringstream ss;
        ss << std::put_time(&res, "%FT%T.") << std::setfill('0')
           << std::setw(3) << msecs;
        if (utc)
        {
            ss << 'Z';
        }
        else
        {
            ss << std::put_time(&res, "%z");
        }
        return ss.str();
    }
} // namespace

namespace Atom::Event
{

    static TraceFunc s_traceFunc = nullptr;
    static int s_traceLevel = TRACE_LEVEL_INFO;

    const char *kTraceStrings[] = {
        "NONE",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "VERBOS"};

#ifdef ATOM_OS_ANDROID
    const int kAndroidLogLevels[] = {
        ANDROID_LOG_INFO,
        ANDROID_LOG_ERROR,
        ANDROID_LOG_WARN,
        ANDROID_LOG_INFO,
        ANDROID_LOG_DEBUG,
        ANDROID_LOG_VERBOSE};
#endif

    void tracePrint(int level, const char *szMessage, ...)
    {
        va_list VAList;
        char szMsgBuf[2048] = {0};
        va_start(VAList, szMessage);
        vsnprintf(szMsgBuf, sizeof(szMsgBuf) - 1, szMessage, VAList);

        if (level > TRACE_LEVEL_MAX)
        {
            level = TRACE_LEVEL_MAX;
        }
        else if (level < TRACE_LEVEL_ERROR)
        {
            level = TRACE_LEVEL_ERROR;
        }

#if defined(ATOM_OS_ANDROID)
        int android_level = kAndroidLogLevels[level];
        __android_log_print(android_level, ATOM_TRACE_TAG, "%s", szMsgBuf);
#else
        std::stringstream ss;
        ss << kTraceStrings[level];
        ss << " [" << getCurrentThreadId() << "] " << szMsgBuf << '\n';
#if defined(ATOM_OS_WIN)
        OutputDebugStringA(ss.str().c_str());
#else
        printf("%s, %s", getDateTimeString(false).c_str(), ss.str().c_str());
#endif
#endif
    }

    void printTrace(int level, const std::string &msg)
    {
        if (level > TRACE_LEVEL_MAX)
        {
            level = TRACE_LEVEL_MAX;
        }
        else if (level < TRACE_LEVEL_ERROR)
        {
            level = TRACE_LEVEL_ERROR;
        }
#if defined(ATOM_OS_ANDROID)
        int android_level = kAndroidLogLevels[level];
        __android_log_print(android_level, ATOM_TRACE_TAG, "%s", msg.c_str());
#else
        std::stringstream ss;
        ss << kTraceStrings[level];
        ss << " [" << getCurrentThreadId() << "] " << msg << '\n';
#if defined(ATOM_OS_WIN)
        OutputDebugStringA(ss.str().c_str());
#else
        printf("%s %s", getDateTimeString(false).c_str(), ss.str().c_str());
#endif
#endif
    }

    void traceWrite(int level, const std::string &msg)
    {
        if (s_traceFunc)
        {
            s_traceFunc(level, std::string(msg));
        }
        else
        {
            printTrace(level, msg);
        }
    }

    void traceWrite(int level, std::string &&msg)
    {
        if (s_traceFunc)
        {
            s_traceFunc(level, std::move(msg));
        }
        else
        {
            printTrace(level, msg);
        }
    }

    void setTraceFunc(TraceFunc func)
    {
        s_traceFunc = std::move(func);
    }

    void setTraceLevel(int level)
    {
        s_traceLevel = level;
    }

    int getTraceLevel()
    {
        return s_traceLevel;
    }

} // namespace Atom::Event
