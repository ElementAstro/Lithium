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

#include <sstream>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <dbghelp.h>
#include <windows.h>
#if !defined(__MINGW32__) && !defined(__MINGW64__)
#pragma comment(lib, "dbghelp.lib")
#endif
#elif defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#endif

namespace atom::error {

#if defined(linux) || defined(__APPLE__)
std::string processString(const std::string& input) {
    // Find the position of "_Z" in the string
    size_t startIndex = input.find("_Z");
    if (startIndex == std::string::npos) {
        return input;
    }
    size_t endIndex = input.find('+', startIndex);
    if (endIndex == std::string::npos) {
        return input;
    }
    std::string abiName = input.substr(startIndex, endIndex - startIndex);
    abiName = meta::DemangleHelper::demangle(abiName);
    std::string result = input;
    result.replace(startIndex, endIndex - startIndex, abiName);
    return result;
}
#endif

StackTrace::StackTrace() { capture(); }

auto StackTrace::toString() const -> std::string {
    std::ostringstream oss;

#ifdef _WIN32
    SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(
        calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (void* frame : frames_) {
        SymFromAddr(GetCurrentProcess(), reinterpret_cast<DWORD64>(frame), 0,
                    symbol);
        std::string symbol_name = symbol->Name;
        if (!symbol_name.empty()) {
            oss << "\t\t"
                << atom::meta::DemangleHelper::demangle("_" + symbol_name)
                << " - 0x" << std::hex << symbol->Address << "\n";
        }
    }
    free(symbol);

#elif defined(__APPLE__) || defined(__linux__)
    for (int i = 0; i < num_frames_; ++i) {
        std::string_view symbol(symbols_.get()[i]);
        auto demangledName = processString(std::string(symbol));
        oss << "\t\t" << demangledName << "\n";
    }

#else
    oss << "\t\tStack trace not available on this platform.\n";
#endif

    return oss.str();
}

void StackTrace::capture() {
#ifdef _WIN32
    constexpr int max_frames = 64;
    frames_.resize(max_frames);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    std::array<void*, max_frames> frame_ptrs;
    WORD captured_frames =
        CaptureStackBackTrace(0, max_frames, frame_ptrs.data(), nullptr);

    frames_.resize(captured_frames);
    std::copy_n(frame_ptrs.begin(), captured_frames, frames_.begin());

#elif defined(__APPLE__) || defined(__linux__)
    constexpr int MAX_FRAMES = 64;
    void* framePtrs[MAX_FRAMES];

    num_frames_ = backtrace(framePtrs, MAX_FRAMES);
    symbols_.reset(backtrace_symbols(framePtrs, num_frames_));

#else
    num_frames_ = 0;
#endif
}

}  // namespace atom::error
