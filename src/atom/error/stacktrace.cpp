/*
 * stacktrace.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Enhanced StackTrace with more details

**************************************************/

#include "stacktrace.hpp"
#include "atom/function/abi.hpp"

#include <regex>
#include <sstream>
#include <string_view>
#include <vector>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#if !defined(__MINGW32__) && !defined(__MINGW64__)
#pragma comment(lib, "dbghelp.lib")
#endif
#elif defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif

namespace atom::error {

namespace {
#if defined(__linux__) || defined(__APPLE__)
auto processString(const std::string& input) -> std::string {
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

auto prettifyStacktrace(const std::string& input) -> std::string {
    std::string output = input;
    static const std::vector<std::pair<std::string, std::string>> REPLACEMENTS =
        {{"std::__1::", "std::"},
         {"__thiscall ", ""},
         {"__cdecl ", ""},
         {", std::allocator<[^<>]+>", ""}};

    for (const auto& [from, to] : REPLACEMENTS) {
        output = std::regex_replace(output, std::regex(from), to);
    }

    // Clean up spaces in template arguments
    output =
        std::regex_replace(output, std::regex(R"(<\s*([^<> ]+)\s*>)"), "<$1>");

    return output;
}

}  // unnamed namespace

StackTrace::StackTrace() { capture(); }

auto StackTrace::toString() const -> std::string {
    std::ostringstream oss;

#ifdef _WIN32
    auto* symbol = reinterpret_cast<SYMBOL_INFO*>(
        calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (void* frame : frames_) {
        DWORD64 displacement = 0;
        if (SymFromAddr(GetCurrentProcess(), reinterpret_cast<DWORD64>(frame),
                        &displacement, symbol) != 0) {
            std::string symbolName = symbol->Name;
            oss << "\t\t" << meta::DemangleHelper::demangle("_" + symbolName)
                << " - 0x" << std::hex << symbol->Address << "\n";
        }
    }
    free(symbol);

#elif defined(__APPLE__) || defined(__linux__)
    for (int i = 0; i < num_frames_; ++i) {
        Dl_info info;
        if (dladdr(frames_[i], &info) && info.dli_sname) {
            std::string symbol_name =
                meta::DemangleHelper::demangle(info.dli_sname);
            oss << "\t\t" << symbol_name << " (" << info.dli_fname << ")\n";
        } else {
            std::string_view symbol(symbols_.get()[i]);
            oss << "\t\t" << processString(std::string(symbol)) << "\n";
        }
    }

#else
    oss << "\t\tStack trace not available on this platform.\n";
#endif

    return prettifyStacktrace(oss.str());
}

void StackTrace::capture() {
#ifdef _WIN32
    constexpr int max_frames = 64;
    frames_.resize(max_frames);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    void* framePtrs[max_frames];
    WORD capturedFrames =
        CaptureStackBackTrace(0, max_frames, framePtrs, nullptr);

    frames_.resize(capturedFrames);
    std::copy_n(framePtrs, capturedFrames, frames_.begin());

#elif defined(__APPLE__) || defined(__linux__)
    constexpr int MAX_FRAMES = 64;
    void* framePtrs[MAX_FRAMES];

    num_frames_ = backtrace(framePtrs, MAX_FRAMES);
    symbols_.reset(backtrace_symbols(framePtrs, num_frames_));
    frames_.assign(framePtrs, framePtrs + num_frames_);

#else
    num_frames_ = 0;
#endif
}

}  // namespace atom::error
