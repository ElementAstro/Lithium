/*
 * task.hpp
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

Date: 2023-7-19

Description: Basic and Simple Task Definition

**************************************************/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace Atom::Task
{
    class BasicTask
    {
    public:
        /**
         * @brief BasicTask类构造函数
         * @param stop_fn 一个可选的停止函数，默认为nullptr
         * @param can_stop 指示任务是否可以停止，默认为false
         */
        BasicTask(const std::function<json(const json &)> &stop_fn, bool can_stop = false);

        /**
         * @brief BasicTask类析构函数
         */
        virtual ~BasicTask();

        /**
         * @brief 执行任务的纯虚函数，由子类实现具体逻辑
         * @return 以json格式返回任务执行结果
         */
        virtual const json execute() = 0;

        /**
         * @brief 将任务序列化为JSON对象
         * @return 表示任务的JSON对象
         */
        virtual const json toJson() const;

        /**
         * @brief 获取任务的执行结果
         * @return 任务的执行结果
         */
        virtual const json getResult() const;

        /**
         * @brief 获取任务的参数模板，主要用于校验
         * @return 任务参数模板
         */
        virtual const json getParamsTemplate() const;

        /**
         * @brief 设置任务参数
         * @param params 要设置的任务参数
         */
        virtual void setParams(const json &params);

        /**
         * @brief 获取任务ID
         * @return 任务ID
         */
        int getId() const;

        /**
         * @brief 设置任务ID
         * @param id 要设置的任务ID
         */
        void setId(int id);

        /**
         * @brief 获取任务名称
         * @return 任务名称
         */
        const std::string &getName() const;

        /**
         * @brief 设置任务名称
         * @param name 要设置的任务名称
         */
        void setName(const std::string &name);

        /**
         * @brief 获取任务描述
         * @return 任务描述
         */
        const std::string &getDescription() const;

        /**
         * @brief 设置任务描述
         * @param description 要设置的任务描述
         */
        void setDescription(const std::string &description);

        /**
         * @brief 设置任务是否可以执行
         * @param can_execute 指示任务是否可以执行
         */
        void setCanExecute(bool can_execute);

        /**
         * @brief 获取任务是否可以执行
         * @return true表示任务可以执行，false表示任务不可执行
         */
        bool isExecutable() const;

        /**
         * @brief 设置停止函数
         * @param stop_fn 停止函数
         */
        void setStopFunction(const std::function<json(const json &)> &stop_fn);

        /**
         * @brief 获取停止标志
         * @return true表示任务已停止，false表示任务未停止
         */
        bool getStopFlag() const;

        /**
         * @brief 设置停止标志
         * @param flag 停止标志，true表示任务已停止，false表示任务未停止
         */
        void setStopFlag(bool flag);

        /**
         * @brief 停止任务的虚函数，由子类实现具体停止逻辑
         */
        virtual void stop();

        /**
         * @brief 验证json值与模板值是否匹配
         * @param data 要验证的JSON值
         * @param templateValue 模板值
         * @return true表示匹配，false表示不匹配
         */
        bool validateJsonValue(const json &data, const json &templateValue);

        /**
         * @brief 验证json字符串与模板字符串是否匹配
         * @param jsonString 要验证的json字符串
         * @param templateString 模板字符串
         * @return true表示匹配，false表示不匹配
         */
        bool validateJsonString(const std::string &jsonString, const std::string &templateString);

    protected:
        // True if the task is completed
        std::atomic<bool> done_ = false;

        // Task ID
        int id_;

        // Task name
        std::string name_;

        // Task description
        std::string description_;

        // True if the task can be stopped
        bool can_stop_;

        // Stop function
        std::function<json(const json &)> stop_fn_;

        // Stop flag
        bool stop_flag_ = false;

        // True if the task can be executed
        bool can_execute_ = true;
    };

    class SimpleTask : public BasicTask
    {
    public:
        /**
         * @brief SimpleTask类构造函数
         * @param func 要执行的函数
         * @param params_template 参数模板，用于参数验证
         * @param stop_fn 一个可选的停止函数，默认为nullptr
         * @param can_stop 指示任务是否可以停止，默认为false
         */
        SimpleTask(const std::function<json(const json &)> &func,
                   const json &params_template,
                   const std::function<json(const json &)> &stop_fn,
                   bool can_stop = false);

        /**
         * @brief 执行任务的虚函数，由子类实现具体逻辑
         * @return 以json格式返回任务执行结果
         */
        virtual const json execute() override;

        /**
         * @brief 设置任务参数
         * @param params 要设置的任务参数
         */
        virtual void setParams(const json &params) override;

        /**
         * @brief 将任务序列化为JSON对象
         * @return 表示任务的JSON对象
         */
        virtual const json toJson() const override;

        /**
         * @brief 获取任务的执行结果
         * @return 任务的执行结果
         */
        virtual const json getResult() const override;

        virtual const json getParamsTemplate() const override;

    private:
        // 要执行的函数
        std::function<json(const json &)> function_;

        // 参数传递给函数
        json params_;

        // 用于检查的参数模板
        json params_template_;

        // 函数的执行结果
        json returns_;
    };

}