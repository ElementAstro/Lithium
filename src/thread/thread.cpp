/*
 * thread.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: Thread Manager

**************************************************/

#include "thread.hpp"

#include <sstream>

#include <spdlog/spdlog.h>

namespace OpenAPT::Thread
{
    ThreadManager::ThreadManager(int maxThreads) : m_maxThreads(maxThreads) {}

    ThreadManager::~ThreadManager()
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            if (!m_stopFlag)
            {
                m_stopFlag = true;
                m_cv.notify_all();
            }
            joinAllThreads();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to destroy ThreadManager: {}", e.what());
        }
    }

    void ThreadManager::addThread(std::function<void()> func, const std::string &name)
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]()
                      { return m_threads.size() < m_maxThreads || m_stopFlag; });
            if (m_stopFlag)
            {
                throw std::runtime_error("Thread manager has stopped, cannot add new thread");
            }
            auto t = std::make_tuple(std::make_unique<std::thread>([this, func]()
                                                                   {
                                                                       try
                                                                       {
                                                                           func();
                                                                       }
                                                                       catch (const std::exception &e)
                                                                       {
                                                                           std::ostringstream ss;
                                                                           ss << std::this_thread::get_id();
                                                                           spdlog::error("Unhandled exception in thread {}: {}", ss.str(), e.what());
                                                                       }
                                                                       std::unique_lock<std::mutex> lock(m_mtx);
                                                                       // joinThread(lock, t);
                                                                   }),
                                     name, false);

            m_threads.emplace_back(std::move(t));
            spdlog::info("Added thread: {}", name);
            m_cv.notify_one();
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to add thread {}: {}", name, e.what());
        }
    }

    void ThreadManager::joinAllThreads()
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]()
                      { return m_threads.empty(); });
            for (auto &t : m_threads)
            {
                joinThread(lock, t);
            }
            m_threads.clear();
            spdlog::info("All threads joined");
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to join all threads: {}", e.what());
        }
    }

    void ThreadManager::joinThreadByName(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                spdlog::warn("Thread {} not found", name);
                return;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    joinThread(lock, t);
                    spdlog::info("Thread {} joined", name);
                    m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(), [&](auto &x)
                                                   { return !std::get<0>(x); }),
                                    m_threads.end());
                    return;
                }
            }
            spdlog::warn("Thread {} not found", name);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to join thread {}: {}", name, e.what());
        }
    }

    bool ThreadManager::sleepThreadByName(const std::string &name, int seconds)
    {
        try
        {
            if (m_threads.empty())
            {
                spdlog::warn("Thread {} not found", name);
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    if (std::get<2>(t))
                    {
                        spdlog::warn("Thread {} is already sleeping", name);
                        return true;
                    }
                    std::get<2>(t) = true;
                    m_cv.notify_all();
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::seconds(seconds));
                    lock.lock();
                    std::get<2>(t) = false;
                    m_cv.notify_all();
                    return true;
                }
            }
            spdlog::warn("Thread {} not found", name);
            return false;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to sleep thread {}: {}", name, e.what());
            return false;
        }
    }

    bool ThreadManager::isThreadRunning(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                spdlog::warn("Thread {} not found", name);
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    return !std::get<2>(t);
                }
            }
            spdlog::warn("Thread {} not found", name);
            return false;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to check if thread {} is running: {}", name, e.what());
            return false;
        }
    }

    void ThreadManager::joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::thread>, std::string, bool> &t)
    {
        if (std::get<0>(t) && std::get<0>(t)->joinable())
        {
            std::get<0>(t)->join();
            std::get<0>(t).reset();
        }
        std::get<2>(t) = true;
        m_cv.notify_all();
        lock.unlock();
        std::get<0>(t).reset();
        lock.lock();
        std::get<2>(t) = false;
        m_cv.notify_all();
    }

}