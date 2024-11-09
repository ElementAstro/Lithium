#include "macro.hpp"
#if defined(__GNUC__) || defined(__clang__)
// Disable all warnings from gcc/clang:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"

#pragma GCC diagnostic ignored "-Wc++98-compat"
#pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning( \
    disable : 4365)  // conversion from 'X' to 'Y', signed/unsigned mismatch
#endif

#include "loguru.hpp"

#ifndef LOGURU_HAS_BEEN_IMPLEMENTED
#define LOGURU_HAS_BEEN_IMPLEMENTED

#define LOGURU_PREAMBLE_WIDTH \
    (53 + LOGURU_THREADNAME_WIDTH + LOGURU_FILENAME_WIDTH)

#undef min
#undef max

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#if LOGURU_SYSLOG
#include <syslog.h>
#else
#define LOG_USER 0
#endif

#ifdef _WIN32
#include <direct.h>
#include <share.h>
#define localtime_r(a, b) \
    localtime_s(b, a)  // No localtime_r with MSVC, but arguments are swapped
                       // for localtime_s
#else
#include <signal.h>
#include <sys/stat.h>  // mkdir
#include <unistd.h>    // STDERR_FILENO
#endif

#ifdef __linux__
#include <linux/limits.h>  // PATH_MAX
#elif !defined(_WIN32)
#include <limits.h>  // PATH_MAX
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

// TODO: use defined(_POSIX_VERSION) for some of these things?

#if defined(_WIN32) || defined(__CYGWIN__)
#define LOGURU_PTHREADS 0
#define LOGURU_WINTHREADS 1
#ifndef LOGURU_STACKTRACES
#define LOGURU_STACKTRACES 0
#endif
#else
#define LOGURU_PTHREADS 1
#define LOGURU_WINTHREADS 0
#ifdef __GLIBC__
#ifndef LOGURU_STACKTRACES
#define LOGURU_STACKTRACES 1
#endif
#else
#ifndef LOGURU_STACKTRACES
#define LOGURU_STACKTRACES 0
#endif
#endif
#endif

#if LOGURU_STACKTRACES
#include <cxxabi.h>    // for __cxa_demangle
#include <dlfcn.h>     // for dladdr
#include <execinfo.h>  // for backtrace
#endif                 // LOGURU_STACKTRACES

#if LOGURU_PTHREADS
#include <pthread.h>
#if defined(__FreeBSD__)
#include <pthread_np.h>
#include <sys/thr.h>
#elif defined(__OpenBSD__)
#include <pthread_np.h>
#endif

#ifdef __linux__
/* On Linux, the default thread name is the same as the name of the binary.
   Additionally, all new threads inherit the name of the thread it got forked
   from. For this reason, Loguru use the pthread Thread Local Storage for
   storing thread names on Linux. */
#ifndef LOGURU_PTLS_NAMES
#define LOGURU_PTLS_NAMES 1
#endif
#endif
#endif

#if LOGURU_WINTHREADS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef LOGURU_PTLS_NAMES
#define LOGURU_PTLS_NAMES 0
#endif

LOGURU_ANONYMOUS_NAMESPACE_BEGIN

namespace loguru {
using namespace std::chrono;

#if LOGURU_WITH_FILEABS
struct FileAbs {
    char path[PATH_MAX];
    char mode_str[4];
    Verbosity verbosity;
    struct stat st;
    FILE* fp;
    bool is_reopening = false;  // to prevent recursive call in fileReopen.
    decltype(steady_clock::now()) last_check_time = steady_clock::now();
};
#else
using FileAbs = FILE*;
#endif

struct Callback {
    std::string id;
    log_handler_t callback;
    void* userData;
    Verbosity verbosity;  // Does not change!
    close_handler_t close;
    flush_handler_t flush;
    unsigned indentation;
} ATOM_ALIGNAS(128);

using CallbackVec = std::vector<Callback>;

using StringPair = std::pair<std::string, std::string>;
using StringPairList = std::vector<StringPair>;

const auto S_START_TIME = steady_clock::now();

Verbosity g_stderr_verbosity = Verbosity_0;
bool g_colorlogtostderr = true;
unsigned g_flush_interval_ms = 0;
bool g_preamble_header = true;
bool g_preamble = true;

Verbosity g_internal_verbosity = Verbosity_0;

// Preamble details
bool g_preamble_date = true;
bool g_preamble_time = true;
bool g_preamble_uptime = true;
bool g_preamble_thread = true;
bool g_preamble_file = true;
bool g_preamble_verbose = true;
bool g_preamble_pipe = true;

static std::recursive_mutex sMutex;
static Verbosity sMaxOutVerbosity = Verbosity_OFF;
static std::string sArgv0Filename;
static std::string sArguments;
static char sCurrentDir[PATH_MAX];
static CallbackVec sCallbacks;
static fatal_handler_t sFatalHandler = nullptr;
static verbosity_to_name_t sVerbosityToNameCallback = nullptr;
static name_to_verbosity_t sNameToVerbosityCallback = nullptr;
static StringPairList sUserStackCleanups;
static bool sStripFilePath = true;
static std::atomic<unsigned> sStderrIndentation{0};

// For periodic flushing:
static std::thread* sFlushThread = nullptr;
static bool sNeedsFlushing = false;

static SignalOptions sSignalOptions = SignalOptions::none();

static const bool S_TERMINAL_HAS_COLOR = []() {
#ifdef _WIN32
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        return SetConsoleMode(hOut, dwMode) != 0;
    }
    return false;
#else
    if (isatty(STDERR_FILENO) == 0) {
        return false;
    }
    if (const char* term = getenv("TERM")) {
        return 0 == strcmp(term, "cygwin") || 0 == strcmp(term, "linux") ||
               0 == strcmp(term, "rxvt-unicode-256color") ||
               0 == strcmp(term, "screen") ||
               0 == strcmp(term, "screen-256color") ||
               0 == strcmp(term, "screen.xterm-256color") ||
               0 == strcmp(term, "tmux-256color") ||
               0 == strcmp(term, "xterm") ||
               0 == strcmp(term, "xterm-256color") ||
               0 == strcmp(term, "xterm-termite") ||
               0 == strcmp(term, "xterm-color");
    }
    return false;

#endif
}();

static void printPreambleHeader(char* out_buff, size_t out_buff_size);

// ------------------------------------------------------------------------------
// Colors

auto terminal_has_color() -> bool { return S_TERMINAL_HAS_COLOR; }

// Colors

#ifdef _WIN32
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

auto terminal_black() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(30) : "";
}
auto terminal_red() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(31) : "";
}
auto terminal_green() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(32) : "";
}
auto terminal_yellow() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(33) : "";
}
auto terminal_blue() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(34) : "";
}
auto terminal_purple() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(35) : "";
}
auto terminal_cyan() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(36) : "";
}
auto terminal_light_gray() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(37) : "";
}
auto terminal_white() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(37) : "";
}
auto terminal_light_red() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(91) : "";
}
auto terminalDim() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(2) : "";
}

// Formating
auto terminal_bold() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(1) : "";
}
auto terminal_underline() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(4) : "";
}

// You should end each line with this!
auto terminal_reset() -> const char* {
    return S_TERMINAL_HAS_COLOR ? VTSEQ(0) : "";
}

// ------------------------------------------------------------------------------
#if LOGURU_WITH_FILEABS
void fileReopen(void* user_data);
inline FILE* toFile(void* user_data) {
    return reinterpret_cast<FileAbs*>(user_data)->fp;
}
#else
inline auto toFile(void* user_data) -> FILE* {
    return reinterpret_cast<FILE*>(user_data);
}
#endif

void fileLog(void* user_data, const Message& message) {
#if LOGURU_WITH_FILEABS
    FileAbs* file_abs = reinterpret_cast<FileAbs*>(user_data);
    if (file_abs->is_reopening) {
        return;
    }
    // It is better checking file change every minute/hour/day,
    // instead of doing this every time we log.
    // Here check_interval is set to zero to enable checking every time;
    const auto check_interval = seconds(0);
    if (duration_cast<seconds>(steady_clock::now() -
                               file_abs->last_check_time) > check_interval) {
        file_abs->last_check_time = steady_clock::now();
        fileReopen(user_data);
    }
    FILE* file = to_file(user_data);
    if (!file) {
        return;
    }
#else
    FILE* file = toFile(user_data);
#endif
    fprintf(file, "%s%s%s%s\n", message.preamble, message.indentation,
            message.prefix, message.message);
    if (g_flush_interval_ms == 0) {
        fflush(file);
    }
}

