/*
 * stopwatcher.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-25

Description: Simple implementation of a stopwatch

**************************************************/

#include "stopwatcher.hpp"

#include <iostream>
#include <thread>
#include <iomanip>
#include <sstream>

namespace Atom::Utils
{
    StopWatcher::StopWatcher() : m_running(false), m_paused(false) {}

    void StopWatcher::start()
    {
        if (!m_running)
        {
            m_start = std::chrono::high_resolution_clock::now();
            m_running = true;
            m_paused = false;
            m_intervals.push_back(m_start);
        }
    }

    void StopWatcher::stop()
    {
        if (m_running && !m_paused)
        {
            auto stopTime = std::chrono::high_resolution_clock::now();
            m_end = stopTime;
            m_running = false;
            m_intervals.push_back(stopTime);
            checkCallbacks(stopTime);
        }
    }

    void StopWatcher::pause()
    {
        if (m_running && !m_paused)
        {
            m_pauseTime = std::chrono::high_resolution_clock::now();
            m_paused = true;
            m_intervals.push_back(m_pauseTime);
        }
    }

    void StopWatcher::resume()
    {
        if (m_running && m_paused)
        {
            auto resumeTime = std::chrono::high_resolution_clock::now();
            m_start += resumeTime - m_pauseTime; // Adjust start time by pause duration
            m_paused = false;
            m_intervals.push_back(resumeTime);
        }
    }

    void StopWatcher::reset()
    {
        m_running = false;
        m_paused = false;
        m_intervals.clear();
    }

    double StopWatcher::elapsedMilliseconds() const
    {
        auto endTime = m_running ? (m_paused ? m_pauseTime : std::chrono::high_resolution_clock::now()) : m_end;
        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_start).count();
    }

    double StopWatcher::elapsedSeconds() const
    {
        return elapsedMilliseconds() / 1000.0;
    }

    std::string StopWatcher::elapsedFormatted() const
    {
        auto totalSeconds = static_cast<int>(elapsedSeconds());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;

        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << hours << ":"
           << std::setw(2) << std::setfill('0') << minutes << ":"
           << std::setw(2) << std::setfill('0') << seconds;

        return ss.str();
    }

    void StopWatcher::registerCallback(std::function<void()> callback, int milliseconds)
    {
        m_callbacks.push_back({callback, milliseconds});
    }

    void StopWatcher::checkCallbacks(const std::chrono::time_point<std::chrono::high_resolution_clock> &currentTime)
    {
        for (auto &callbackPair : m_callbacks)
        {
            auto targetTime = m_start + std::chrono::milliseconds(callbackPair.second);
            if (currentTime >= targetTime)
            {
                callbackPair.first();
            }
        }
    }
}
