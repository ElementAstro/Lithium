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

#include <chrono>
#include <ctime>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#include <DbgHelp.h>
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

    for (void* frame : frames) {
        SymFromAddr(GetCurrentProcess(), (DWORD64)frame, 0, symbol);
        oss << "\t\t" << symbol->Name << " - 0x" << std::hex << symbol->Address
            << "\n";
    }
    free(symbol);
#elif defined(__APPLE__) || defined(__linux__)
    for (int i = 0; i < num_frames; ++i) {
        char* symbol_name = nullptr;
        char* offset_begin = nullptr;
        char* offset_end = nullptr;

        for (char* p = symbols.get()[i]; *p; ++p) {
            if (*p == '(')
                symbol_name = p;
            else if (*p == '+')
                offset_begin = p;
            else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        if (symbol_name && offset_begin && offset_end &&
            symbol_name < offset_begin) {
            *symbol_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end = '\0';

            int status = 0;
            char* demangled_name =
                abi::__cxa_demangle(symbol_name, nullptr, 0, &status);
            if (status == 0) {
                oss << "\t\t" << demangled_name << " +" << offset_begin
                    << offset_end << "\n";
                free(demangled_name);
            } else {
                oss << "\t\t" << symbol_name << " +" << offset_begin
                    << offset_end << "\n";
            }
        } else {
            oss << "\t\t" << symbols.get()[i] << "\n";
        }
    }
#else
    oss << "\t\tStack trace not available on this platform.\n";
#endif
    return oss.str();
}

void StackTrace::capture() {
#ifdef _WIN32
    const int max_frames = 64;
    frames.resize(max_frames);
    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    WORD captured_frames =
        CaptureStackBackTrace(0, max_frames, frames.data(), NULL);
    frames.resize(captured_frames);
#elif defined(__APPLE__) || defined(__linux__)
    const int max_frames = 64;
    void* frame_pointers[max_frames];
    num_frames = backtrace(frame_pointers, max_frames);
    symbols.reset(backtrace_symbols(frame_pointers, num_frames));
#else
    num_frames = 0;
#endif
}
}  // namespace atom::error