void fileClose(void* user_data) {
    FILE* file = toFile(user_data);
    if (file != nullptr) {
        fclose(file);
    }
#if LOGURU_WITH_FILEABS
    delete reinterpret_cast<FileAbs*>(user_data);
#endif
}

void fileFlush(void* user_data) {
    FILE* file = toFile(user_data);
    fflush(file);
}

#if LOGURU_WITH_FILEABS
void fileReopen(void* user_data) {
    FileAbs* file_abs = reinterpret_cast<FileAbs*>(user_data);
    struct stat st;
    int ret;
    if (!file_abs->fp || (ret = stat(file_abs->path, &st)) == -1 ||
        (st.st_ino != file_abs->st.st_ino)) {
        file_abs->is_reopening = true;
        if (file_abs->fp) {
            fclose(file_abs->fp);
        }
        if (!file_abs->fp) {
            VLOG_F(g_internal_verbosity,
                   "Reopening file '" LOGURU_FMT(s) "' due to previous error",
                   file_abs->path);
        } else if (ret < 0) {
            const auto why = errno_as_text();
            VLOG_F(
                g_internal_verbosity,
                "Reopening file '" LOGURU_FMT(s) "' due to '" LOGURU_FMT(s) "'",
                file_abs->path, why.c_str());
        } else {
            VLOG_F(g_internal_verbosity,
                   "Reopening file '" LOGURU_FMT(s) "' due to file changed",
                   file_abs->path);
        }
        // try reopen current file.
        if (!create_directories(file_abs->path)) {
            LOG_F(ERROR, "Failed to create directories to '" LOGURU_FMT(s) "'",
                  file_abs->path);
        }
        file_abs->fp = fopen(file_abs->path, file_abs->mode_str);
        if (!file_abs->fp) {
            LOG_F(ERROR, "Failed to open '" LOGURU_FMT(s) "'", file_abs->path);
        } else {
            stat(file_abs->path, &file_abs->st);
        }
        file_abs->is_reopening = false;
    }
}
#endif
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#if LOGURU_SYSLOG
void syslogLog(void* /*user_data*/, const Message& message) {
    /*
            Level 0: Is reserved for kernel panic type situations.
            Level 1: Is for Major resource failure.
            Level 2->7 Application level failures
    */
    int level;
    if (message.verbosity < Verbosity_FATAL) {
        level = 1;  // System Alert
    } else {
        switch (message.verbosity) {
            case Verbosity_FATAL:
                level = 2;
                break;  // System Critical
            case Verbosity_ERROR:
                level = 3;
                break;  // System Error
            case Verbosity_WARNING:
                level = 4;
                break;  // System Warning
            case Verbosity_INFO:
                level = 5;
                break;  // System Notice
            case Verbosity_1:
                level = 6;
                break;  // System Info
            default:
                level = 7;
                break;  // System Debug
        }
    }

    // Note: We don't add the time info.
    // This is done automatically by the syslog deamon.
    // Otherwise log all information that the file log does.
    syslog(level, "%s%s%s", message.indentation, message.prefix,
           message.message);
}

void syslogClose(void* /*user_data*/) { closelog(); }

void syslogFlush(void* /*user_data*/) {}
#endif
// ------------------------------------------------------------------------------
// Helpers:

Text::~Text() { free(str_); }

#if LOGURU_USE_FMTLIB
#if __cplusplus >= 202002L
auto vtextprintf(const char* format, std::format_args args) -> Text {
    return Text(STRDUP(std::vformat(format, args).c_str()));
}
#else
Text vtextprintf(const char* format, fmt::format_args args) {
    return Text(STRDUP(fmt::vformat(format, args).c_str()));
}
#endif
#else
LOGURU_PRINTF_LIKE(1, 0)
static Text vtextprintf(const char* format, va_list vlist) {
#ifdef _WIN32
    int bytes_needed = _vscprintf(format, vlist);
    CHECK_F(bytes_needed >= 0, "Bad string format: '%s'", format);
    char* buff = (char*)malloc(bytes_needed + 1);
    vsnprintf(buff, bytes_needed + 1, format, vlist);
    return Text(buff);
#else
    char* buff = nullptr;
    int result = vasprintf(&buff, format, vlist);
    CHECK_F(result >= 0, "Bad string format: '" LOGURU_FMT(s) "'", format);
    return Text(buff);
#endif
}

Text textprintf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    auto result = vtextprintf(format, vlist);
    va_end(vlist);
    return result;
}
#endif

// Overloaded for variadic template matching.
auto textprintf() -> Text { return Text(static_cast<char*>(calloc(1, 1))); }

static const char* indentation(unsigned depth) {
    static const char buff[] =
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   "
        ".   .   .   .   .   .   .   .   .   .   ";
    static const size_t INDENTATION_WIDTH = 4;
    static const size_t NUM_INDENTATIONS =
        (sizeof(buff) - 1) / INDENTATION_WIDTH;
    depth = std::min<unsigned>(depth, NUM_INDENTATIONS);
    return buff + INDENTATION_WIDTH * (NUM_INDENTATIONS - depth);
}

static void parseArgs(int& argc, char* argv[], const char* verbosity_flag) {
    int argDest = 1;
    int outArgc = argc;

    for (int argIt = 1; argIt < argc; ++argIt) {
        auto* cmd = argv[argIt];
        auto argLen = strlen(verbosity_flag);

        bool lastIsAlpha = false;
#if LOGURU_USE_LOCALE
        try {  // locale variant of isalpha will throw on error
            lastIsAlpha = std::isalpha(cmd[argLen], std::locale(""));
        } catch (...) {
            lastIsAlpha = std::isalpha(static_cast<int>(cmd[argLen]));
        }
#else
        lastIsAlpha = std::isalpha(static_cast<int>(cmd[argLen]));
#endif

        if (strncmp(cmd, verbosity_flag, argLen) == 0 && !lastIsAlpha) {
            outArgc -= 1;
            auto* valueStr = cmd + argLen;
            if (valueStr[0] == '\0') {
                // Value in separate argument
                argIt += 1;
                CHECK_LT_F(argIt, argc,
                           "Missing verbosiy level after " LOGURU_FMT(s) "",
                           verbosity_flag);
                valueStr = argv[argIt];
                outArgc -= 1;
            }
            if (*valueStr == '=') {
                valueStr += 1;
            }

            auto reqVerbosity = get_verbosity_from_name(valueStr);
            if (reqVerbosity != Verbosity_INVALID) {
                g_stderr_verbosity = reqVerbosity;
            } else {
                char* end = 0;
                g_stderr_verbosity =
                    static_cast<int>(strtol(valueStr, &end, 10));
                CHECK_F(end && *end == '\0',
                        "Invalid verbosity. Expected integer, INFO, WARNING, "
                        "ERROR or OFF, got '" LOGURU_FMT(s) "'",
                        valueStr);
            }
        } else {
            argv[argDest++] = argv[argIt];
        }
    }

    argc = outArgc;
    argv[argc] = nullptr;
}

static auto nowNs() -> long long {
    return duration_cast<nanoseconds>(
               high_resolution_clock::now().time_since_epoch())
        .count();
}

// Returns the part of the path after the last / or \ (if any).
auto filename(const char* path) -> const char* {
    for (const auto* ptr = path; *ptr != 0; ++ptr) {
        if (*ptr == '/' || *ptr == '\\') {
            path = ptr + 1;
        }
    }
    return path;
}

// ------------------------------------------------------------------------------

static void onAtexit() {
    VLOG_F(g_internal_verbosity, "atexit");
    flush();
}

static void installSignalHandlers(const SignalOptions& signal_options);

static void writeHexDigit(std::string& out, unsigned num) {
    DCHECK_LT_F(num, 16u);
    out.push_back(num < 10u ? char('0' + num) : char('A' + num - 10));
}

static void writeHexByte(std::string& out, uint8_t n) {
    writeHexDigit(out, n >> 4u);
    writeHexDigit(out, n & 0x0f);
}

