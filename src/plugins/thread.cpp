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

    // 添加一个新线程，并指定线程名称
    // func：要执行的函数
    // name：线程名称
    void ThreadManager::addThread(std::function<void()> func, const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mtx); // 线程锁
        m_threads.emplace_back(std::make_unique<std::thread>([this, func]() { // 添加线程
            try {
                func(); // 执行函数
            } catch (...) {
                spdlog::error("Unhandled exception in thread"); // 异常处理
            }
        }));
        m_threadNames.emplace_back(name); // 添加线程名
        m_sleepFlags.push_back(false); // 初始化睡眠标志
        m_stopFlag = false;
        spdlog::info("Added thread: {}", name); // 日志输出
    }

    // 等待所有线程完成并销毁 ThreadManager 对象
    void ThreadManager::joinAllThreads() {
        m_stopFlag = true; // 设置停止标志
        if (m_threads.empty()) { // 如果 m_threads 为空
            return;
        }
        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                continue;
            }
            if (auto& t = m_threads[i]; t) {
                t->join();
                m_threads.erase(m_threads.begin() + i);
                m_threadNames.erase(m_threadNames.begin() + i);
                m_sleepFlags.erase(m_sleepFlags.begin() + i);
                spdlog::info("Thread {} joined", m_threadNames[i]); // 日志输出
                --i;
            }
        }
        m_threads.clear();
        m_threadNames.clear();
        m_sleepFlags.clear();
        spdlog::info("All threads joined"); // 日志输出
    }

    // 等待指定名称的线程完成，并从 ThreadManager 中移除该线程
    // name：线程名称
    void ThreadManager::joinThreadByName(const std::string& name) {
        if (m_threads.empty()) { // 如果 m_threads 为空
            spdlog::warn("Thread {} not found", name); // 日志输出
            return;
        }
        std::lock_guard<std::mutex> lock(m_mtx); // 线程锁
        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                continue;
            }
            if (m_threadNames[i] == name) { // 找到要加入的线程
                if (auto& t = m_threads[i]; t) {
                    t->join(); // 加入线程
                    m_threads.erase(m_threads.begin() + i); // 从容器中移除线程
                    m_threadNames.erase(m_threadNames.begin() + i); // 从容器中移除线程名称
                    m_sleepFlags.erase(m_sleepFlags.begin() + i); // 从容器中移除睡眠标志
                    spdlog::info("Thread {} joined", name); // 日志输出
                } else {
                    spdlog::warn("Thread {} has already joined", name); // 日志输出
                }
                return;
            }
        }
        spdlog::warn("Thread {} not found", name); // 日志输出
    }

    // 让指定名称的线程休眠指定时间
    // name：线程名称
    // seconds：休眠时间（秒）
    // 返回值：如果找到了该线程，则返回 true；否则返回 false
    bool ThreadManager::sleepThreadByName(const std::string& name, int seconds) {
        if (m_threads.empty()) { // 如果 m_threads 为空
            spdlog::warn("Thread {} not found", name); // 日志输出
            return false;
        }
        std::lock_guard<std::mutex> lock(m_mtx); // 线程锁
        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                continue;
            }
            if (m_threadNames[i] == name) { // 找到要加入的线程
                if (auto& t = m_threads[i]; t) {
                    t->join(); // 加入线程
                    m_threads.erase(m_threads.begin() + i); // 从容器中移除线程
                    m_threadNames.erase(m_threadNames.begin() + i); // 从容器中移除线程名称
                    m_sleepFlags.erase(m_sleepFlags.begin() + i); // 从容器中移除睡眠标志
                    spdlog::info("Thread {} joined", name); // 日志输出
                } else {
                    spdlog::warn("Thread {} has already joined", name); // 日志输出
                }
                return true;
            }
        }
        spdlog::warn("Thread {} not found", name); // 日志输出
        return false;
    }

    // 检查指定名称的线程是否在运行
    // name：线程名称
    // 返回值：如果找到了该线程，则返回 true，表示该线程在运行；否则返回 false，表示该线程未找到
    bool ThreadManager::isThreadRunning(const std::string& name) {
        if (m_threads.empty()) { // 如果 m_threads 为空
            spdlog::warn("Thread {} not found", name); // 日志输出
            return false;
        }
        std::lock_guard<std::mutex> lock(m_mtx); // 线程锁
        for (size_t i = 0; i < m_threads.size(); ++i) {
            if (i >= m_threads.size() || i >= m_threadNames.size() || i >= m_sleepFlags.size()) {
                spdlog::error("Index out of range!");
                continue;
            }
            if (m_threadNames[i] == name) { // 找到要检查的线程
                return !m_sleepFlags[i]; // 返回线程是否在运行
            }
        }
        spdlog::warn("Thread {} not found", name); // 日志输出
        return false;
    }
}