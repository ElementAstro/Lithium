#include "progress.hpp"

#include <iostream>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#define CLEAR_SCREEN "cls"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#else
#define CLEAR_SCREEN "clear"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#endif

namespace lithium::debug {

constexpr float PERCENTAGE_MULTIPLIER = 100.0f;
constexpr int MILLISECONDS_IN_A_SECOND = 1000;
constexpr int SECONDS_IN_A_MINUTE = 60;
constexpr int MILLISECONDS_IN_A_MINUTE =
    MILLISECONDS_IN_A_SECOND * SECONDS_IN_A_MINUTE;
constexpr int SLEEP_DURATION_MS = 100;

auto getColorCode(Color color) -> std::string {
    switch (color) {
        case Color::Red:
            return "\033[31m";
        case Color::Green:
            return "\033[32m";
        case Color::Yellow:
            return "\033[33m";
        case Color::Blue:
            return "\033[34m";
        default:
            return "\033[0m";
    }
}

void ProgressBar::printProgressBar() {
    float progress = static_cast<float>(current) / static_cast<float>(total);
    int pos = static_cast<int>(progress * static_cast<float>(width));

    // Clear screen in a thread-safe manner
    std::cout << "\033[2J\033[1;1H";

    std::cout << getColorCode(color);
    std::cout << "[";
#pragma unroll
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            std::cout << completeChar;
        } else if (i == pos) {
            std::cout << ">";
        } else {
            std::cout << incompleteChar;
        }
    }
    std::cout << "] ";

    if (showTimeLeft && current > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - start_time)
                           .count();
        int remaining = static_cast<int>((elapsed * total) / current - elapsed);
        std::cout << static_cast<int>(progress * PERCENTAGE_MULTIPLIER)
                  << " % (ETA: " << remaining / MILLISECONDS_IN_A_MINUTE << "m "
                  << (remaining / MILLISECONDS_IN_A_SECOND) %
                         SECONDS_IN_A_MINUTE
                  << "s)";
    } else {
        std::cout << static_cast<int>(progress * PERCENTAGE_MULTIPLIER) << " %";
    }

    std::cout << "\033[0m" << std::endl;  // Reset color
}

ProgressBar::ProgressBar(int total, int width, char completeChar,
                         char incompleteChar, bool showTimeLeft, Color color)
    : total(total),
      width(width),
      completeChar(completeChar),
      incompleteChar(incompleteChar),
      showTimeLeft(showTimeLeft),
      color(color),
      current(0),
      running(false),
      paused(false) {}

void ProgressBar::start() {
    running = true;
    paused = false;
    start_time = std::chrono::steady_clock::now();

    std::cout << HIDE_CURSOR;  // Hide cursor

    future = std::async(std::launch::async, [this]() {
#pragma unroll
        while (running) {
            std::unique_lock lock(mutex);
            cv.wait(lock, [this]() { return !paused || !running; });

            if (!running) {
                break;
            }

            printProgressBar();

            if (++current > total) {
                running = false;
            } else {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(SLEEP_DURATION_MS));
            }
        }

        std::cout << SHOW_CURSOR;  // Show cursor when done
    });
}

void ProgressBar::pause() {
    std::lock_guard lock(mutex);
    paused = true;
}

void ProgressBar::resume() {
    std::lock_guard lock(mutex);
    paused = false;
    cv.notify_one();
}

void ProgressBar::stop() {
    std::lock_guard lock(mutex);
    running = false;
    cv.notify_one();
}

void ProgressBar::reset() {
    std::lock_guard lock(mutex);
    current = 0;
    start_time = std::chrono::steady_clock::now();
    paused = false;
    running = true;
    cv.notify_one();
}

void ProgressBar::wait() {
    if (future.valid()) {
        future.wait();
    }
}

auto ProgressBar::getCurrent() const -> int { return current; }

}  // namespace lithium::debug
