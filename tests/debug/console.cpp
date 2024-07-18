#include "debug/console.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace lithium::debug {
#ifdef _WIN32
// Mock class for Windows-specific functions
class MockWindowsConsole {
public:
    MOCK_METHOD(HANDLE, GetStdHandle, (DWORD nStdHandle), ());
    MOCK_METHOD(BOOL, GetConsoleScreenBufferInfo,
                (HANDLE hConsoleOutput,
                 PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo),
                ());
    MOCK_METHOD(BOOL, FillConsoleOutputCharacter,
                (HANDLE hConsoleOutput, TCHAR cCharacter, DWORD nLength,
                 COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten),
                ());
    MOCK_METHOD(BOOL, FillConsoleOutputAttribute,
                (HANDLE hConsoleOutput, WORD wAttribute, DWORD nLength,
                 COORD dwWriteCoord, LPDWORD lpNumberOfAttrsWritten),
                ());
    MOCK_METHOD(BOOL, SetConsoleCursorPosition,
                (HANDLE hConsoleOutput, COORD dwCursorPosition), ());
    MOCK_METHOD(BOOL, GetConsoleCursorInfo,
                (HANDLE hConsoleOutput,
                 PCONSOLE_CURSOR_INFO lpConsoleCursorInfo),
                ());
    MOCK_METHOD(BOOL, SetConsoleCursorInfo,
                (HANDLE hConsoleOutput,
                 const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo),
                ());
    MOCK_METHOD(BOOL, SetConsoleTextAttribute,
                (HANDLE hConsoleOutput, WORD wAttributes), ());
};
#endif
}  // namespace lithium::debug

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

TEST(ConsoleTest, ClearScreen_Windows) {
#ifdef _WIN32
    lithium::debug::MockWindowsConsole mockConsole;

    HANDLE mockHandle = reinterpret_cast<HANDLE>(0x1234);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    EXPECT_CALL(mockConsole, GetStdHandle(STD_OUTPUT_HANDLE))
        .WillOnce(Return(mockHandle));
    EXPECT_CALL(mockConsole, GetConsoleScreenBufferInfo(mockHandle, _))
        .WillOnce(DoAll(SetArgPointee<1>(csbi), Return(TRUE)));
    EXPECT_CALL(mockConsole,
                FillConsoleOutputCharacter(mockHandle, ' ', _, _, _))
        .WillOnce(Return(TRUE));
    EXPECT_CALL(mockConsole, FillConsoleOutputAttribute(mockHandle, _, _, _, _))
        .WillOnce(Return(TRUE));
    EXPECT_CALL(mockConsole, SetConsoleCursorPosition(mockHandle, _))
        .WillOnce(Return(TRUE));

    ASSERT_NO_THROW(lithium::debug::clear_screen());
#endif
}

TEST(ConsoleTest, ClearScreen_Unix) {
#ifndef _WIN32
    // Redirect stdout to a buffer
    testing::internal::CaptureStdout();

    lithium::debug::clearScreen();

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "\033[2J\033[1;1H");
#endif
}

TEST(ConsoleTest, SetTextColor_Unix) {
#ifndef _WIN32
    // Redirect stdout to a buffer
    testing::internal::CaptureStdout();

    lithium::debug::setTextColor(lithium::debug::Color::RED);

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "\033[31m");
#endif
}

TEST(ConsoleTest, SetBackgroundColor_Unix) {
#ifndef _WIN32
    // Redirect stdout to a buffer
    testing::internal::CaptureStdout();

    lithium::debug::setBackgroundColor(lithium::debug::Color::Blue);

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "\033[44m");
#endif
}

TEST(ConsoleTest, ResetTextFormat_Unix) {
#ifndef _WIN32
    // Redirect stdout to a buffer
    testing::internal::CaptureStdout();

    lithium::debug::resetTextFormat();

    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "\033[0m");
#endif
}
