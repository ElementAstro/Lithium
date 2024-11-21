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
#include "atom/log/loguru.hpp"

namespace lithium::debug {

#ifdef _WIN32

void clearScreen() {
    LOG_F(INFO, "Clearing screen");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD cellCount;
    DWORD count;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    if (GetConsoleScreenBufferInfo(hConsole, &csbi) == 0) {
        LOG_F(ERROR, "Console buffer info error");
        THROW_RUNTIME_ERROR("Console buffer info error");
    }

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords,
                                    &count)) {
        LOG_F(ERROR, "Fill console output error");
        THROW_RUNTIME_ERROR("Fill console output error");
    }

    if (FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount,
                                   homeCoords, &count) == 0) {
        LOG_F(ERROR, "Fill console attribute error");
        THROW_RUNTIME_ERROR("Fill console attribute error");
    }

    SetConsoleCursorPosition(hConsole, homeCoords);
    LOG_F(INFO, "Screen cleared");
}

void setTextColor(Color color) {
    LOG_F(INFO, "Setting text color");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
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
    LOG_F(INFO, "Text color set");
}

void setBackgroundColor(Color color) {
    LOG_F(INFO, "Setting background color");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
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
    LOG_F(INFO, "Background color set");
}

void resetTextFormat() {
    LOG_F(INFO, "Resetting text format");
    setTextColor(Color::Default);
    setBackgroundColor(Color::Default);
    LOG_F(INFO, "Text format reset");
}

void moveCursor(int row, int col) {
    LOG_F(INFO, "Moving cursor to row: {}, col: {}", row, col);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    COORD position = {static_cast<SHORT>(col), static_cast<SHORT>(row)};
    SetConsoleCursorPosition(hConsole, position);
    LOG_F(INFO, "Cursor moved");
}

void hideCursor() {
    LOG_F(INFO, "Hiding cursor");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    LOG_F(INFO, "Cursor hidden");
}

void showCursor() {
    LOG_F(INFO, "Showing cursor");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    LOG_F(INFO, "Cursor shown");
}

auto getTerminalSize() -> std::pair<int, int> {
    LOG_F(INFO, "Getting terminal size");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        THROW_RUNTIME_ERROR("Invalid handle");
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi) == 0) {
        LOG_F(ERROR, "Console buffer info error");
        THROW_RUNTIME_ERROR("Console buffer info error");
    }

    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    LOG_F(INFO, "Terminal size: rows={}, cols={}", rows, cols);
    return {rows, cols};
}

void setConsoleSize(int width, int height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Invalid handle");
        return;
    }

    COORD bufferSize;
    bufferSize.X = width;
    bufferSize.Y = height;
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize)) {
        LOG_F(ERROR, "Unable to set console buffer size");
        return;
    }

    SMALL_RECT windowSize;
    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = width - 1;
    windowSize.Bottom = height - 1;

    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
        LOG_F(ERROR, "Unable to set console window size");
        return;
    }

    LOG_F(INFO, "Console size set to {}x{}.", width, height);
}

#else

void clearScreen() {
    LOG_F(INFO, "Clearing screen");
    std::cout << "\033[2J\033[1;1H";
    LOG_F(INFO, "Screen cleared");
}

void setTextColor(Color color) {
    LOG_F(INFO, "Setting text color");
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
    LOG_F(INFO, "Text color set");
}

void setBackgroundColor(Color color) {
    LOG_F(INFO, "Setting background color");
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
    LOG_F(INFO, "Background color set");
}

void resetTextFormat() {
    LOG_F(INFO, "Resetting text format");
    std::cout << "\033[0m";
    LOG_F(INFO, "Text format reset");
}

void moveCursor(int row, int col) {
    LOG_F(INFO, "Moving cursor to row: {}, col: {}", row, col);
    std::cout << "\033[" << row << ";" << col << "H";
    LOG_F(INFO, "Cursor moved");
}

void hideCursor() {
    LOG_F(INFO, "Hiding cursor");
    std::cout << "\033[?25l";
    LOG_F(INFO, "Cursor hidden");
}

void showCursor() {
    LOG_F(INFO, "Showing cursor");
    std::cout << "\033[?25h";
    LOG_F(INFO, "Cursor shown");
}

auto getTerminalSize() -> std::pair<int, int> {
    LOG_F(INFO, "Getting terminal size");
    struct winsize w {};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    LOG_F(INFO, "Terminal size: rows={}, cols={}", w.ws_row, w.ws_col);
    return {w.ws_row, w.ws_col};
}

void setTerminalSize(int width, int height) {
    LOG_F(INFO, "Setting terminal size to width: {}, height: {}", width,
          height);
    struct winsize ws;

    ws.ws_col = width;
    ws.ws_row = height;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1) {
        LOG_F(ERROR, "Error: Unable to set terminal size.");
        std::cerr << "Error: Unable to set terminal size." << std::endl;
        return;
    }

    LOG_F(INFO, "Terminal size set to {} x {}.", width, height);
}

#endif

auto supportsColor() -> bool {
    LOG_F(INFO, "Checking if terminal supports color");
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        LOG_F(WARNING, "Invalid handle");
        return false;
    }

    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode) == 0) {
        LOG_F(WARNING, "GetConsoleMode failed");
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    bool result = SetConsoleMode(hOut, dwMode) != 0;
    LOG_F(INFO, "Terminal supports color: {}", result ? "true" : "false");
    return result;
#else
    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        LOG_F(WARNING, "TERM environment variable not set");
        return false;
    }

    std::string termStr(term);
    bool result = (termStr == "xterm" || termStr == "xterm-color" ||
                   termStr == "xterm-256color" || termStr == "screen" ||
                   termStr == "screen-256color" || termStr == "tmux" ||
                   termStr == "tmux-256color" || termStr == "rxvt-unicode" ||
                   termStr == "rxvt-unicode-256color" || termStr == "linux" ||
                   termStr == "cygwin") &&
                  (isatty(fileno(stdout)) != 0);
    LOG_F(INFO, "Terminal supports color: {}", result ? "true" : "false");
    return result;
#endif
}

}  // namespace lithium::debug