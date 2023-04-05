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

#include <spdlog/spdlog.h>

namespace OpenAPT {

    // 析构函数，停止所有线程并销毁 ThreadManager 对象
    ThreadManager::~ThreadManager() {
        if(!m_stopFlag)
            joinAllThreads();
    }

    void ThreadManager::addThread(std::function<void()> func, const std::string& name = "") {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this]() { return m_threads.size() < m_maxThreads || m_stopFlag; });
        if (m_stopFlag) {
            spdlog::warn("Thread manager has stopped, cannot add new thread");
            return;
        }
        m_threads.emplace_back(std::make_tuple(std::make_unique<std::thread>([this, func]() {
            try {
                func();
            } catch (const std::exception& e) {
                std::ostringstream ss;
                ss << std::this_thread::get_id();
                spdlog::error("Unhandled exception in thread {}: {}", ss.str(), e.what());
            }
        }), name, false));
        spdlog::info("Added thread: {}", name);
        m_cv.notify_one();
    }

    void ThreadManager::joinAllThreads() {
        try {
            if (m_threads.empty()) {
                return;
            }
            m_stopFlag = true;
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.notify_all();
            for (auto& t : m_threads) {
                if (std::get<0>(t) && std::get<0>(t)->joinable()) {
                    std::get<0>(t)->join();
                    std::get<0>(t).reset(); // 使用 reset() 函数释放智能指针资源
                }
            }
            m_threads.clear();
            spdlog::info("All threads joined");
        } catch (const std::exception& e) {

        }
        
    }


    // 等待指定名称的线程完成，并从 ThreadManager 中移除该线程
    // name：线程名称
    void ThreadManager::joinThreadByName(const std::string& name) {
        if (m_threads.empty()) {
            spdlog::warn("Thread {} not found", name);
            return;
        }
        std::unique_lock<std::mutex> lock(m_mtx);
        for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
            if (std::get<1>(*it) == name) {
                if (std::get<0>(*it) && std::get<0>(*it)->joinable()) {
                    std::get<0>(*it)->join();
                }
                spdlog::info("Thread {} joined", name);
                m_threads.erase(it);
                return;
            }
        }
        spdlog::warn("Thread {} not found", name);
    }

    // 让指定名称的线程休眠指定时间
    // name：线程名称
    // seconds：休眠时间（秒）
    // 返回值：如果找到了该线程，则返回 true；否则返回 false
    bool ThreadManager::sleepThreadByName(const std::string& name, int seconds) {
        if (m_threads.empty()) {
            spdlog::warn("Thread {} not found", name);
            return false;
        }
        std::unique_lock<std::mutex> lock(m_mtx);
        for (auto& t : m_threads) {
            if (std::get<1>(t) == name) {
                if (std::get<2>(t)) {
                    spdlog::warn("Thread {} is already sleeping", name);
                    return true;
                }
                std::get<2>(t) = true;
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::seconds(seconds));
                lock.lock();
                std::get<2>(t) = false;
                return true;
            }
        }
        spdlog::warn("Thread {} not found", name);
        return false;
    }

    // 检查指定名称的线程是否在运行
    // name：线程名称
    // 返回值：如果找到了该线程，则返回 true，表示该线程在运行；否则返回 false，表示该线程未找到
    bool ThreadManager::isThreadRunning(const std::string& name) {
        if (m_threads.empty()) {
            spdlog::warn("Thread {} not found", name);
            return false;
        }
        std::unique_lock<std::mutex> lock(m_mtx);
        for (auto& t : m_threads) {
            if (std::get<1>(t) == name) {
                return !std::get<2>(t);
            }
        }
        spdlog::warn("Thread {} not found", name);
        return false;
    }
}