static void escape(std::string& out, const std::string& str) {
    for (char c : str) {
        /**/ if (c == '\a') {
            out += "\\a";
        } else if (c == '\b') {
            out += "\\b";
        } else if (c == '\f') {
            out += "\\f";
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else if (c == '\v') {
            out += "\\v";
        } else if (c == '\\') {
            out += "\\\\";
        } else if (c == '\'') {
            out += "\\\'";
        } else if (c == '\"') {
            out += "\\\"";
        } else if (c == ' ') {
            out += "\\ ";
        } else if (0 <= c && c < 0x20) {  // ASCI control character:
            // else if (c < 0x20 || c != (c & 127)) { // ASCII control character
            // or UTF-8:
            out += "\\x";
            writeHexByte(out, static_cast<uint8_t>(c));
        } else {
            out += c;
        }
    }
}

auto errno_as_text() -> Text {
    char buff[256];
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
    // GNU Version
    return Text(STRDUP(strerror_r(errno, buff, sizeof(buff))));
#elif defined(__APPLE__) || _POSIX_C_SOURCE >= 200112L
    // XSI Version
    strerror_r(errno, buff, sizeof(buff));
    return Text(strdup(buff));
#elif defined(_WIN32)
    strerror_s(buff, sizeof(buff), errno);
    return Text(STRDUP(buff));
#else
    // Not thread-safe.
    return Text(STRDUP(strerror(errno)));
#endif
}

void init(int& argc, char* argv[], const Options& options) {
    CHECK_GT_F(argc, 0, "Expected proper argc/argv");
    CHECK_EQ_F(argv[argc], nullptr, "Expected proper argc/argv");

    sArgv0Filename = filename(argv[0]);

#ifdef _WIN32
#define getcwd _getcwd
#endif

    if (!getcwd(sCurrentDir, sizeof(sCurrentDir))) {
        const auto ERROR_TEXT = errno_as_text();
        LOG_F(WARNING,
              "Failed to get current working directory: " LOGURU_FMT(s) "",
              ERROR_TEXT.c_str());
    }

    sArguments = "";
    for (int i = 0; i < argc; ++i) {
        escape(sArguments, argv[i]);
        if (i + 1 < argc) {
            sArguments += " ";
        }
    }

    if (options.verbosity_flag != nullptr) {
        parseArgs(argc, argv, options.verbosity_flag);
    }

    if (const auto main_thread_name = options.main_thread_name) {
#if LOGURU_PTLS_NAMES || LOGURU_WINTHREADS
        set_thread_name(main_thread_name);
#elif LOGURU_PTHREADS
        char old_thread_name[16] = {0};
        auto this_thread = pthread_self();
#if defined(__APPLE__) || defined(__linux__) || defined(__sun)
        pthread_getname_np(this_thread, old_thread_name,
                           sizeof(old_thread_name));
#endif
        if (old_thread_name[0] == 0) {
#ifdef __APPLE__
            pthread_setname_np(main_thread_name);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
            pthread_set_name_np(this_thread, main_thread_name);
#elif defined(__linux__) || defined(__sun)
            pthread_setname_np(this_thread, main_thread_name);
#endif
        }
#endif  // LOGURU_PTHREADS
    }

    if (g_stderr_verbosity >= Verbosity_INFO) {
        if (g_preamble_header) {
            char preambleExplain[LOGURU_PREAMBLE_WIDTH];
            printPreambleHeader(preambleExplain, sizeof(preambleExplain));
            if (g_colorlogtostderr && S_TERMINAL_HAS_COLOR) {
                fprintf(stderr, "%s%s%s\n", terminal_reset(), terminalDim(),
                        preambleExplain);
            } else {
                fprintf(stderr, "%s\n", preambleExplain);
            }
        }
        fflush(stderr);
    }
    VLOG_F(g_internal_verbosity, "arguments: " LOGURU_FMT(s) "",
           sArguments.c_str());
    if (strlen(sCurrentDir) != 0) {
        VLOG_F(g_internal_verbosity, "Current dir: " LOGURU_FMT(s) "",
               sCurrentDir);
    }
    VLOG_F(g_internal_verbosity, "stderr verbosity: " LOGURU_FMT(d) "",
           g_stderr_verbosity);
    VLOG_F(g_internal_verbosity, "-----------------------------------");

    installSignalHandlers(options.signal_options);

    atexit(onAtexit);
}

void shutdown() {
    VLOG_F(g_internal_verbosity, "loguru::shutdown()");
    remove_all_callbacks();
    set_fatal_handler(nullptr);
    set_verbosity_to_name_callback(nullptr);
    set_name_to_verbosity_callback(nullptr);
}

void write_date_time(char* buff, unsigned long long buff_size) {
    auto now = system_clock::now();
    long long ms_since_epoch =
        duration_cast<milliseconds>(now.time_since_epoch()).count();
    time_t sec_since_epoch = time_t(ms_since_epoch / 1000);
    tm time_info;
    localtime_r(&sec_since_epoch, &time_info);
    snprintf(buff, buff_size, "%04d%02d%02d_%02d%02d%02d.%03lld",
             1900 + time_info.tm_year, 1 + time_info.tm_mon, time_info.tm_mday,
             time_info.tm_hour, time_info.tm_min, time_info.tm_sec,
             ms_since_epoch % 1000);
}

auto argv0_filename() -> const char* { return sArgv0Filename.c_str(); }

auto arguments() -> const char* { return sArguments.c_str(); }

auto current_dir() -> const char* { return sCurrentDir; }

const char* home_dir() {
#ifdef __MINGW32__
    auto home = getenv("USERPROFILE");
    CHECK_F(home != nullptr, "Missing USERPROFILE");
    return home;
#elif defined(_WIN32)
    char* user_profile;
    size_t len;
    errno_t err = _dupenv_s(&user_profile, &len, "USERPROFILE");
    CHECK_F(err == 0, "Missing USERPROFILE");
    return user_profile;
#else   // _WIN32
    auto home = getenv("HOME");
    CHECK_F(home != nullptr, "Missing HOME");
    return home;
#endif  // _WIN32
}

void suggest_log_path(const char* prefix, char* buff,
                      unsigned long long buff_size) {
    if (prefix[0] == '~') {
        snprintf(buff, buff_size - 1, "%s%s", home_dir(), prefix + 1);
    } else {
        snprintf(buff, buff_size - 1, "%s", prefix);
    }

    // Check for terminating /
    size_t n = strlen(buff);
    if (n != 0) {
        if (buff[n - 1] != '/') {
            CHECK_F(n + 2 < buff_size, "Filename buffer too small");
            buff[n] = '/';
            buff[n + 1] = '\0';
        }
    }

#ifdef _WIN32
    strncat_s(buff, buff_size - strlen(buff) - 1, s_argv0_filename.c_str(),
              buff_size - strlen(buff) - 1);
    strncat_s(buff, buff_size - strlen(buff) - 1, "/",
              buff_size - strlen(buff) - 1);
    write_date_time(buff + strlen(buff), buff_size - strlen(buff));
    strncat_s(buff, buff_size - strlen(buff) - 1, ".log",
              buff_size - strlen(buff) - 1);
#else
    strncat(buff, sArgv0Filename.c_str(), buff_size - strlen(buff) - 1);
    strncat(buff, "/", buff_size - strlen(buff) - 1);
    write_date_time(buff + strlen(buff), buff_size - strlen(buff));
    strncat(buff, ".log", buff_size - strlen(buff) - 1);
#endif
}

bool create_directories(const char* file_path_const) {
    CHECK_F(file_path_const && *file_path_const);
    char* file_path = STRDUP(file_path_const);
    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';

#ifdef _WIN32
        if (_mkdir(file_path) == -1) {
#else
        if (mkdir(file_path, 0755) == -1) {
#endif
            if (errno != EEXIST) {
                LOG_F(ERROR, "Failed to create directory '" LOGURU_FMT(s) "'",
                      file_path);
                LOG_IF_F(ERROR, errno == EACCES, "EACCES");
                LOG_IF_F(ERROR, errno == ENAMETOOLONG, "ENAMETOOLONG");
                LOG_IF_F(ERROR, errno == ENOENT, "ENOENT");
                LOG_IF_F(ERROR, errno == ENOTDIR, "ENOTDIR");
                LOG_IF_F(ERROR, errno == ELOOP, "ELOOP");

                *p = '/';
                free(file_path);
                return false;
            }
        }
        *p = '/';
    }
    free(file_path);
    return true;
}
bool add_file(const char* path_in, FileMode mode, Verbosity verbosity) {
    char path[PATH_MAX];
    if (path_in[0] == '~') {
        snprintf(path, sizeof(path) - 1, "%s%s", home_dir(), path_in + 1);
    } else {
        snprintf(path, sizeof(path) - 1, "%s", path_in);
    }

    if (!create_directories(path)) {
        LOG_F(ERROR, "Failed to create directories to '" LOGURU_FMT(s) "'",
              path);
    }

    const char* mode_str = (mode == FileMode::Truncate ? "w" : "a");
    FILE* file;
#ifdef _WIN32
    file = _fsopen(path, mode_str, _SH_DENYNO);
#else
    file = fopen(path, mode_str);
#endif
    if (!file) {
        LOG_F(ERROR, "Failed to open '" LOGURU_FMT(s) "'", path);
        return false;
    }
#if LOGURU_WITH_FILEABS
    FileAbs* file_abs = new FileAbs();  // this is deleted in file_close;
    snprintf(file_abs->path, sizeof(file_abs->path) - 1, "%s", path);
    snprintf(file_abs->mode_str, sizeof(file_abs->mode_str) - 1, "%s",
             mode_str);
    stat(file_abs->path, &file_abs->st);
    file_abs->fp = file;
    file_abs->verbosity = verbosity;
    add_callback(path_in, file_log, file_abs, verbosity, file_close,
                 file_flush);
#else
    add_callback(path_in, fileLog, file, verbosity, fileClose, fileFlush);
#endif

    if (mode == FileMode::Append) {
        fprintf(file, "\n\n\n\n\n");
    }
    if (!sArguments.empty()) {
        fprintf(file, "arguments: %s\n", sArguments.c_str());
    }
    if (strlen(sCurrentDir) != 0) {
        fprintf(file, "Current dir: %s\n", sCurrentDir);
    }
    fprintf(file, "File verbosity level: %d\n", verbosity);
    if (g_preamble_header) {
        char preamble_explain[LOGURU_PREAMBLE_WIDTH];
        printPreambleHeader(preamble_explain, sizeof(preamble_explain));
        fprintf(file, "%s\n", preamble_explain);
    }
    fflush(file);

    VLOG_F(g_internal_verbosity,
           "Logging to '" LOGURU_FMT(s) "', mode: '" LOGURU_FMT(
               s) "', verbosity: " LOGURU_FMT(d) "",
           path, mode_str, verbosity);
    return true;
}

/*
        Will add syslog as a standard sink for log messages
        Any logging message with a verbosity lower or equal to
        the given verbosity will be included.

        This works for Unix like systems (i.e. Linux/Mac)
        There is no current implementation for Windows (as I don't know the
        equivalent calls or have a way to test them). If you know please
        add and send a pull request.

        The code should still compile under windows but will only generate
        a warning message that syslog is unavailable.

        Search for LOGURU_SYSLOG to find and fix.
*/
auto add_syslog(const char* app_name, Verbosity verbosity) -> bool {
    return add_syslog(app_name, verbosity, LOG_USER);
}
auto add_syslog(const char* app_name, Verbosity verbosity,
                int facility) -> bool {
#if LOGURU_SYSLOG
    if (app_name == nullptr) {
        app_name = argv0_filename();
    }
    openlog(app_name, 0, facility);
    add_callback("'syslog'", syslogLog, nullptr, verbosity, syslogClose,
                 syslogFlush);

    VLOG_F(g_internal_verbosity,
           "Logging to 'syslog' , verbosity: " LOGURU_FMT(d) "", verbosity);
    return true;
#else
    (void)app_name;
    (void)verbosity;
    (void)facility;
    VLOG_F(g_internal_verbosity,
           "syslog not implemented on this system. Request to install syslog "
           "logging ignored.");
    return false;
#endif
}
// Will be called right before abort().
void set_fatal_handler(fatal_handler_t handler) { sFatalHandler = handler; }

fatal_handler_t get_fatal_handler() { return sFatalHandler; }

void set_verbosity_to_name_callback(verbosity_to_name_t callback) {
    sVerbosityToNameCallback = callback;
}

void set_name_to_verbosity_callback(name_to_verbosity_t callback) {
    sNameToVerbosityCallback = callback;
}

void add_stack_cleanup(const char* find_this, const char* replace_with_this) {
    if (strlen(find_this) <= strlen(replace_with_this)) {
        LOG_F(WARNING,
              "add_stack_cleanup: the replacement should be shorter than the "
              "pattern!");
        return;
    }

    sUserStackCleanups.emplace_back(find_this, replace_with_this);
}

static void on_callback_change() {
    sMaxOutVerbosity = Verbosity_OFF;
    for (const auto& callback : sCallbacks) {
        sMaxOutVerbosity = std::max(sMaxOutVerbosity, callback.verbosity);
    }
}

void add_callback(const char* id, log_handler_t callback, void* user_data,
                  Verbosity verbosity, close_handler_t on_close,
                  flush_handler_t on_flush) {
    std::lock_guard lock(sMutex);
    sCallbacks.push_back(
        Callback{id, callback, user_data, verbosity, on_close, on_flush, 0});
    on_callback_change();
}

// Returns a custom verbosity name if one is available, or nullptr.
// See also set_verbosity_to_name_callback.
auto get_verbosity_name(Verbosity verbosity) -> const char* {
    const auto* name = sVerbosityToNameCallback
                           ? (*sVerbosityToNameCallback)(verbosity)
                           : nullptr;

    // Use standard replacements if callback fails:
    if (name == nullptr) {
        if (verbosity <= Verbosity_FATAL) {
            name = "FATL";
        } else if (verbosity == Verbosity_ERROR) {
            name = "ERR";
        } else if (verbosity == Verbosity_WARNING) {
            name = "WARN";
        } else if (verbosity == Verbosity_INFO) {
            name = "INFO";
        }
    }

    return name;
}

// Returns Verbosity_INVALID if the name is not found.
// See also set_name_to_verbosity_callback.
auto get_verbosity_from_name(const char* name) -> Verbosity {
    auto verbosity = sNameToVerbosityCallback
                         ? (*sNameToVerbosityCallback)(name)
                         : Verbosity_INVALID;

    // Use standard replacements if callback fails:
    if (verbosity == Verbosity_INVALID) {
        if (strcmp(name, "OFF") == 0) {
            verbosity = Verbosity_OFF;
        } else if (strcmp(name, "INFO") == 0) {
            verbosity = Verbosity_INFO;
        } else if (strcmp(name, "WARNING") == 0) {
            verbosity = Verbosity_WARNING;
        } else if (strcmp(name, "ERROR") == 0) {
            verbosity = Verbosity_ERROR;
        } else if (strcmp(name, "FATAL") == 0) {
            verbosity = Verbosity_FATAL;
        }
    }

    return verbosity;
}

auto remove_callback(const char* id) -> bool {
    std::lock_guard lock(sMutex);
    auto it = std::find_if(begin(sCallbacks), end(sCallbacks),
                           [&](const Callback& c) { return c.id == id; });
    if (it != sCallbacks.end()) {
        if (it->close) {
            it->close(it->userData);
        }
        sCallbacks.erase(it);
        on_callback_change();
        return true;
    }
    LOG_F(ERROR, "Failed to locate callback with id '" LOGURU_FMT(s) "'", id);
    return false;
}

void remove_all_callbacks() {
    std::lock_guard lock(sMutex);
    for (auto& callback : sCallbacks) {
        if (callback.close != nullptr) {
            callback.close(callback.userData);
        }
    }
    sCallbacks.clear();
    on_callback_change();
}

// Returns the maximum of g_stderr_verbosity and all file/custom outputs.
auto current_verbosity_cutoff() -> Verbosity {
    return g_stderr_verbosity > sMaxOutVerbosity ? g_stderr_verbosity
                                                 : sMaxOutVerbosity;
}

// ------------------------------------------------------------------------
// Threads names

#if LOGURU_PTLS_NAMES
static pthread_once_t s_pthread_key_once = PTHREAD_ONCE_INIT;
static pthread_key_t s_pthread_key_name;

void make_pthread_key_name() {
    (void)pthread_key_create(&s_pthread_key_name, free);
}
#endif

#if LOGURU_WINTHREADS
// Where we store the custom thread name set by `set_thread_name`
char* thread_name_buffer() {
    thread_local static char thread_name[LOGURU_THREADNAME_WIDTH + 1] = {0};
    return &thread_name[0];
}
#endif  // LOGURU_WINTHREADS

void set_thread_name(const char* name) {
#if LOGURU_PTLS_NAMES
    // Store thread name in thread-local storage at `s_pthread_key_name`
    (void)pthread_once(&s_pthread_key_once, make_pthread_key_name);
    (void)pthread_setspecific(s_pthread_key_name, STRDUP(name));
#elif LOGURU_PTHREADS
// Tell the OS the thread name
#ifdef __APPLE__
    pthread_setname_np(name);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    pthread_set_name_np(pthread_self(), name);
#elif defined(__linux__) || defined(__sun)
    pthread_setname_np(pthread_self(), name);
#endif
#elif LOGURU_WINTHREADS
    // Store thread name in a thread-local storage:
    strncpy_s(thread_name_buffer(), LOGURU_THREADNAME_WIDTH + 1, name,
              _TRUNCATE);
#else   // LOGURU_PTHREADS
        // TODO: on these weird platforms we should also store the thread name
        // in a generic thread-local storage.
    (void)name;
#endif  // LOGURU_PTHREADS
}

void get_thread_name(char* buffer, unsigned long long length,
                     bool right_align_hex_id) {
    CHECK_NE_F(length, 0u, "Zero length buffer in get_thread_name");
    CHECK_NOTNULL_F(buffer, "nullptr in get_thread_name");

#if LOGURU_PTLS_NAMES
    (void)pthread_once(&s_pthread_key_once, make_pthread_key_name);
    if (const char* name =
            static_cast<const char*>(pthread_getspecific(s_pthread_key_name))) {
        snprintf(buffer, static_cast<size_t>(length), "%s", name);
    } else {
        buffer[0] = 0;
    }
#elif LOGURU_PTHREADS
    // Ask the OS about the thread name.
    // This is what we *want* to do on all platforms, but
    // only some platforms support it (currently).
    pthread_getname_np(pthread_self(), buffer, length);
#elif LOGURU_WINTHREADS
    snprintf(buffer, static_cast<size_t>(length), "%s", thread_name_buffer());
#else
    // Thread names unsupported
    buffer[0] = 0;
#endif

    if (buffer[0] == 0) {
        // We failed to get a readable thread name.
        // Write a HEX thread ID instead.
        // We try to get an ID that is the same as the ID you could
        // read in your debugger, system monitor etc.

#ifdef __APPLE__
        uint64_t thread_id;
        pthread_threadid_np(pthread_self(), &thread_id);
#elif defined(__FreeBSD__)
        long thread_id;
        (void)thr_self(&thread_id);
#elif LOGURU_PTHREADS
        uint64_t thread_id = pthread_self();
#else
        // This ID does not correllate to anything we can get from the OS,
        // so this is the worst way to get the ID.
        const auto thread_id =
            std::hash<std::thread::id>{}(std::this_thread::get_id());
#endif

        if (right_align_hex_id) {
            snprintf(buffer, static_cast<size_t>(length), "%*X",
                     static_cast<int>(length - 1),
                     static_cast<unsigned>(thread_id));
        } else {
            snprintf(buffer, static_cast<size_t>(length), "%X",
                     static_cast<unsigned>(thread_id));
        }
    }
}

// ------------------------------------------------------------------------
// Stack traces

#if LOGURU_STACKTRACES
auto demangle(const char* name) -> Text {
    int status = -1;
    char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
    Text result{status == 0 ? demangled : STRDUP(name)};
    return result;
}

#if LOGURU_RTTI
template <class T>
std::string type_name() {
    auto demangled = demangle(typeid(T).name());
    return demangled.c_str();
}
#endif  // LOGURU_RTTI

static const StringPairList REPLACE_LIST = {
#if LOGURU_RTTI
    {type_name<std::string>(), "std::string"},
    {type_name<std::wstring>(), "std::wstring"},
    {type_name<std::u16string>(), "std::u16string"},
    {type_name<std::u32string>(), "std::u32string"},
#endif  // LOGURU_RTTI
    {"std::__1::", "std::"},
    {"__thiscall ", ""},
    {"__cdecl ", ""},
};

void do_replacements(const StringPairList& replacements, std::string& str) {
    for (auto&& p : replacements) {
        if (p.first.size() <= p.second.size()) {
            // On gcc, "type_name<std::string>()" is "std::string"
            continue;
        }

        size_t it;
        while ((it = str.find(p.first)) != std::string::npos) {
            str.replace(it, p.first.size(), p.second);
        }
    }
}

std::string prettify_stacktrace(const std::string& input) {
    std::string output = input;

    do_replacements(sUserStackCleanups, output);
    do_replacements(REPLACE_LIST, output);

    try {
        std::regex std_allocator_re(R"(,\s*std::allocator<[^<>]+>)");
        output = std::regex_replace(output, std_allocator_re, std::string(""));

        std::regex template_spaces_re(R"(<\s*([^<> ]+)\s*>)");
        output =
            std::regex_replace(output, template_spaces_re, std::string("<$1>"));
    } catch (std::regex_error&) {
        // Probably old GCC.
    }

    return output;
}

std::string stacktrace_as_stdstring(int skip) {
    void* callstack[128];
    const auto max_frames = sizeof(callstack) / sizeof(callstack[0]);
    int numFrames = backtrace(callstack, max_frames);

    // Using unique_ptr with custom deleter to manage memory safely
    std::unique_ptr<char*, decltype(&free)> symbols(
        backtrace_symbols(callstack, numFrames), free);

    std::ostringstream oss;

    // Print stack traces so the most relevant ones are written last
    for (int i = numFrames - 1; i >= skip; --i) {
        Dl_info info;
        if ((dladdr(callstack[i], &info) != 0) && info.dli_sname) {
            int status = 0;
            std::unique_ptr<char, decltype(&free)> demangled(
                info.dli_sname[0] == '_'
                    ? abi::__cxa_demangle(info.dli_sname, nullptr, nullptr,
                                          &status)
                    : nullptr,
                free);

            oss << std::dec << i - skip << " " << callstack[i] << " "
                << (status == 0 && demangled      ? demangled.get()
                    : (info.dli_sname != nullptr) ? info.dli_sname
                                                  : symbols.get()[i])
                << " + "
                << static_cast<char*>(callstack[i]) -
                       static_cast<char*>(info.dli_saddr)
                << '\n';
        } else {
            oss << std::dec << i - skip << " " << callstack[i] << " "
                << symbols.get()[i] << '\n';
        }
    }

    std::string result = oss.str();

    if (numFrames == max_frames) {
        result = "[truncated]\n" + result;
    }

    // Trim the trailing newline if present
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return prettify_stacktrace(result);
}

#else  // LOGURU_STACKTRACES
Text demangle(const char* name) { return Text(STRDUP(name)); }

std::string stacktrace_as_stdstring(int) {
    // No stacktraces available on this platform"
    return "";
}

#endif  // LOGURU_STACKTRACES

auto stacktrace(int skip) -> Text {
    auto str = stacktrace_as_stdstring(skip + 1);
    return Text(STRDUP(str.c_str()));
}

// ------------------------------------------------------------------------

static void printPreambleHeader(char* out_buff, size_t out_buff_size) {
    if (out_buff_size == 0) {
        return;
    }
    out_buff[0] = '\0';
    size_t pos = 0;
    if (g_preamble_date && pos < out_buff_size) {
        int bytes =
            snprintf(out_buff + pos, out_buff_size - pos, "date       ");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_time && pos < out_buff_size) {
        int bytes =
            snprintf(out_buff + pos, out_buff_size - pos, "time         ");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_uptime && pos < out_buff_size) {
        int bytes =
            snprintf(out_buff + pos, out_buff_size - pos, "( uptime  ) ");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_thread && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "[%-*s]",
                             LOGURU_THREADNAME_WIDTH, " thread name/id");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_file && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%*s:line  ",
                             LOGURU_FILENAME_WIDTH, "file");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_verbose && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "   v");
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_pipe && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "| ");
        if (bytes > 0) {
            pos += bytes;
        }
    }
}

