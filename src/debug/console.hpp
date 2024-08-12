
#ifndef LITHIUM_DEBUG_CONSOLE_HPP
#define LITHIUM_DEBUG_CONSOLE_HPP

#include <utility>

namespace lithium::debug {
enum class Color {
    Default,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

void clearScreen();
void setTextColor(Color color);
void setBackgroundColor(Color color);
void resetTextFormat();
void moveCursor(int row, int col);
void hideCursor();
void showCursor();
auto getTerminalSize() -> std::pair<int, int>;
auto supportsColor() -> bool;
}  // namespace lithium::debug

#endif
