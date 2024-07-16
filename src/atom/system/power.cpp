#include "power.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#else
#include <cstdlib>
#endif

namespace atom::system {
auto shutdown() -> bool {
#ifdef _WIN32
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
#else
    int ret = std::system("shutdown -h now");
    if (ret == 0 || ret == 1) {
        return true;
    }
#endif
    return true;
}

auto reboot() -> bool {
#ifdef _WIN32
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
#else
    int ret = std::system("reboot");
    if (ret == 0 || ret == 1) {
        return true;
    }
#endif
    return true;
}

auto hibernate() -> bool {
#ifdef _WIN32
    SetSystemPowerState(TRUE, FALSE);
#else
    int ret = std::system("systemctl hibernate");
    if (ret == 0 || ret == 1) {
        return true;
    }
#endif
    return false;
}

auto logout() -> bool {
#ifdef _WIN32
    ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
#else
    int ret = std::system("pkill -KILL -u $(whoami)");
    if (ret == 0 || ret == 1) {
        return true;
    }
#endif
    return false;
}

auto lockScreen() -> bool {
#ifdef _WIN32
    LockWorkStation();
#else
    int ret = std::system("gnome-screensaver-command -l");
    if (ret == 0 || ret == 1) {
        return true;
    }
#endif
    return false;
}
}  // namespace atom::system