static void print_preamble(char* out_buff, size_t out_buff_size,
                           Verbosity verbosity, const char* file,
                           unsigned line) {
    if (out_buff_size == 0) {
        return;
    }
    out_buff[0] = '\0';
    if (!g_preamble) {
        return;
    }
    long long msSinceEpoch =
        duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count();
    time_t secSinceEpoch = time_t(msSinceEpoch / 1000);
    tm timeInfo;
    localtime_r(&secSinceEpoch, &timeInfo);

    auto uptimeMs =
        duration_cast<milliseconds>(steady_clock::now() - S_START_TIME).count();
    auto uptimeSec = static_cast<double>(uptimeMs) / 1000.0;

    char threadName[LOGURU_THREADNAME_WIDTH + 1] = {0};
    get_thread_name(threadName, LOGURU_THREADNAME_WIDTH + 1, true);

    if (sStripFilePath) {
        file = filename(file);
    }

    char levelBuff[6];
    const char* customLevelName = get_verbosity_name(verbosity);
    if (customLevelName != nullptr) {
        snprintf(levelBuff, sizeof(levelBuff) - 1, "%s", customLevelName);
    } else {
        snprintf(levelBuff, sizeof(levelBuff) - 1, "% 4d",
                 static_cast<int8_t>(verbosity));
    }

    size_t pos = 0;

    if (g_preamble_date && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos,
                             "%04d-%02d-%02d ", 1900 + timeInfo.tm_year,
                             1 + timeInfo.tm_mon, timeInfo.tm_mday);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_time && pos < out_buff_size) {
        int bytes =
            snprintf(out_buff + pos, out_buff_size - pos,
                     "%02d:%02d:%02d.%03lld ", timeInfo.tm_hour,
                     timeInfo.tm_min, timeInfo.tm_sec, msSinceEpoch % 1000);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_uptime && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "(%8.3fs) ",
                             uptimeSec);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_thread && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "[%-*s]",
                             LOGURU_THREADNAME_WIDTH, threadName);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_file && pos < out_buff_size) {
        char shortenedFilename[LOGURU_FILENAME_WIDTH + 1];
        snprintf(shortenedFilename, LOGURU_FILENAME_WIDTH + 1, "%s", file);
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "%*s:%-5u ",
                             LOGURU_FILENAME_WIDTH, shortenedFilename, line);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_verbose && pos < out_buff_size) {
        int bytes =
            snprintf(out_buff + pos, out_buff_size - pos, "%4s", levelBuff);
        if (bytes > 0) {
            pos += bytes;
        }
    }
    if (g_preamble_pipe && pos < out_buff_size) {
        int bytes = snprintf(out_buff + pos, out_buff_size - pos, "| ");
        if (bytes > 0) {
            pos += bytes;
        }
    }
}

