/*
 * serialize.hpp
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

Date: 2023-11-3

Description: This file contains the declaration of the SerializationEngine class

**************************************************/

#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <any>
#include <optional>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// 基类渲染引擎
class RenderEngine
{
public:
    /**
     * @brief 渲染函数，用于将数据渲染为字符串
     *
     * @param data 渲染的数据，可以是任意类型
     * @param format 是否格式化输出，默认为false
     *
     * @return std::string 渲染后的字符串
     */
    virtual std::string render(const std::any &data, bool format = false) const = 0;
};

/**
 * @brief Json渲染引擎
 *
 * Json rendering engine
 */
class JsonRenderEngine : public RenderEngine
{
public:
    std::string render(const std::any &data, bool format = false) const override;
};

/**
 * @brief XML渲染引擎
 *
 * XML rendering engine
 */
class XmlRenderEngine : public RenderEngine
{
public:
    std::string render(const std::any &data, bool format = false) const override;
};

/**
 * @brief YAML渲染引擎
 *
 * YAML rendering engine
 */
class YamlRenderEngine : public RenderEngine
{
public:
    std::string render(const std::any &data, bool format = false) const override;
};

/**
 * @brief INI渲染引擎
 *
 * INI rendering engine
 */
class IniRenderEngine : public RenderEngine
{
public:
    std::string render(const std::any &data, bool format = false) const override;
};

/**
 * @brief TOML渲染引擎
 *
 * TOML rendering engine
 */
class TomlRenderEngine : public RenderEngine
{
public:
    std::string render(const std::any &data, bool format = false) const override;
};

/**
 * @brief 序列化引擎类
 *
 * Serialization engine class
 */
class SerializationEngine
{
public:
    /**
     * @brief 构造函数，初始化SerializationEngine对象
     *
     * Constructor, initializes the SerializationEngine object
     */
    SerializationEngine();

    /**
     * @brief 析构函数，默认析构函数
     *
     * Destructor, default destructor
     */
    ~SerializationEngine() = default;

    /**
     * @brief 添加渲染引擎到SerializationEngine中
     *
     * Add a render engine to the SerializationEngine
     *
     * @param name 渲染引擎的名称
     * @param renderEngine 渲染引擎的智能指针
     */
    void addRenderEngine(const std::string &name, const std::shared_ptr<RenderEngine> &renderEngine);

    /**
     * @brief 设置当前选中的渲染引擎
     *
     * Set the currently selected render engine
     *
     * @param name 渲染引擎的名称
     *
     * @return bool 设置是否成功，成功返回true，否则返回false
     */
    bool setCurrentRenderEngine(const std::string &name);

    /**
     * @brief 序列化数据
     *
     * Serialize data
     *
     * @tparam T 序列化数据的类型
     * @param data 序列化的数据
     * @param format 是否格式化输出，默认为false
     *
     * @return std::optional<std::string> 序列化后的字符串，如果序列化失败，则返回std::nullopt
     */
    template <typename T>
    std::optional<std::string> serialize(const T &data, bool format = false) const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_mutex));

        auto it = m_renderEngines.find(m_currentRenderEngine);
        if (it != m_renderEngines.end())
        {
            try
            {
                std::any anyData = data;
                return it->second->render(anyData, format);
            }
            catch (const std::bad_any_cast &)
            {
                return std::nullopt;
            }
        }

        return std::nullopt;
    }

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<RenderEngine>> m_renderEngines; // 渲染引擎的容器，用于存储不同渲染引擎
#else
    std::unordered_map<std::string, std::shared_ptr<RenderEngine>> m_renderEngines; // 渲染引擎的容器，用于存储不同渲染引擎
#endif
    std::string m_currentRenderEngine; // 记录当前选中的 RenderEngine 的名称
    std::mutex m_mutex;                // 互斥锁，保证线程安全
};
