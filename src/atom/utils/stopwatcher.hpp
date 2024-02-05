/*
 * stopwatcher.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-25

Description: Simple implementation of a stopwatch

**************************************************/

#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <functional>

namespace Atom::Utils
{
    class StopWatcher
    {
    public:

        StopWatcher();

        void start();

        void stop();

        void pause();

        void resume();

        void reset();

        double elapsedMilliseconds() const;

        double elapsedSeconds() const;

        std::string elapsedFormatted() const;

        void registerCallback(std::function<void()> callback, int milliseconds);

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end, m_pauseTime;
        std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> m_intervals;
        bool m_running, m_paused;
        std::vector<std::pair<std::function<void()>, int>> m_callbacks;

        void checkCallbacks(const std::chrono::time_point<std::chrono::high_resolution_clock> &currentTime);
    };
}