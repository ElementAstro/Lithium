/*
 * serialize.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: This file contains the declaration of the SerializationEngine class

**************************************************/

#ifndef ATOM_SERVER_SERIALIZE_HPP
#define ATOM_SERVER_SERIALIZE_HPP

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// 基类渲染引擎
class Serialization {
public:
    /**
     * @brief 渲染函数，用于将数据渲染为字符串
     *
     * @param data 渲染的数据，可以是任意类型
     * @param format 是否格式化输出，默认为false
     *
     * @return std::string 渲染后的字符串
     */
    virtual std::string serialize(const std::any &data,
                                  bool format = false) const = 0;
};

/**
 * @brief Json渲染引擎
 *
 * Json serializeing engine
 */
class JsonSerializationEngine : public Serialization {
public:
    std::string serialize(const std::any &data,
                          bool format = false) const override;
};

/**
 * @brief XML渲染引擎
 *
 * XML serializeing engine
 */
class XmlSerializationEngine : public Serialization {
public:
    std::string serialize(const std::any &data,
                          bool format = false) const override;
};

/**
 * @brief YAML渲染引擎
 *
 * YAML serializeing engine
 */
class YamlSerializationEngine : public Serialization {
public:
    std::string serialize(const std::any &data,
                          bool format = false) const override;
};

/**
 * @brief INI渲染引擎
 *
 * INI serializeing engine
 */
class IniSerializationEngine : public Serialization {
public:
    std::string serialize(const std::any &data,
                          bool format = false) const override;
};

namespace Atom::Server {
/**
 * @brief 序列化引擎类
 *
 * Serialization engine class
 */
class SerializationEngine {
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
     * Add a serialize engine to the SerializationEngine
     *
     * @param name 渲染引擎的名称
     * @param serializeEngine 渲染引擎的智能指针
     */
    void addSerializationEngine(
        const std::string &name,
        const std::shared_ptr<Serialization> &serializeEngine);

    /**
     * @brief 设置当前选中的渲染引擎
     *
     * Set the currently selected serialize engine
     *
     * @param name 渲染引擎的名称
     *
     * @return bool 设置是否成功，成功返回true，否则返回false
     */
    bool setCurrentSerializationEngine(const std::string &name);

    /**
     * @brief 序列化数据
     *
     * Serialize data
     *
     * @tparam T 序列化数据的类型
     * @param data 序列化的数据
     * @param format 是否格式化输出，默认为false
     *
     * @return std::optional<std::string>
     * 序列化后的字符串，如果序列化失败，则返回std::nullopt
     */
    template <typename T>
    std::optional<std::string> serialize(const T &data,
                                         bool format = false) const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<Serialization>>
        m_serializeEngines;  // 渲染引擎的容器，用于存储不同渲染引擎
#else
    std::unordered_map<std::string, std::shared_ptr<Serialization>>
        m_serializeEngines;  // 渲染引擎的容器，用于存储不同渲染引擎
#endif
    std::string m_currentSerializationEngine;  // 记录当前选中的
                                               // SerializationEngine 的名称
    std::mutex m_mutex;                        // 互斥锁，保证线程安全
};

template <typename T>
std::optional<std::string> SerializationEngine::serialize(const T &data,
                                                          bool format) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_mutex));

    auto it = m_serializeEngines.find(m_currentSerializationEngine);
    if (it != m_serializeEngines.end()) {
        try {
            std::any anyData = data;
            return it->second->serialize(anyData, format);
        } catch (const std::bad_any_cast &) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

}  // namespace Atom::Server

#endif
