/*
 * exception.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Better Exception Library

**************************************************/

#include "exception.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#if !defined(__MINGW32__) && !defined(__MINGW64__)
#pragma comment(lib, "dbghelp.lib")
#endif
#elif defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#include <unistd.h>
#endif

namespace atom::error {
const char* Exception::what() const noexcept {
    if (full_message_.empty()) {
        std::ostringstream oss;
        oss << "[" << getCurrentTime() << "] ";
        oss << "Exception at " << file_ << ":" << line_ << " in " << func_
            << "()";
        oss << " (thread " << thread_id_ << ")";
        oss << "\n\tMessage: " << message_;
        oss << "\n\tStack trace:\n" << getStackTrace();
        full_message_ = oss.str();
    }
    return full_message_.c_str();
}

const std::string& Exception::getFile() const { return file_; }
int Exception::getLine() const { return line_; }
const std::string& Exception::getFunction() const { return func_; }
const std::string& Exception::getMessage() const { return message_; }
std::thread::id Exception::getThreadId() const { return thread_id_; }

std::string Exception::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&time));
    return buffer;
}

std::string Exception::getStackTrace() const {
    std::ostringstream oss;

#ifdef _WIN32
    const int max_frames = 64;
    void* frames[max_frames];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    WORD num_frames = CaptureStackBackTrace(0, max_frames, frames, NULL);
    std::unique_ptr<SYMBOL_INFO> symbol(
        (SYMBOL_INFO*)std::calloc(sizeof(SYMBOL_INFO) + 256, 1));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (int i = 0; i < num_frames; ++i) {
        SymFromAddr(process, (DWORD64)(frames[i]), 0, symbol.get());
        oss << "\t\t" << symbol->Name << " - 0x" << std::hex << symbol->Address
            << "\n";
    }
#elif defined(__APPLE__) || defined(__linux__)
    const int max_frames = 64;
    void* frames[max_frames];
    int num_frames = backtrace(frames, max_frames);
    std::unique_ptr<char*> symbols(backtrace_symbols(frames, num_frames));

    for (int i = 0; i < num_frames; ++i) {
        char* symbol_name = nullptr;
        char* offset_begin = nullptr;
        char* offset_end = nullptr;

        for (char* p = symbols.get()[i]; *p; ++p) {
            if (*p == '(') {
                symbol_name = p;
            } else if (*p == '+') {
                offset_begin = p;
            } else if (*p == ')') {
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
                std::free(demangled_name);
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
}  // namespace atom::error
