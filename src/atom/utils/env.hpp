/*
 * env.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Environment variable management

**************************************************/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace Atom::Utils
{
    /**
     * @brief 环境变量类，用于获取和设置程序的环境变量、命令行参数等信息。
     */
    class Env
    {
    public:
        /**
         * @brief 构造函数，初始化环境变量信息。
         * @param argc 命令行参数数量。
         * @param argv 命令行参数数组。
         */
        Env(int argc, char **argv);

        /**
         * @brief 添加一个键值对到环境变量中。
         * @param key 键名。
         * @param val 键值。
         */
        void add(const std::string &key, const std::string &val);

        /**
         * @brief 判断环境变量中是否存在指定的键名。
         * @param key 键名。
         * @return 如果存在则返回 true，否则返回 false。
         */
        bool has(const std::string &key);

        /**
         * @brief 从环境变量中删除指定的键值对。
         * @param key 键名。
         */
        void del(const std::string &key);

        /**
         * @brief 获取指定键名的键值，如果不存在则返回默认值。
         * @param key 键名。
         * @param default_value 默认值。
         * @return 键值或默认值。
         */
        std::string get(const std::string &key, const std::string &default_value);

        /**
         * @brief 添加一个命令行参数和描述到帮助信息列表中。
         * @param key 参数名。
         * @param desc 参数描述。
         */
        void addHelp(const std::string &key, const std::string &desc);

        /**
         * @brief 从帮助信息列表中删除指定的参数信息。
         * @param key 参数名。
         */
        void removeHelp(const std::string &key);

        /**
         * @brief 打印程序的帮助信息。包含所有已添加的命令行参数和描述。
         */
        void printHelp();

        /**
         * @brief 设置指定键名的环境变量值。
         * @param key 键名。
         * @param val 键值。
         * @return 如果设置成功则返回 true，否则返回 false。
         */
        bool setEnv(const std::string &key, const std::string &val);

        /**
         * @brief 获取指定键名的环境变量值，如果不存在则返回默认值。
         * @param key 键名。
         * @param default_value 默认值。
         * @return 键值或默认值。
         */
        std::string getEnv(const std::string &key, const std::string &default_value);

        /**
         * @brief 获取指定路径的绝对路径。
         * @param path 路径。
         * @return 绝对路径。
         */
        std::string getAbsolutePath(const std::string &path) const;

        /**
         * @brief 获取工作目录下指定路径的绝对路径。
         * @param path 路径。
         * @return 绝对路径。
         */
        std::string getAbsoluteWorkPath(const std::string &path) const;

        /**
         * @brief 获取配置文件的路径。默认情况下，配置文件与程序在同一目录下。
         * @return 配置文件路径。
         */
        std::string getConfigPath();

    private:
        std::string m_exe;     ///< 可执行文件的全路径。
        std::string m_cwd;     ///< 工作目录。
        std::string m_program; ///< 程序名称。

        std::unordered_map<std::string, std::string> m_args;      ///< 命令行参数列表。
        std::vector<std::pair<std::string, std::string>> m_helps; ///< 帮助信息列表。
        mutable std::mutex m_mutex;                               ///< 互斥锁，用于保护成员变量。
    };

}