// stack_trace_skip is just if verbosity == FATAL.
static void log_message(int stack_trace_skip, Message& message,
                        bool with_indentation, bool abort_if_fatal) {
    const auto verbosity = message.verbosity;
    std::lock_guard lock(sMutex);

    if (message.verbosity == Verbosity_FATAL) {
        auto st = loguru::stacktrace(stack_trace_skip + 2);
        if (!st.empty()) {
            RAW_LOG_F(ERROR, "Stack trace:\n" LOGURU_FMT(s) "", st.c_str());
        }

        auto ec = loguru::get_error_context();
        if (!ec.empty()) {
            RAW_LOG_F(ERROR, "" LOGURU_FMT(s) "", ec.c_str());
        }
    }

    if (with_indentation) {
        message.indentation = indentation(sStderrIndentation);
    }

    if (verbosity <= g_stderr_verbosity) {
        if (g_colorlogtostderr && S_TERMINAL_HAS_COLOR) {
            if (verbosity > Verbosity_WARNING) {
                fprintf(stderr, "%s%s%s%s%s%s%s%s\n", terminal_reset(),
                        terminalDim(), message.preamble, message.indentation,
                        verbosity == Verbosity_INFO ? terminal_reset()
                                                    : "",  // un-dim for info
                        message.prefix, message.message, terminal_reset());
            } else {
                fprintf(stderr, "%s%s%s%s%s%s%s\n", terminal_reset(),
                        verbosity == Verbosity_WARNING ? terminal_yellow()
                                                       : terminal_red(),
                        message.preamble, message.indentation, message.prefix,
                        message.message, terminal_reset());
            }
        } else {
            fprintf(stderr, "%s%s%s%s\n", message.preamble, message.indentation,
                    message.prefix, message.message);
        }

        if (g_flush_interval_ms == 0) {
            fflush(stderr);
        } else {
            sNeedsFlushing = true;
        }
    }

    for (auto& p : sCallbacks) {
        if (verbosity <= p.verbosity) {
            if (with_indentation) {
                message.indentation = indentation(p.indentation);
            }
            p.callback(p.userData, message);
            if (g_flush_interval_ms == 0) {
                if (p.flush) {
                    p.flush(p.userData);
                }
            } else {
                sNeedsFlushing = true;
            }
        }
    }

    if (g_flush_interval_ms > 0 && (sFlushThread == nullptr)) {
        sFlushThread = new std::thread([]() {
            for (;;) {
                if (sNeedsFlushing) {
                    flush();
                }
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(g_flush_interval_ms));
            }
        });
    }

    if (message.verbosity == Verbosity_FATAL) {
        flush();

        if (sFatalHandler != nullptr) {
            sFatalHandler(message);
            flush();
        }

        if (abort_if_fatal) {
#if !defined(_WIN32)
            if (sSignalOptions.sigabrt) {
                // Make sure we don't catch our own abort:
                signal(SIGABRT, SIG_DFL);
            }
#endif
            abort();
        }
    }
}

