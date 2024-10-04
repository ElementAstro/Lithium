#ifndef LITHIUM_DEBUG_CONSOLE_HPP
#define LITHIUM_DEBUG_CONSOLE_HPP

#include <utility>

namespace lithium::debug {

/**
 * @brief Enum class representing text colors.
 */
enum class Color {
    Default,  ///< Default terminal color
    Black,    ///< Black color
    Red,      ///< Red color
    Green,    ///< Green color
    Yellow,   ///< Yellow color
    Blue,     ///< Blue color
    Magenta,  ///< Magenta color
    Cyan,     ///< Cyan color
    White     ///< White color
};

/**
 * @brief Clears the terminal screen.
 */
void clearScreen();

/**
 * @brief Sets the text color in the terminal.
 *
 * @param color The color to set the text to.
 */
void setTextColor(Color color);

/**
 * @brief Sets the background color in the terminal.
 *
 * @param color The color to set the background to.
 */
void setBackgroundColor(Color color);

/**
 * @brief Resets the text format to the terminal's default.
 */
void resetTextFormat();

/**
 * @brief Moves the cursor to the specified position in the terminal.
 *
 * @param row The row to move the cursor to.
 * @param col The column to move the cursor to.
 */
void moveCursor(int row, int col);

/**
 * @brief Hides the cursor in the terminal.
 */
void hideCursor();

/**
 * @brief Shows the cursor in the terminal.
 */
void showCursor();

/**
 * @brief Gets the size of the terminal.
 *
 * @return A pair containing the number of rows and columns in the terminal.
 */
auto getTerminalSize() -> std::pair<int, int>;

/**
 * @brief Checks if the terminal supports color.
 *
 * @return True if the terminal supports color, false otherwise.
 */
auto supportsColor() -> bool;

}  // namespace lithium::debug

#endif
