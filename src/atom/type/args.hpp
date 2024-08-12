/*
 * args.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: Argument Container Library for C++

**************************************************/

#ifndef ATOM_TYPE_ARG_HPP
#define ATOM_TYPE_ARG_HPP

#include <any>
#include <optional>
#include <string_view>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// 设置参数的便捷宏
#define SET_ARGUMENT(container, name, value) container.set(#name, value)

// 获取参数的便捷宏
#define GET_ARGUMENT(container, name, type) \
    container.get<type>(#name).value_or(type{})

// 检查参数是否存在的便捷宏
#define HAS_ARGUMENT(container, name) container.contains(#name)

// 删除参数的便捷宏
#define REMOVE_ARGUMENT(container, name) container.remove(#name)

/**
 * @brief 通用容器,用于存储任意类型的键值对。
 * @brief A universal container for storing any type of key-value pairs.
 * @note 这是ArgumentContainer的弱化版,虽然功能少,但是性能更好。
 * @note This is a weak version of ArgumentContainer, although it has fewer
 * features, it has better performance.
 */
class Args {
public:
    /**
     * @brief 设置键值对。
     * @brief Set key-value pairs.
     * @param key 键。
     * @param key Key.
     * @param value 值。
     * @param value Value.
     * @note 如果键已存在,则会覆盖原有的值。
     * @note If the key exists, it will overwrite the original value.
     */
    template <typename T>
    void set(std::string_view key, T &&value) {
        m_data_[key] = std::forward<T>(value);
    }

    /**
     * @brief 获取键对应的值。
     * @brief Get the value corresponding to the key.
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     * @note 如果键不存在,则会抛出异常。
     * @note If the key does not exist, an exception will be thrown.
     */
    template <typename T>
    auto get(std::string_view key) const -> T {
        return std::any_cast<T>(m_data_.at(key));
    }

    /**
     * @brief 获取键对应的值(如果键不存在,则返回默认值)。
     * @brief Get the value corresponding to the key (if the key does not exist,
     * return the default value).
     * @param key 键。
     * @param key Key.
     * @param default_value 默认值。
     * @param default_value Default value.
     * @return 值。
     * @return Value.
     */
    template <typename T>
    auto getOr(std::string_view key, T &&default_value) const -> T {
        if (auto data = m_data_.find(key); data != m_data_.end()) {
            return std::any_cast<T>(data->second);
        }
        return std::forward<T>(default_value);
    }

    /**
     * @brief 获取键对应的值(如果键不存在,则返回 std::nullopt)。
     * @brief Get the value corresponding to the key (if the key does not exist,
     * return std::nullopt).
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     */
    template <typename T>
    auto getOptional(std::string_view key) const -> std::optional<T> {
        if (auto data = m_data_.find(key); data != m_data_.end()) {
            return std::any_cast<T>(data->second);
        }
        return std::nullopt;
    }

    /**
     * @brief 检查键是否存在。
     * @brief Check if the key exists.
     * @param key 键。
     * @param key Key.
     * @return 如果键存在,则返回true；否则返回false。
     * @return If the key exists, return true; otherwise return false.
     */
    auto contains(std::string_view key) const noexcept -> bool {
        if (auto data = m_data_.find(key); data != m_data_.end()) {
            return true;
        }
        return false;
    }

    /**
     * @brief 移除键值对。
     * @brief Remove key-value pairs.
     * @param key 键。
     * @param key Key.
     */
    void remove(std::string_view key) { m_data_.erase(key); }

    /**
     * @brief 清空容器。
     * @brief Clear the container.
     */
    void clear() noexcept { m_data_.clear(); }
    /**
     * @brief 获取容器中键值对的数量。
     * @brief Get the number of key-value pairs in the container.
     * @return 键值对的数量。
     * @return The number of key-value pairs.
     */
    auto size() const noexcept -> size_t { return m_data_.size(); }

    /**
     * @brief 检查容器是否为空。
     * @brief Check if the container is empty.
     * @return 如果容器为空,则返回true；否则返回false。
     * @return If the container is empty, return true; otherwise return false.
     */
    auto empty() const noexcept -> bool { return m_data_.empty(); }

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string_view, std::any> data()
#else
    std::unordered_map<std::string_view, std::any> data()
#endif
        const noexcept {
        return m_data_;
    }

    /**
     * @brief 获取指定键对应的值。
     * @brief Get the value corresponding to the specified key.
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     * @note 如果键不存在,则会插入一个新的键值对。
     * @note If the key does not exist, a new key-value pair will be inserted.
     */
    template <typename T>
    auto operator[](std::string_view key) -> T & {
        return std::any_cast<T &>(m_data_[key]);
    }

    /**
     * @brief 获取指定键对应的值。
     * @brief Get the value corresponding to the specified key.
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     * @note 如果键不存在,则会抛出异常。
     * @note If the key does not exist, an exception will be thrown.
     */
    template <typename T>
    auto operator[](std::string_view key) const -> const T & {
        return std::any_cast<const T &>(m_data_.at(key));
    }

    /**
     * @brief 获取指定键对应的值。
     * @brief Get the value corresponding to the specified key.
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     * @note 如果键不存在,则会插入一个新的键值对。
     * @note If the key does not exist, a new key-value pair will be inserted.
     */
    auto operator[](std::string_view key) -> std::any & { return m_data_[key]; }

    /**
     * @brief 获取指定键对应的值。
     * @brief Get the value corresponding to the specified key.
     * @param key 键。
     * @param key Key.
     * @return 值。
     * @return Value.
     * @note 如果键不存在,则会抛出异常。
     * @note If the key does not exist, an exception will be thrown.
     */
    auto operator[](std::string_view key) const -> const std::any & {
        return m_data_.at(key);
    }

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_data_;
#else
    std::unordered_map<std::string_view, std::any> m_data_;
#endif
};

#endif
