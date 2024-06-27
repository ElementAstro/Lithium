#include "power.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#else
#include <cstdlib>
#endif

namespace atom::system {
bool shutdown() {
#ifdef _WIN32
    ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
#else
    int ret = std::system("shutdown -h now");
    if (ret == 0 || ret == 1 /* success */) {
        return true;
    }
#endif
    return true;
}

// 重启函数
bool reboot() {
#ifdef _WIN32
    ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
#else
    int ret = std::system("reboot");
    if (ret == 0 || ret == 1 /* success */) {
        return true;
    }
#endif
    return true;
}

bool hibernate() {
#ifdef _WIN32
    // Windows: 设置系统进入休眠状态
    SetSystemPowerState(TRUE, FALSE);
#else
    // Linux: 使用 pm-utils 或 systemd 来休眠系统
    int ret = std::system("systemctl hibernate");
    if (ret == 0 || ret == 1 /* success */) {
        return true;
    }
#endif
    return false;
}

bool logout() {
#ifdef _WIN32
    ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, 0);
#else
    int ret = std::system("pkill -KILL -u $(whoami)");
    if (ret == 0 || ret == 1 /* success */) {
        return true;
    }
#endif
    return false;
}

bool lockScreen() {
#ifdef _WIN32
    LockWorkStation();
#else
    int ret = std::system("gnome-screensaver-command -l");
    if (ret == 0 || ret == 1 /* success */) {
        return true;
    }
#endif
    return false;
}
}  // namespace atom::system
