/*
 * thread.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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

#include "thread_pool.hpp"

namespace Atom::Async
{
	class ThreadContainer
	{
	public:
		ThreadContainer() = default;
		virtual ~ThreadContainer() = default;

	private:
#if __cplusplus >= 202002L
		std::unique_ptr<std::jthread> m_thread;
#else
		std::unique_ptr<std::thread> m_thread;
#endif
		std::string m_name;
		std::atomic_bool m_isRunning;
	};

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

		static std::shared_ptr<ThreadManager> createShared(int maxThreads = 10);

		/**
		 * @brief 添加线程
		 * @param func 线程函数
		 * @param name 线程名称
		 */
		void addThread(std::function<void()> func, const std::string &name);

		/**
		 * @brief 等待所有线程执行完毕
		 */
		bool joinAllThreads();

		/**
		 * @brief 根据名称等待指定线程执行完毕
		 * @param name 线程名称
		 */
		bool joinThreadByName(const std::string &name);

		/**
		 * @brief 判断指定线程是否正在运行
		 * @param name 线程名称
		 * @return 是否正在运行
		 */
		[[nodiscard]] bool isThreadRunning(const std::string &name);

		void setMaxThreads(int maxThreads);

		[[nodiscard]] int getMaxThreads() const;

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