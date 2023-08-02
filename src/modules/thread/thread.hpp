/*
 * thread.hpp
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

Date: 2023-3-27

Description: Thread Manager

**************************************************/

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <functional>
#include <condition_variable>

namespace Lithium::Thread
{
	/**
	 * @brief 线程管理器类，用于管理多个线程
	 */
	class ThreadManager
	{
	public:
		/**
		 * @brief 构造函数
		 * @param maxThreads 最大线程数
		 */
		explicit ThreadManager(int maxThreads);

		/**
		 * @brief 析构函数
		 */
		~ThreadManager();

		/**
		 * @brief 添加线程
		 * @param func 线程函数
		 * @param name 线程名称
		 */
		void addThread(std::function<void()> func, const std::string &name);

		/**
		 * @brief 等待所有线程执行完毕
		 */
		void joinAllThreads();

		/**
		 * @brief 根据名称等待指定线程执行完毕
		 * @param name 线程名称
		 */
		void joinThreadByName(const std::string &name);

		/**
		 * @brief 根据名称让指定线程休眠一定时间
		 * @param name 线程名称
		 * @param seconds 休眠时间（秒）
		 * @return 是否成功休眠线程
		 */
		bool sleepThreadByName(const std::string &name, int seconds);

		/**
		 * @brief 判断指定线程是否正在运行
		 * @param name 线程名称
		 * @return 是否正在运行
		 */
		bool isThreadRunning(const std::string &name);

		/**
		 * @brief 生成指定长度的随机字符串
		 * @param length 字符串长度
		 * @return 生成的随机字符串
		 */
		const std::string generateRandomString(int length);

	private:
#if __cplusplus >= 202002L
		/**
		 * @brief 等待线程执行完毕
		 * @param lock 互斥锁
		 * @param t 线程相关信息（包括线程对象、名称和是否正在运行）
		 */
		void joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::jthread>, std::string, bool> &t);
#else
		/**
		 * @brief 等待线程执行完毕
		 * @param lock 互斥锁
		 * @param t 线程相关信息（包括线程对象、名称和是否正在运行）
		 */
		void joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::thread>, std::string, bool> &t);
#endif

		int m_maxThreads; ///< 最大线程数
#if __cplusplus >= 202002L
		std::vector<std::tuple<std::unique_ptr<std::jthread>, std::string, bool>> m_threads; ///< 线程列表（包括线程对象、名称和是否正在运行）
#else
		std::vector<std::tuple<std::unique_ptr<std::thread>, std::string, bool>> m_threads; ///< 线程列表（包括线程对象、名称和是否正在运行）
#endif
		std::mutex m_mtx;			  ///< 互斥锁
		std::condition_variable m_cv; ///< 条件变量
		std::atomic<bool> m_stopFlag; ///< 停止标志位
	};

}