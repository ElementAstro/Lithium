#include "signal.hpp"
#include <csignal>

std::map<int, SignalHandler::SignalHandlerFunc> SignalHandler::handlers;

void SignalHandler::handleSignal(int signal)
{
    auto it = handlers.find(signal);
    if (it != handlers.end())
    {
        it->second();
    }
}

void SignalHandler::registerHandler(int signal, const SignalHandlerFunc &handlerFunc)
{
#ifdef _WIN32
    if (signal == SIGINT || signal == SIGTERM)
    {
        SetConsoleCtrlHandler(handleConsoleEvent, TRUE);
    }
    else
    {
        handlers[signal] = handlerFunc;
    }
#else
    handlers[signal] = handlerFunc;
    std::signal(signal, handleSignal);
#endif
}

void SignalHandler::unregisterHandler(int signal)
{
#ifdef _WIN32
    if (signal == SIGINT || signal == SIGTERM)
    {
        SetConsoleCtrlHandler(handleConsoleEvent, FALSE);
    }
    else
    {
        handlers.erase(signal);
    }
#else
    handlers.erase(signal);
    std::signal(signal, SIG_DFL);
#endif
}

#ifdef _WIN32
BOOL WINAPI SignalHandler::handleConsoleEvent(DWORD eventType)
{
    switch (eventType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        handleSignal(SIGINT);
        return TRUE;
    default:
        return FALSE;
    }
}
#endif