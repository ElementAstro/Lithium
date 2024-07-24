#include "nodebugger.hpp"

#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <fstream>
#include <string>
#endif

namespace atom::system {
#ifdef _WIN32
bool isDebuggerAttached() { return IsDebuggerPresent(); }
#elif __linux__
auto isDebuggerAttached() -> bool {
    std::ifstream status("/proc/self/status");
    if (!status.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(status, line)) {
        if (line.find("TracerPid:") == 0) {
            int tracerPid;
            sscanf(line.c_str(), "TracerPid:\t%d", &tracerPid);
            return tracerPid != 0;
        }
    }
    return false;
}
#else
#error "Unsupported platform"
#endif

void checkDebuggerAndExit() {
    if (isDebuggerAttached()) {
        std::exit(EXIT_FAILURE);
    }
}

}  // namespace atom::system
