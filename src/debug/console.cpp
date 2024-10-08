#include "console.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include "atom/error/exception.hpp"

namespace lithium::debug {

#ifdef _WIN32

void clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD cellCount;
    DWORD count;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    if (GetConsoleScreenBufferInfo(hConsole, &csbi) == 0) {
        THROW_RUNTIME_ERROR("Console buffer info error");
    }

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords,
                                    &count)) {
        THROW_RUNTIME_ERROR("Fill console output error");
}

    if (FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount,
                                    homeCoords, &count) == 0) {
        THROW_RUNTIME_ERROR("Fill console attribute error");
}

    SetConsoleCursorPosition(hConsole, homeCoords);
}

void setTextColor(Color color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    WORD attributes = 0;

    switch (color) {
        case Color::Black:
            attributes = 0;
            break;
        case Color::Red:
            attributes = FOREGROUND_RED;
            break;
        case Color::Green:
            attributes = FOREGROUND_GREEN;
            break;
        case Color::Yellow:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN;
            break;
        case Color::Blue:
            attributes = FOREGROUND_BLUE;
            break;
        case Color::Magenta:
            attributes = FOREGROUND_RED | FOREGROUND_BLUE;
            break;
        case Color::Cyan:
            attributes = FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case Color::White:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case Color::Default:
        default:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
    }

    SetConsoleTextAttribute(hConsole, attributes);
}

void setBackgroundColor(Color color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    WORD attributes = 0;

    switch (color) {
        case Color::Black:
            attributes = 0;
            break;
        case Color::Red:
            attributes = BACKGROUND_RED;
            break;
        case Color::Green:
            attributes = BACKGROUND_GREEN;
            break;
        case Color::Yellow:
            attributes = BACKGROUND_RED | BACKGROUND_GREEN;
            break;
        case Color::Blue:
            attributes = BACKGROUND_BLUE;
            break;
        case Color::Magenta:
            attributes = BACKGROUND_RED | BACKGROUND_BLUE;
            break;
        case Color::Cyan:
            attributes = BACKGROUND_GREEN | BACKGROUND_BLUE;
            break;
        case Color::White:
            attributes = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
            break;
        case Color::Default:
        default:
            attributes = 0;
            break;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    SetConsoleTextAttribute(hConsole, (csbi.wAttributes & 0x0F) | attributes);
}

void resetTextFormat() {
    setTextColor(Color::Default);
    setBackgroundColor(Color::Default);
}

void moveCursor(int row, int col) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    COORD position = {static_cast<SHORT>(col), static_cast<SHORT>(row)};
    SetConsoleCursorPosition(hConsole, position);
}

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void showCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

auto getTerminalSize() -> std::pair<int, int> {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        THROW_RUNTIME_ERROR("Invalid handle");
}

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi) == 0) {
        THROW_RUNTIME_ERROR("Console buffer info error");
}

    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return {rows, cols};
}

#else

void clearScreen() { std::cout << "\033[2J\033[1;1H"; }

void setTextColor(Color color) {
    switch (color) {
        case Color::Black:
            std::cout << "\033[30m";
            break;
        case Color::Red:
            std::cout << "\033[31m";
            break;
        case Color::Green:
            std::cout << "\033[32m";
            break;
        case Color::Yellow:
            std::cout << "\033[33m";
            break;
        case Color::Blue:
            std::cout << "\033[34m";
            break;
        case Color::Magenta:
            std::cout << "\033[35m";
            break;
        case Color::Cyan:
            std::cout << "\033[36m";
            break;
        case Color::White:
            std::cout << "\033[37m";
            break;
        case Color::Default:
        default:
            std::cout << "\033[39m";
            break;
    }
}

void setBackgroundColor(Color color) {
    switch (color) {
        case Color::Black:
            std::cout << "\033[40m";
            break;
        case Color::Red:
            std::cout << "\033[41m";
            break;
        case Color::Green:
            std::cout << "\033[42m";
            break;
        case Color::Yellow:
            std::cout << "\033[43m";
            break;
        case Color::Blue:
            std::cout << "\033[44m";
            break;
        case Color::Magenta:
            std::cout << "\033[45m";
            break;
        case Color::Cyan:
            std::cout << "\033[46m";
            break;
        case Color::White:
            std::cout << "\033[47m";
            break;
        case Color::Default:
        default:
            std::cout << "\033[49m";
            break;
    }
}

void resetTextFormat() { std::cout << "\033[0m"; }

void moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void hideCursor() { std::cout << "\033[?25l"; }

void showCursor() { std::cout << "\033[?25h"; }

auto getTerminalSize() -> std::pair<int, int> {
    struct winsize w {};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return {w.ws_row, w.ws_col};
}

#endif

auto supportsColor() -> bool {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode) == 0) {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(hOut, dwMode) != 0;
#else
    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        return false;
    }

    std::string termStr(term);
    return (termStr == "xterm" || termStr == "xterm-color" ||
            termStr == "xterm-256color" || termStr == "screen" ||
            termStr == "screen-256color" || termStr == "tmux" ||
            termStr == "tmux-256color" || termStr == "rxvt-unicode" ||
            termStr == "rxvt-unicode-256color" || termStr == "linux" ||
            termStr == "cygwin") &&
           (isatty(fileno(stdout)) != 0);
#endif
}

}  // namespace lithium::debug
