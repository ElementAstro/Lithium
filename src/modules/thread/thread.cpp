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
#include <iostream>
#include <random>

#include "loguru/loguru.hpp"

namespace Lithium::Thread
{
    ThreadManager::ThreadManager(int maxThreads)
        : m_maxThreads(maxThreads), m_stopFlag(false)
    {
    }

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
            lock.unlock();
            joinAllThreads();
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to destroy ThreadManager: %s", e.what());
        }
    }

    void ThreadManager::addThread(std::function<void()> func, const std::string &name)
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]
                      { return m_threads.size() < m_maxThreads || m_stopFlag; });

            if (m_stopFlag)
            {
                throw std::runtime_error("Thread manager has stopped, cannot add new thread");
            }
            if (name != "")
            {
                auto t = std::make_tuple(
#if __cplusplus >= 202002L
                    std::make_unique<std::jthread>([func]
#else
                    std::make_unique<std::thread>([func]
#endif

                                                   {
                try
                {
                    func();
                }
                catch (const std::exception &e)
                {
                    LOG_F(ERROR, "Unhandled exception in thread: %s", e.what());
                } }),
                    name,
                    false);
                m_threads.emplace_back(std::move(t));
            }
            else
            {
                auto t = std::make_tuple(
#if __cplusplus >= 202002L
                    std::make_unique<std::jthread>([func]
#else
                    std::make_unique<std::thread>([func]
#endif

                                                   {
                try
                {
                    func();
                }
                catch (const std::exception &e)
                {
                    LOG_F(ERROR, "Unhandled exception in thread: %s", e.what());
                } }),
                    generateRandomString(16),
                    false);
                m_threads.emplace_back(std::move(t));
            }
            LOG_F(INFO, "Added thread: %s", name.c_str());
            m_cv.notify_all();
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to add thread %s: %s", name.c_str(), e.what());
        }
    }

    void ThreadManager::joinAllThreads()
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]
                      { return m_threads.empty(); });
            for (auto &t : m_threads)
            {
                joinThread(lock, t);
            }
            m_threads.clear();
            LOG_F(INFO, "All threads joined");
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to join all threads: %s", e.what());
        }
    }

    void ThreadManager::joinThreadByName(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                LOG_F(WARNING, "Thread %s not found", name.c_str());
                return;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    joinThread(lock, t);
                    LOG_F(INFO, "Thread %s joined", name.c_str());
                    m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(),
                                                   [&](auto &x)
                                                   { return !std::get<0>(x); }),
                                    m_threads.end());
                    return;
                }
            }
            LOG_F(WARNING, "Thread %s not found", name.c_str());
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to join thread %s: %s", name.c_str(), e.what());
        }
    }

    bool ThreadManager::sleepThreadByName(const std::string &name, int seconds)
    {
        try
        {
            if (m_threads.empty())
            {
                LOG_F(WARNING, "Thread %s not found", name.c_str());
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    if (std::get<2>(t))
                    {
                        LOG_F(WARNING, "Thread %s is already sleeping", name.c_str());
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
            LOG_F(WARNING, "Thread %s not found", name.c_str());
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to sleep thread %s: %s", name.c_str(), e.what());
            return false;
        }
    }

    bool ThreadManager::isThreadRunning(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                LOG_F(WARNING, "Thread %s not found", name.c_str());
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
            LOG_F(WARNING, "Thread %s not found", name.c_str());
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to check if thread %s is running: %s", name.c_str(), e.what());
            return false;
        }
    }

#if __cplusplus >= 202002L
    void ThreadManager::joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::jthread>, std::string, bool> &t)
#else
    void ThreadManager::joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::thread>, std::string, bool> &t)
#endif
    {
        auto &threadPtr = std::get<0>(t);
        if (threadPtr && threadPtr->joinable())
        {
            threadPtr->join();
#if __cplusplus >= 202002L
            threadPtr->request_stop();
#endif
            threadPtr.reset();
        }
        std::get<2>(t) = true;
        m_cv.notify_all();
        lock.unlock();
        threadPtr.reset();
        lock.lock();
        std::get<2>(t) = false;
        m_cv.notify_all();
    }

    const std::string ThreadManager::generateRandomString(int length)
    {
        static const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static std::random_device rd;
        static std::mt19937 generator(rd());
        static std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

        std::string randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i)
        {
            randomString.push_back(characters[distribution(generator)]);
        }

        return randomString;
    }
}