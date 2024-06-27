/*
 * ini.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: INI File Read/Write Library

**************************************************/

#ifndef ATOM_TYPE_INI_HPP
#define ATOM_TYPE_INI_HPP

#include <any>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace atom::type {
class INIFile {
public:
    /**
     * @brief 加载INI文件
     * @param filename 文件名
     */
    void load(const std::string &filename);

    /**
     * @brief 保存INI文件
     * @param filename 文件名
     */
    void save(const std::string &filename) const;

    /**
     * @brief 设置INI文件中的值
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @param value 值
     */
    template <typename T>
    void set(const std::string &section, const std::string &key,
             const T &value);

    /**
     * @brief 获取INI文件中的值
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @return 值，如果不存在则返回std::nullopt
     */
    template <typename T>
    [[nodiscard]] auto get(const std::string &section,
                                       const std::string &key) const -> std::optional<T>;

    /**
     * @brief 判断INI文件中是否存在指定键
     * @param section 部分名
     * @param key 键
     * @return 存在返回true，否则返回false
     */
    [[nodiscard]] auto has(const std::string &section,
                           const std::string &key) const -> bool;

    /**
     * @brief 判断INI文件中是否存在指定部分
     * @param section 部分名
     * @return 存在返回true，否则返回false
     */
    [[nodiscard]] auto hasSection(const std::string &section) const -> bool;

    /**
     * @brief 获取INI文件中所有的section
     * @return section列表
     */
    [[nodiscard]] auto sections() const -> std::vector<std::string>;

    /**
     * @brief 获取指定section下的所有key
     * @param section 部分名
     * @return key列表
     */
    [[nodiscard]] std::vector<std::string> keys(
        const std::string &section) const;

    /**
     * @brief 获取INI文件中的部分
     * @param section 部分名
     * @return 部分内容
     */
    auto operator[](
        const std::string &section) -> std::unordered_map<std::string, std::any> & {
        return data_[section];
    }

    /**
     * @brief 将INI文件中的数据转换为JSON字符串
     * @return JSON字符串
     */
    [[nodiscard]] std::string toJson() const;

    /**
     * @brief 将INI文件中的数据转换为XML字符串
     * @return XML字符串
     */
    [[nodiscard]] std::string toXml() const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::any>>
        data_;                                 // 存储数据的映射表
    mutable std::shared_mutex m_sharedMutex_;  // 共享互斥锁，用于线程安全

    /**
     * @brief 解析INI文件的一行，并更新当前部分
     * @param line 行内容
     * @param currentSection 当前部分
     */
    void parseLine(std::string_view line, std::string &currentSection);
};
}  // namespace atom::type

#include "ini.inl"

#endif