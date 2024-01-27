/*
 * thread.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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
            LOG_F(ERROR, "Failed to destroy ThreadManager: {}", e.what());
        }
    }

    std::shared_ptr<ThreadManager> ThreadManager::createShared(int maxThreads)
    {
        return std::make_shared<ThreadManager>(maxThreads);
    }

    void ThreadManager::addThread(std::function<void()> func, const std::string &name)
    {
        if (m_stopFlag)
        {
            throw std::runtime_error("Thread manager has stopped, cannot add new thread");
        }
        try
        {
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
                    LOG_F(ERROR, "Unhandled exception in thread: {}", e.what());
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
                    LOG_F(ERROR, "Unhandled exception in thread: {}", e.what());
                } }),
                    Atom::Utils::generateRandomString(16),
                    false);
                m_threads.emplace_back(std::move(t));
            }
            m_cv.notify_all();
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to add thread: {}", e.what());
        }
    }
    
    bool ThreadManager::joinAllThreads()
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
        }
        catch (const std::exception &e)
        {
            return false;
        }
        return true;
    }

    bool ThreadManager::joinThreadByName(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
                DLOG_F(WARNING, "Thread {} not found", name);
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mtx);
            for (auto &t : m_threads)
            {
                if (std::get<1>(t) == name)
                {
                    DLOG_F(INFO, "Thread {} found", name);
                    joinThread(lock, t);
                    DLOG_F(INFO, "Thread {} joined", name);
                    m_threads.erase(std::remove_if(m_threads.begin(), m_threads.end(),
                                                   [&](auto &x)
                                                   { return !std::get<0>(x); }),
                                    m_threads.end());
                    return true;
                }
            }
            DLOG_F(WARNING, "Thread {} not found", name);
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to join thread {}: {}", name, e.what());
        }
        return false;
    }

    bool ThreadManager::isThreadRunning(const std::string &name)
    {
        try
        {
            if (m_threads.empty())
            {
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
            return false;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to check if thread {} is running: {}", name, e.what());
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