// stack_trace_skip is just if verbosity == FATAL.
void log_to_everywhere(int stack_trace_skip, Verbosity verbosity,
                       const char* file, unsigned line, const char* prefix,
                       const char* buff) {
    char preambleBuff[LOGURU_PREAMBLE_WIDTH];
    print_preamble(preambleBuff, sizeof(preambleBuff), verbosity, file, line);
    auto message =
        Message{verbosity, file, line, preambleBuff, "", prefix, buff};
    log_message(stack_trace_skip + 1, message, true, true);
}

#if LOGURU_USE_FMTLIB
#if __cplusplus >= 202002L
void vlog(Verbosity verbosity, const char* file, unsigned line,
          const char* format, std::format_args args) {
    auto formatted = std::vformat(format, args);
    log_to_everywhere(1, verbosity, file, line, "", formatted.c_str());
}

void raw_vlog(Verbosity verbosity, const char* file, unsigned line,
              const char* format, std::format_args args) {
    auto formatted = std::vformat(format, args);
    auto message =
        Message{verbosity, file, line, "", "", "", formatted.c_str()};
    log_message(1, message, false, true);
}
#else
void vlog(Verbosity verbosity, const char* file, unsigned line,
          const char* format, fmt::format_args args) {
    auto formatted = fmt::vformat(format, args);
    log_to_everywhere(1, verbosity, file, line, "", formatted.c_str());
}

void raw_vlog(Verbosity verbosity, const char* file, unsigned line,
              const char* format, fmt::format_args args) {
    auto formatted = fmt::vformat(format, args);
    auto message =
        Message{verbosity, file, line, "", "", "", formatted.c_str()};
    log_message(1, message, false, true);
}
#endif
#else
void log(Verbosity verbosity, const char* file, unsigned line,
         const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    vlog(verbosity, file, line, format, vlist);
    va_end(vlist);
}

void vlog(Verbosity verbosity, const char* file, unsigned line,
          const char* format, va_list vlist) {
    auto buff = vtextprintf(format, vlist);
    log_to_everywhere(1, verbosity, file, line, "", buff.c_str());
}

void raw_log(Verbosity verbosity, const char* file, unsigned line,
             const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    auto buff = vtextprintf(format, vlist);
    auto message = Message{verbosity, file, line, "", "", "", buff.c_str()};
    log_message(1, message, false, true);
    va_end(vlist);
}
#endif

void flush() {
    std::lock_guard lock(sMutex);
    fflush(stderr);
    for (const auto& callback : sCallbacks) {
        if (callback.flush != nullptr) {
            callback.flush(callback.userData);
        }
    }
    sNeedsFlushing = false;
}

LogScopeRAII::LogScopeRAII(Verbosity verbosity, const char* file, unsigned line,
                           const char* format, va_list vlist)
    : _verbosity(verbosity), _file(file), _line(line) {
    this->Init(format, vlist);
}

LogScopeRAII::LogScopeRAII(Verbosity verbosity, const char* file, unsigned line,
                           const char* format, ...)
    : _verbosity(verbosity), _file(file), _line(line) {
    va_list vlist;
    va_start(vlist, format);
    this->Init(format, vlist);
    va_end(vlist);
}

LogScopeRAII::~LogScopeRAII() {
    if (_file != nullptr) {
        std::lock_guard lock(sMutex);
        if (_indent_stderr && sStderrIndentation > 0) {
            --sStderrIndentation;
        }
        for (auto& p : sCallbacks) {
            // Note: Callback indentation cannot change!
            if (_verbosity <= p.verbosity) {
                // in unlikely case this callback is new
                if (p.indentation > 0) {
                    --p.indentation;
                }
            }
        }
#if LOGURU_VERBOSE_SCOPE_ENDINGS
        auto duration_sec = static_cast<double>(nowNs() - _start_time_ns) / 1e9;
#if LOGURU_USE_FMTLIB
        auto buff = textprintf("{:.{}f} s: {:s}", duration_sec,
                               LOGURU_SCOPE_TIME_PRECISION, _name);
#else
        auto buff = textprintf("%.*f s: %s", LOGURU_SCOPE_TIME_PRECISION,
                               duration_sec, _name);
#endif
        log_to_everywhere(1, _verbosity, _file, _line, "} ", buff.c_str());
#else
        log_to_everywhere(1, _verbosity, _file, _line, "}", "");
#endif
    }
}

void LogScopeRAII::Init(const char* format, va_list vlist) {
    if (_verbosity <= current_verbosity_cutoff()) {
        std::lock_guard lock(sMutex);
        _indent_stderr = (_verbosity <= g_stderr_verbosity);
        _start_time_ns = nowNs();
        vsnprintf(_name, sizeof(_name), format, vlist);
        log_to_everywhere(1, _verbosity, _file, _line, "{ ", _name);

        if (_indent_stderr) {
            ++sStderrIndentation;
        }

        for (auto& p : sCallbacks) {
            if (_verbosity <= p.verbosity) {
                ++p.indentation;
            }
        }
    } else {
        _file = nullptr;
    }
}

