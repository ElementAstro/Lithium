/*
 * stacktrace.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: StackTrace

**************************************************/

#include "stacktrace.hpp"
#include "atom/function/abi.hpp"

#include <ctime>
#include <sstream>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbgHelp.h>
// clang-format on
#if !defined(__MINGW32__) && !defined(__MINGW64__)
#pragma comment(lib, "dbghelp.lib")
#endif
#elif defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#endif

namespace atom::error {
StackTrace::StackTrace() { capture(); }

std::string StackTrace::toString() const {
    std::ostringstream oss;
#ifdef _WIN32
    SYMBOL_INFO* symbol =
        (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (void* frame : frames_) {
        SymFromAddr(GetCurrentProcess(), (DWORD64)frame, 0, symbol);
        std::string symbol_name = symbol->Name;
        if (!symbol_name.empty()) {
            oss << "\t\t"
                << atom::meta::DemangleHelper::Demangle("_" + symbol_name)
                << " - 0x" << std::hex << symbol->Address << "\n";
        }
    }
    free(symbol);
#elif defined(__APPLE__) || defined(__linux__)
    for (int i = 0; i < num_frames_; ++i) {
        char* symbolName = nullptr;
        char* offsetBegin = nullptr;
        char* offsetEnd = nullptr;

        for (char* p = symbols_.get()[i]; *p != 0; ++p) {
            if (*p == '(') {
                symbolName = p;
            } else if (*p == '+') {
                offsetBegin = p;
            } else if (*p == ')') {
                offsetEnd = p;
                break;
            }
        }

        if ((symbolName != nullptr) && (offsetBegin != nullptr) &&
            (offsetEnd != nullptr) && symbolName < offsetBegin) {
            *symbolName++ = '\0';
            *offsetBegin++ = '\0';
            *offsetEnd = '\0';
            auto demangledName =
                atom::meta::DemangleHelper::demangle(symbolName);

            oss << "\t\t" << demangledName << " +" << offsetBegin << offsetEnd
                << "\n";
        } else {
            oss << "\t\t" << symbols_.get()[i] << "\n";
        }
    }
#else
    oss << "\t\tStack trace not available on this platform.\n";
#endif
    return oss.str();
}

void StackTrace::capture() {
#ifdef _WIN32
    constexpr int max_frames = 64;
    frames.resize(max_frames);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    std::array<void*, max_frames> frame_ptrs;
    WORD captured_frames =
        CaptureStackBackTrace(0, max_frames, frame_ptrs.data(), nullptr);

    frames.resize(captured_frames);
    std::copy_n(frame_ptrs.begin(), captured_frames, frames.begin());

#elif defined(__APPLE__) || defined(__linux__)
    constexpr int MAX_FRAMES = 64;
    void* framePtrs[MAX_FRAMES];

    num_frames_ = backtrace(framePtrs, MAX_FRAMES);

#ifdef USE_GSL
    gsl::span<void*> frameSpan(framePtrs, num_frames_);
    symbols_.reset(backtrace_symbols(frameSpan, num_frames_));
#else
    symbols_.reset(backtrace_symbols(framePtrs, num_frames_));
#endif

#else
    num_frames_ = 0;
#endif
}
}  // namespace atom::error
