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

    void ThreadManager::addThread(std::function<void()> func, const std::string& name) {
        std::unique_lock<std::mutex> lock(m_mtx); // 使用 unique_lock 方便后续的等待和唤醒操作
        m_cv.wait(lock, [this] { return m_threads.size() < m_maxThreads || m_stopFlag; }); // 等待直到可以新建线程或者线程管理器被停止
        if (m_stopFlag) { // 如果线程管理器已经被停止，直接返回
            spdlog::warn("Thread manager has stopped, cannot add new thread");
            return;
        }
        m_threads.emplace_back(std::make_unique<std::thread>([this, func]() {
            try {
                func();
            } catch (...) {
                spdlog::error("Unhandled exception in thread");
            }
        }));
        m_threadNames.emplace_back(name);
        m_sleepFlags.push_back(false);
        m_stopFlag = false;
        spdlog::info("Added thread: {}", name);
        m_cv.notify_one(); // 唤醒等待的线程
    }

    void ThreadManager::joinAllThreads() {
        if (m_threads.empty()) {
            return;
        }
        m_stopFlag = true;
        std::lock_guard<std::mutex> lock(m_mtx);
        size_t i = 0;
        while (i < m_threads.size()) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                ++i;
                continue;
            }
            if (auto* t = m_threads[i].get(); t) {
                t->join();
                m_threads.erase(m_threads.begin() + i);
                m_threadNames.erase(m_threadNames.begin() + i);
                m_sleepFlags.erase(m_sleepFlags.begin() + i);
                spdlog::info("Thread {} joined", m_threadNames[i]);
            } else {
                spdlog::warn("Thread {} has already joined", m_threadNames[i]);
                ++i;
            }
        }
        m_threads.clear();
        m_threadNames.clear();
        m_sleepFlags.clear();
        spdlog::info("All threads joined");
    }

    // 等待指定名称的线程完成，并从 ThreadManager 中移除该线程
    // name：线程名称
    void ThreadManager::joinThreadByName(const std::string& name) {
        if (m_threads.empty()) {
            spdlog::warn("Thread {} not found", name);
            return;
        }
        std::lock_guard<std::mutex> lock(m_mtx);
        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                continue;
            }
            if (m_threadNames[i] == name) {
                if (auto& t = m_threads[i]; t) {
                    t->join();
                    m_threads.erase(m_threads.begin() + i);
                    m_threadNames.erase(m_threadNames.begin() + i);
                    m_sleepFlags.erase(m_sleepFlags.begin() + i);
                    spdlog::info("Thread {} joined", name);
                } else {
                    spdlog::warn("Thread {} has already joined", name);
                }
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
        std::lock_guard<std::mutex> lock(m_mtx);
        size_t i = 0;
        while (i < m_threads.size()) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                ++i;
                continue;
            }
            if (m_threadNames[i] == name) {
                m_sleepFlags[i] = true; // 设置线程为睡眠状态
                std::this_thread::sleep_for(std::chrono::seconds(seconds)); // 线程休眠
                m_sleepFlags[i] = false; // 设置线程为非睡眠状态
                return true;
            } else {
                ++i;
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
        std::lock_guard<std::mutex> lock(m_mtx);
        size_t i = 0;
        while (i < m_threads.size() && i < m_threadNames.size() && i < m_sleepFlags.size()) {
            if (m_threadNames[i] == name) {
                return !m_sleepFlags[i]; // 如果线程不在睡眠状态，则表示线程在运行
            } else {
                ++i;
            }
        }
        spdlog::warn("Thread {} not found", name);
        return false;
    }
}