#if LOGURU_USE_FMTLIB
#if __cplusplus >= 202002L
void vlog_and_abort(int stack_trace_skip, const char* expr, const char* file,
                    unsigned line, const char* format, std::format_args args) {
    auto formatted = std::vformat(format, args);
    log_to_everywhere(stack_trace_skip + 1, Verbosity_FATAL, file, line, expr,
                      formatted.c_str());
    std::abort();  // log_to_everywhere already does this, but this makes the
                   // analyzer happy.
}
#else
void vlog_and_abort(int stack_trace_skip, const char* expr, const char* file,
                    unsigned line, const char* format, fmt::format_args args) {
    auto formatted = fmt::vformat(format, args);
    log_to_everywhere(stack_trace_skip + 1, Verbosity_FATAL, file, line, expr,
                      formatted.c_str());
    abort();  // log_to_everywhere already does this, but this makes the
              // analyzer happy.
}
#endif
#else
void log_and_abort(int stack_trace_skip, const char* expr, const char* file,
                   unsigned line, const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    auto buff = vtextprintf(format, vlist);
    log_to_everywhere(stack_trace_skip + 1, Verbosity_FATAL, file, line, expr,
                      buff.c_str());
    va_end(vlist);
    abort();  // log_to_everywhere already does this, but this makes the
              // analyzer happy.
}
#endif

void log_and_abort(int stack_trace_skip, const char* expr, const char* file,
                   unsigned line) {
    log_and_abort(stack_trace_skip + 1, expr, file, line, " ");
}

// ----------------------------------------------------------------------------
// Streams:

#if LOGURU_USE_FMTLIB
template <typename... Args>
std::string vstrprintf(const char* format, const Args&... args) {
    auto text = textprintf(format, args...);
    std::string result = text.c_str();
    return result;
}

template <typename... Args>
std::string strprintf(const char* format, const Args&... args) {
    return vstrprintf(format, args...);
}
#else
std::string vstrprintf(const char* format, va_list vlist) {
    auto text = vtextprintf(format, vlist);
    std::string result = text.c_str();
    return result;
}

std::string strprintf(const char* format, ...) {
    va_list vlist;
    va_start(vlist, format);
    auto result = vstrprintf(format, vlist);
    va_end(vlist);
    return result;
}
#endif

#if LOGURU_WITH_STREAMS

StreamLogger::~StreamLogger() noexcept(false) {
    auto message = _ss.str();
    log(_verbosity, _file, _line, LOGURU_FMT(s), message.c_str());
}

AbortLogger::~AbortLogger() noexcept(false) {
    auto message = _ss.str();
    loguru::log_and_abort(1, _expr, _file, _line, LOGURU_FMT(s),
                          message.c_str());
}

#endif  // LOGURU_WITH_STREAMS

// ----------------------------------------------------------------------------
// 888888 88""Yb 88""Yb  dP"Yb  88""Yb      dP""b8  dP"Yb  88b 88 888888 888888
// Yb  dP 888888 88__   88__dP 88__dP dP   Yb 88__dP     dP   `" dP   Yb 88Yb88
// 88   88__    YbdP    88 88""   88"Yb  88"Yb  Yb   dP 88"Yb      Yb      Yb dP
// 88 Y88   88   88""    dPYb    88 888888 88  Yb 88  Yb  YbodP  88  Yb YboodP
// YbodP  88  Y8   88   888888 dP  Yb   88
// ----------------------------------------------------------------------------

struct StringStream {
    std::string str;
} ATOM_ALIGNAS(32);

// Use this in your EcPrinter implementations.
void stream_print(StringStream& out_string_stream, const char* text) {
    out_string_stream.str += text;
}

// ----------------------------------------------------------------------------

using ECPtr = EcEntryBase*;

#if defined(_WIN32) || (defined(__APPLE__) && !TARGET_OS_IPHONE)
#ifdef __APPLE__
#define LOGURU_THREAD_LOCAL __thread
#else
#define LOGURU_THREAD_LOCAL thread_local
#endif
static LOGURU_THREAD_LOCAL ECPtr thread_ec_ptr = nullptr;

ECPtr& get_thread_ec_head_ref() { return thread_ec_ptr; }
#else   // !thread_local
static pthread_once_t sEcPthreadOnce = PTHREAD_ONCE_INIT;
static pthread_key_t sEcPthreadKey;

void freeEcHeadRef(void* io_error_context) {
    delete reinterpret_cast<ECPtr*>(io_error_context);
}

void ecMakePthreadKey() {
    (void)pthread_key_create(&sEcPthreadKey, freeEcHeadRef);
}

auto getThreadEcHeadRef() -> ECPtr& {
    (void)pthread_once(&sEcPthreadOnce, ecMakePthreadKey);
    auto* ec = reinterpret_cast<ECPtr*>(pthread_getspecific(sEcPthreadKey));
    if (ec == nullptr) {
        ec = new ECPtr(nullptr);
        (void)pthread_setspecific(sEcPthreadKey, ec);
    }
    return *ec;
}
#endif  // !thread_local

// ----------------------------------------------------------------------------

auto get_thread_ec_handle() -> EcHandle { return getThreadEcHeadRef(); }

auto get_error_context() -> Text {
    return get_error_context_for(getThreadEcHeadRef());
}

auto get_error_context_for(const EcEntryBase* ec_head) -> Text {
    std::vector<const EcEntryBase*> stack;
    while (ec_head != nullptr) {
        stack.push_back(ec_head);
        ec_head = ec_head->_previous;
    }
    std::reverse(stack.begin(), stack.end());

    StringStream result;
    if (!stack.empty()) {
        result.str += "------------------------------------------------\n";
        for (const auto* entry : stack) {
            const auto DESCRIPTION = std::string(entry->_descr) + ":";
#if LOGURU_USE_FMTLIB
            auto prefix = textprintf(
                "[ErrorContext] {.{}s}:{:-5u} {:-20s} ", filename(entry->_file),
                LOGURU_FILENAME_WIDTH, entry->_line, DESCRIPTION.c_str());
#else
            auto prefix = textprintf(
                "[ErrorContext] %*s:%-5u %-20s ", LOGURU_FILENAME_WIDTH,
                filename(entry->_file), entry->_line, description.c_str());
#endif
            result.str += prefix.c_str();
            entry->print_value(result);
            result.str += "\n";
        }
        result.str += "------------------------------------------------";
    }
    return Text(STRDUP(result.str.c_str()));
}

EcEntryBase::EcEntryBase(const char* file, unsigned line, const char* descr)
    : _file(file), _line(line), _descr(descr) {
    EcEntryBase*& ecHead = getThreadEcHeadRef();
    _previous = ecHead;
    ecHead = this;
}

EcEntryBase::~EcEntryBase() { getThreadEcHeadRef() = _previous; }

// ------------------------------------------------------------------------

auto ec_to_text(const char* value) -> Text {
    // Add quotes around the string to make it obvious where it begin and ends.
    // This is great for detecting erroneous leading or trailing spaces in e.g.
    // an identifier.
    auto str = "\"" + std::string(value) + "\"";
    return Text{STRDUP(str.c_str())};
}

auto ec_to_text(char c) -> Text {
    // Add quotes around the character to make it obvious where it begin and
    // ends.
    std::string str = "'";

    auto writeHexDigit = [&](unsigned num) {
        if (num < 10u) {
            str += char('0' + num);
        } else {
            str += char('a' + num - 10);
        }
    };

    auto writeHex16 = [&](uint16_t n) {
        writeHexDigit((n >> 12u) & 0x0f);
        writeHexDigit((n >> 8u) & 0x0f);
        writeHexDigit((n >> 4u) & 0x0f);
        writeHexDigit((n >> 0u) & 0x0f);
    };

    if (c == '\\') {
        str += "\\\\";
    } else if (c == '\"') {
        str += "\\\"";
    } else if (c == '\'') {
        str += "\\\'";
    } else if (c == '\0') {
        str += "\\0";
    } else if (c == '\b') {
        str += "\\b";
    } else if (c == '\f') {
        str += "\\f";
    } else if (c == '\n') {
        str += "\\n";
    } else if (c == '\r') {
        str += "\\r";
    } else if (c == '\t') {
        str += "\\t";
    } else if (0 <= c && c < 0x20) {
        str += "\\u";
        writeHex16(static_cast<uint16_t>(c));
    } else {
        str += c;
    }

    str += "'";

    return Text{STRDUP(str.c_str())};
}

