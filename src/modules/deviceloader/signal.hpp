#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <functional>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

class SignalHandler
{
public:
    using SignalHandlerFunc = std::function<void()>;

    static void handleSignal(int signal);
    static void registerHandler(int signal, const SignalHandlerFunc &handlerFunc);
    static void unregisterHandler(int signal);

private:
    static std::map<int, SignalHandlerFunc> handlers;

#ifdef _WIN32
    static BOOL WINAPI handleConsoleEvent(DWORD eventType);
#endif
};

#endif // SIGNALHANDLER_H
