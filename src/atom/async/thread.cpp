/*
 * thread.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-3-29

Description: Thread Manager

**************************************************/

#include "thread.hpp"
#include "config.h"

#include <sstream>
#include <iostream>
#include <random>

#include "atom/log/loguru.hpp"
#include "atom/utils/random.hpp"

namespace Atom::Async
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
            LOG_F(ERROR, _("Failed to destroy ThreadManager: {}"), e.what());
        }
    }

    std::shared_ptr<ThreadManager> ThreadManager::createShared(int maxThreads)
    {
        return std::make_shared<ThreadManager>(maxThreads);
    }

    void ThreadManager::addThread(std::function<void()> func, const std::string &name)
    {
        LOG_SCOPE_FUNCTION(MAX);
        if (m_stopFlag)
        {
            throw std::runtime_error(_("Thread manager has stopped, cannot add new thread"));
        }
        try
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::addThread: trying to add thread {}"), name);
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]
                      { return static_cast<int>(m_threads.size()) < m_maxThreads || m_stopFlag; });
            if (name != "")
            {
                auto t = std::make_tuple(
#if __cplusplus >= 202002L
                std::make_unique<std::jthread>([func]
                {
#else
                std::make_unique<std::thread>([func]
                {
#endif
                try
                {
                    func();
                }
                catch (const std::exception &e)
                {
                    LOG_F(ERROR, _("Unhandled exception in thread: {}"), e.what());
                } }),
                    name,
                    false);
                m_threads.emplace_back(std::move(t));
                VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::addThread: added thread {}"), name);
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
                    LOG_F(ERROR, _("Unhandled exception in thread: {}"), e.what());
                } }),
                    Atom::Utils::generateRandomString(16),
                    false);
                m_threads.emplace_back(std::move(t));
                VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::addThread: added thread {}"), std::get<2>(t));
            }
            m_cv.notify_all();
        }
        catch (const std::exception &e)
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::addThread: failed to add thread {}: {}"), name, e.what());
        }
    }

    void ThreadManager::joinAllThreads()
    {
        LOG_SCOPE_FUNCTION(MAX);
        try
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinAllThreads: trying to join all threads"));
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]
                      { return m_threads.empty(); });
            for (auto &t : m_threads)
            {
                VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinAllThreads: trying to join thread {}"), std::get<2>(t));
                joinThread(lock, t);
            }
            m_threads.clear();
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinAllThreads: all threads joined"));
        }
        catch (const std::exception &e)
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinAllThreads: failed to join all threads: {}"), e.what());
        }
    }

    void ThreadManager::joinThreadByName(const std::string &name)
    {
        LOG_SCOPE_FUNCTION(MAX);
        try
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinThreadByName: trying to join thread {}"), name);
            if (m_threads.empty())
            {
                VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinThreadByName: no threads to join"));
                DLOG_F(WARNING, _("Thread {} not found"), name);
                return;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    DLOG_F(INFO, _("Thread {} found"), name);
                    joinThread(lock, t);
                    DLOG_F(INFO, _("Thread {} joined"), name);
                    m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(),
                                                   [&](auto &x)
                                                   { return !std::get<0>(x); }),
                                    m_threads.end());
                    return;
                }
            }
            VLOG_SCOPE_F(9 , _("Atom::Async::ThreadManager::joinThreadByName: thread {} not found"), name);
            DLOG_F(WARNING, _("Thread {} not found"), name);
        }
        catch (const std::exception &e)
        {
            VLOG_SCOPE_F(9, _("Atom::Async::ThreadManager::joinThreadByName: failed to join thread {}: {}"), name, e.what());
            LOG_F(ERROR, _("Failed to join thread {}: {}"), name, e.what());
        }
    }

    bool ThreadManager::isThreadRunning(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                DLOG_F(WARNING, _("Thread {} not found"), name);
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
            DLOG_F(WARNING, _("Thread {} not found"), name);
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, _("Failed to check if thread {} is running: {}"), name, e.what());
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

    void ThreadManager::setMaxThreads(int maxThreads)
    {
        if (maxThreads <= 0)
        {
            m_maxThreads = std::thread::hardware_concurrency();
        }
        else
        {
            m_maxThreads = maxThreads;
        }
    }

    int ThreadManager::getMaxThreads() const
    {
        return m_maxThreads;
    }
}