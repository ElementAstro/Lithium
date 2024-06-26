
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

void clear_screen();
void set_text_color(Color color);
void set_background_color(Color color);
void reset_text_format();
void move_cursor(int row, int col);
void hide_cursor();
void show_cursor();
std::pair<int, int> get_terminal_size();
bool supportsColor();
}  // namespace lithium::debug

#endif
