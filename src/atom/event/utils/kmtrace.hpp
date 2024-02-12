/*
 * kmtrace.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Trace Destroy Detector

*************************************************/

#pragma once

#include "../kmconf.hpp"

#include <sstream>
#include <functional>
#include <assert.h>

namespace Atom::Event
{

#define KM_TRACE(l, x)                                \
    do                                                \
    {                                                 \
        if (l <= kev::getTraceLevel())                \
        {                                             \
            std::ostringstream __kev_oss_42__;        \
            __kev_oss_42__ << x;                      \
            kev::traceWrite(l, __kev_oss_42__.str()); \
        }                                             \
    } while (0)

#define KM_XTRACE(l, x)                                  \
    do                                                   \
    {                                                    \
        if (l <= kev::getTraceLevel())                   \
        {                                                \
            std::ostringstream __kev_oss_42__;           \
            __kev_oss_42__ << getObjKey() << ":: " << x; \
            kev::traceWrite(l, __kev_oss_42__.str());    \
        }                                                \
    } while (0)

#define KM_INFOXTRACE(x) KM_XTRACE(kev::TRACE_LEVEL_INFO, x)
#define KM_WARNXTRACE(x) KM_XTRACE(kev::TRACE_LEVEL_WARN, x)
#define KM_ERRXTRACE(x) KM_XTRACE(kev::TRACE_LEVEL_ERROR, x)
#define KM_DBGXTRACE(x) KM_XTRACE(kev::TRACE_LEVEL_DEBUG, x)

#define KM_INFOTRACE(x) KM_TRACE(kev::TRACE_LEVEL_INFO, x)
#define KM_WARNTRACE(x) KM_TRACE(kev::TRACE_LEVEL_WARN, x)
#define KM_ERRTRACE(x) KM_TRACE(kev::TRACE_LEVEL_ERROR, x)
#define KM_DBGTRACE(x) KM_TRACE(kev::TRACE_LEVEL_DEBUG, x)

#define KM_INFOTRACE_THIS(x) KM_TRACE(kev::TRACE_LEVEL_INFO, x << ", this=" << this)
#define KM_WARNTRACE_THIS(x) KM_TRACE(kev::TRACE_LEVEL_WARN, x << ", this=" << this)
#define KM_ERRTRACE_THIS(x) KM_TRACE(kev::TRACE_LEVEL_ERROR, x << ", this=" << this)
#define KM_DBGTRACE_THIS(x) KM_TRACE(kev::TRACE_LEVEL_DEBUG, x << ", this=" << this)

#define KM_ASSERT(x) assert(x)

    const int TRACE_LEVEL_ERROR = 1;
    const int TRACE_LEVEL_WARN = 2;
    const int TRACE_LEVEL_INFO = 3;
    const int TRACE_LEVEL_DEBUG = 4;
    const int TRACE_LEVEL_VERBOS = 5;
    const int TRACE_LEVEL_MAX = TRACE_LEVEL_VERBOS;

    void traceWrite(int level, const std::string &msg);
    void traceWrite(int level, std::string &&msg);

    using TraceFunc = std::function<void(int level, std::string &&msg)>;
    void setTraceFunc(TraceFunc func);
    void setTraceLevel(int level);
    int getTraceLevel();

} // namespace Atom::Event