#define DEFINE_EC(Type)                   \
    Text ec_to_text(Type value) {         \
        auto str = std::to_string(value); \
        return Text{STRDUP(str.c_str())}; \
    }

DEFINE_EC(int)
DEFINE_EC(unsigned int)
DEFINE_EC(long)
DEFINE_EC(unsigned long)
DEFINE_EC(long long)
DEFINE_EC(unsigned long long)
DEFINE_EC(float)
DEFINE_EC(double)
DEFINE_EC(long double)

#undef DEFINE_EC

auto ec_to_text(EcHandle ec_handle) -> Text {
    Text parentEc = get_error_context_for(ec_handle);
    size_t bufferSize = strlen(parentEc.c_str()) + 2;
    char* withNewline = reinterpret_cast<char*>(malloc(bufferSize));
    withNewline[0] = '\n';
#ifdef _WIN32
    strncpy_s(with_newline + 1, buffer_size, parent_ec.c_str(),
              buffer_size - 2);
#else
    strcpy(withNewline + 1, parentEc.c_str());
#endif
    return Text(withNewline);
}

// ----------------------------------------------------------------------------

}  // namespace loguru

// ----------------------------------------------------------------------------
// .dP"Y8 88  dP""b8 88b 88    db    88     .dP"Y8
// `Ybo." 88 dP   `" 88Yb88   dPYb   88     `Ybo."
// o.`Y8b 88 Yb  "88 88 Y88  dP__Yb  88  .o o.`Y8b
// 8bodP' 88  YboodP 88  Y8 dP""""Yb 88ood8 8bodP'
// ----------------------------------------------------------------------------

#ifdef _WIN32
#include <dbghelp.h>
#include <windows.h>
#include <csignal>

namespace loguru {

LONG WINAPI windowsExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    const char* signalName = "UNKNOWN EXCEPTION";

    switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION:
            signalName = "EXCEPTION_ACCESS_VIOLATION";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            signalName = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
            break;
        case EXCEPTION_BREAKPOINT:
            signalName = "EXCEPTION_BREAKPOINT";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            signalName = "EXCEPTION_DATATYPE_MISALIGNMENT";
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            signalName = "EXCEPTION_FLT_DENORMAL_OPERAND";
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            signalName = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
            break;
        case EXCEPTION_FLT_INEXACT_RESULT:
            signalName = "EXCEPTION_FLT_INEXACT_RESULT";
            break;
        case EXCEPTION_FLT_INVALID_OPERATION:
            signalName = "EXCEPTION_FLT_INVALID_OPERATION";
            break;
        case EXCEPTION_FLT_OVERFLOW:
            signalName = "EXCEPTION_FLT_OVERFLOW";
            break;
        case EXCEPTION_FLT_STACK_CHECK:
            signalName = "EXCEPTION_FLT_STACK_CHECK";
            break;
        case EXCEPTION_FLT_UNDERFLOW:
            signalName = "EXCEPTION_FLT_UNDERFLOW";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            signalName = "EXCEPTION_ILLEGAL_INSTRUCTION";
            break;
        case EXCEPTION_IN_PAGE_ERROR:
            signalName = "EXCEPTION_IN_PAGE_ERROR";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            signalName = "EXCEPTION_INT_DIVIDE_BY_ZERO";
            break;
        case EXCEPTION_INT_OVERFLOW:
            signalName = "EXCEPTION_INT_OVERFLOW";
            break;
        case EXCEPTION_INVALID_DISPOSITION:
            signalName = "EXCEPTION_INVALID_DISPOSITION";
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            signalName = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            signalName = "EXCEPTION_PRIV_INSTRUCTION";
            break;
        case EXCEPTION_SINGLE_STEP:
            signalName = "EXCEPTION_SINGLE_STEP";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            signalName = "EXCEPTION_STACK_OVERFLOW";
            break;
    }

    writeToStderr("\n");
    writeToStderr("Loguru caught an exception: ");
    writeToStderr(signalName);
    writeToStderr("\n");

    if (sSignalOptions.unsafe_signal_handler) {
        flush();
        char preamble_buff[LOGURU_PREAMBLE_WIDTH];
        print_preamble(preamble_buff, sizeof(preamble_buff), Verbosity_FATAL,
                       "", 0);
        auto message = Message{Verbosity_FATAL, "",        0, preamble_buff, "",
                               "Exception: ",   signalName};
        try {
            log_message(1, message, false, false);
        } catch (...) {
            writeToStderr(
                "Exception caught and ignored by Loguru exception handler.\n");
        }
        flush();
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void install_signal_handlers(const SignalOptions& signal_options) {
    sSignalOptions = signal_options;
    SetUnhandledExceptionFilter(windowsExceptionHandler);
}

}  // namespace loguru

#else  // _WIN32

namespace loguru {
void writeToStderr(const char* data, size_t size) {
    auto result = write(STDERR_FILENO, data, size);
    (void)result;  // Ignore errors.
}

void writeToStderr(const char* data) { writeToStderr(data, strlen(data)); }

void callDefaultSignalHandler(int signal_number) {
    struct sigaction sigAction;
    memset(&sigAction, 0, sizeof(sigAction));
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_handler = SIG_DFL;
    sigaction(signal_number, &sigAction, NULL);
    kill(getpid(), signal_number);
}

void signalHandler(int signal_number, siginfo_t*, void*) {
    const char* signalName = "UNKNOWN SIGNAL";

    if (signal_number == SIGABRT) {
        signalName = "SIGABRT";
    }
    if (signal_number == SIGBUS) {
        signalName = "SIGBUS";
    }
    if (signal_number == SIGFPE) {
        signalName = "SIGFPE";
    }
    if (signal_number == SIGILL) {
        signalName = "SIGILL";
    }
    if (signal_number == SIGINT) {
        signalName = "SIGINT";
    }
    if (signal_number == SIGSEGV) {
        signalName = "SIGSEGV";
    }
    if (signal_number == SIGTERM) {
        signalName = "SIGTERM";
    }

    if (g_colorlogtostderr && S_TERMINAL_HAS_COLOR) {
        writeToStderr(terminal_reset());
        writeToStderr(terminal_bold());
        writeToStderr(terminal_light_red());
    }
    writeToStderr("\n");
    writeToStderr("Loguru caught a signal: ");
    writeToStderr(signalName);
    writeToStderr("\n");
    if (g_colorlogtostderr && S_TERMINAL_HAS_COLOR) {
        writeToStderr(terminal_reset());
    }

    if (sSignalOptions.unsafe_signal_handler) {
        flush();
        char preamble_buff[LOGURU_PREAMBLE_WIDTH];
        print_preamble(preamble_buff, sizeof(preamble_buff), Verbosity_FATAL,
                       "", 0);
        auto message = Message{Verbosity_FATAL, "",        0, preamble_buff, "",
                               "Signal: ",      signalName};
        try {
            log_message(1, message, false, false);
        } catch (...) {
            writeToStderr(
                "Exception caught and ignored by Loguru signal handler.\n");
        }
        flush();
    }

    callDefaultSignalHandler(signal_number);
}

void installSignalHandlers(const SignalOptions& signal_options) {
    sSignalOptions = signal_options;

    struct sigaction sig_action;
    memset(&sig_action, 0, sizeof(sig_action));
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags |= SA_SIGINFO;
    sig_action.sa_sigaction = &signalHandler;

    if (signal_options.sigabrt) {
        CHECK_F(sigaction(SIGABRT, &sig_action, NULL) != -1,
                "Failed to install handler for SIGABRT");
    }
    if (signal_options.sigbus) {
        CHECK_F(sigaction(SIGBUS, &sig_action, NULL) != -1,
                "Failed to install handler for SIGBUS");
    }
    if (signal_options.sigfpe) {
        CHECK_F(sigaction(SIGFPE, &sig_action, NULL) != -1,
                "Failed to install handler for SIGFPE");
    }
    if (signal_options.sigill) {
        CHECK_F(sigaction(SIGILL, &sig_action, NULL) != -1,
                "Failed to install handler for SIGILL");
    }
    if (signal_options.sigint) {
        CHECK_F(sigaction(SIGINT, &sig_action, NULL) != -1,
                "Failed to install handler for SIGINT");
    }
    if (signal_options.sigsegv) {
        CHECK_F(sigaction(SIGSEGV, &sig_action, NULL) != -1,
                "Failed to install handler for SIGSEGV");
    }
    if (signal_options.sigterm) {
        CHECK_F(sigaction(SIGTERM, &sig_action, NULL) != -1,
                "Failed to install handler for SIGTERM");
    }
}
}  // namespace loguru

#endif  // _WIN32

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

LOGURU_ANONYMOUS_NAMESPACE_END

#endif  // LOGURU_IMPLEMENTATION
