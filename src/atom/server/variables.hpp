/*
 * variables.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-14

Description: Variable Registry 类，用于注册、获取和观察变量值。

**************************************************/

#ifndef ATOM_SERVER_VARIABLES_HPP
#define ATOM_SERVER_VARIABLES_HPP

#include <any>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <string>

#if ENALE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/noncopyable.hpp"
#include "atom/type/string.hpp"

#include "atom/error/error_stack.hpp"

/**
 * @brief 变量注册器类，用于注册、获取和观察变量值。
 */
class VariableRegistry : public NonCopyable {
public:
    explicit VariableRegistry(const std::string &name);

    /**
     * @brief 获取变量值的回调函数类型。
     */
    template <typename T>
    using Getter = std::function<T()>;
    /**
     * @brief 设置变量值的回调函数类型。
     */
    template <typename T>
    using Setter = std::function<bool(const T &)>;

    /**
     * @brief 观察者 struct，包含观察者名称和回调函数。
     */
    struct Observer {
        std::string name;
        std::function<void(const std::string &)> callback;
    };

    /**
     * @brief 注册变量，如果变量已经存在，则返回 false。
     * @tparam T 变量类型，任何可转换为 std::any 类型的类型均可。
     * @param name 变量名称。
     * @param initialValue 变量初始值。
     * @param descirption 变量描述。
     * @return 是否注册成功，如果变量名已经存在，则返回 false。
     */
    template <typename T>
    bool RegisterVariable(const std::string &name, const T &initialValue,
                          const std::string descirption = "");

    template <typename T>
    void SetVariableRange(const std::string &name, const T &lower,
                          const T &upper);

    /**
     * @brief 设置指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 变量值。
     * @return 是否设置成功，如果变量不存在，则返回 false。
     */
    template <typename T>
    bool SetVariable(const std::string &name, const T &value);

    /**
     * @brief 获取指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @return 指定名称的变量值，如果不存在，返回 std::nullopt。
     */
    template <typename T>
    [[nodiscard]] std::optional<T> GetVariable(const std::string &name) const;

    /**
     * @brief 判断指定名称的变量是否存在。
     * @param name 变量名称。
     * @return 指定名称的变量是否存在。
     */
    bool HasVariable(const std::string &name) const;

    /**
     * @brief 获取指定名称的变量描述。
     * @param name 变量名称。
     * @return 指定名称的变量描述，如果不存在，返回空字符串。
     */
    [[nodiscard]] std::string GetDescription(const std::string &name) const;

    /**
     * @brief 添加观察者，用于观察变量值的变化。
     * @param name 变量名称。
     * @param observer 观察者 struct。
     */
    void AddObserver(const std::string &name, const Observer &observer);

    /**
     * @brief 通知指定变量名称的观察者，变量值已经发生变化。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 新的变量值。
     */
    template <typename T>
    void NotifyObservers(const std::string &name, const T &value) const;

    /**
     * @brief 移除指定变量名称的观察者。
     * @param name 变量名称。
     * @param observerName 观察者名称。
     * @return 是否移除成功，如果观察者不存在，则返回 false。
     */
    bool RemoveObserver(const std::string &name,
                        const std::string &observerName);

    /**
     * @brief 获取所有变量。
     * @return 所有变量的 map。
     */
#if ENALE_FASTHASH
    [[nodiscard]] emhash8::HashMap<std::string, std::any> GetAll() const;
#else
    [[nodiscard]] std::unordered_map<std::string, std::any> GetAll() const;
#endif

    /**
     * @brief 清空所有变量。
     */
    bool RemoveAll();

    /**
     * @brief 添加获取指定名称变量的回调函数。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param getter 获取变量值的函数。
     */
    template <typename T>
    void AddGetter(const std::string &name, const std::function<T()> &getter);

    /**
     * @brief 添加检测指定名称变量修改的回调函数。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param setter 检测变量修改的函数。
     */
    template <typename T>
    void AddSetter(const std::string &name,
                   const std::function<void(const std::any &)> &setter);

private:
    std::string m_name;
#if ENALE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_variables;
    emhash8::HashMap<std::string, std::string> m_descriptions;
    emhash8::HashMap<std::string, std::vector<Observer>> m_observers;
    emhash8::HashMap<std::string, std::function<std::any()>> m_getters;
    emhash8::HashMap<std::string, std::function<void(const std::any &)>>
        m_setters;
#else
    /**
     * @brief 所有变量的集合。
     */
    std::unordered_map<std::string, std::any> m_variables;

    /**
     * @brief 变量范围的集合，以变量名称为键。
     * @note 注意，只有数字类型支持
     */
    std::unordered_map<std::string, std::pair<std::any, std::any>> m_ranges;

    /**
     * @brief 所有变量的描述。
     */
    std::unordered_map<std::string, std::string> m_descriptions;

    /**
     * @brief 观察者的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::vector<Observer>> m_observers;

    /**
     * @brief 获取函数的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::function<std::any()>> m_getters;

    /**
     * @brief 检测修改函数的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::function<void(const std::any &)>>
        m_setters;
#endif

    /**
     * @brief 共享互斥锁，用于保证多线程安全。
     */
    mutable std::shared_mutex m_sharedMutex;
};

template <typename T>
bool VariableRegistry::RegisterVariable(const std::string &name,
                                        const T &initialValue,
                                        const std::string descirption) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);

    if (m_variables.find(name) != m_variables.end()) {
        return false;
    }

    m_variables[name] = initialValue;
    m_descriptions[name] = descirption;
    return true;
}

template <typename T>
bool VariableRegistry::SetVariable(const std::string &name, const T &value) {
    // std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (auto it = m_variables.find(name); it != m_variables.end()) {
        if (auto range = m_ranges.find(name); range != m_ranges.end()) {
            if (range->second.first.has_value() &&
                range->second.second.has_value()) {
                try {
                    if (value < std::any_cast<T>(range->second.first) ||
                        value > std::any_cast<T>(range->second.second)) {
                        return false;
                    }
                } catch (const std::bad_any_cast &e) {
                    return false;
                }
            }
        }
        if (auto setter = m_setters.find(name); setter != m_setters.end()) {
            setter->second(value);
        }
        it->second = value;
        NotifyObservers(name, value);
        return true;
    }
    return false;
}

template <typename T>
void VariableRegistry::SetVariableRange(const std::string &name, const T &lower,
                                        const T &upper) {
    static_assert(std::is_arithmetic<T>::value,
                  "Only numeric types are supported.");
    m_ranges[name] = std::make_pair(lower, upper);
}

template <typename T>
std::optional<T> VariableRegistry::GetVariable(const std::string &name) const {
    // std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    if (auto it = m_variables.find(name); it != m_variables.end()) {
        if (typeid(T) != it->second.type()) {
            return std::nullopt;
        }
        if (auto getter = m_getters.find(name); getter != m_getters.end()) {
            getter->second();
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast &) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

template <typename T>
void VariableRegistry::NotifyObservers(const std::string &name,
                                       const T &value) const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    if (!HasVariable(name)) {
        return;
    }
    if (auto it = m_observers.find(name); it != m_observers.end()) {
        std::stringstream ss;
        ss << value;
        std::string valueSring = ss.str();
        for (const auto &observer : it->second) {
            observer.callback(valueSring);
        }
    }
}

template <typename T>
void VariableRegistry::AddGetter(const std::string &name,
                                 const std::function<T()> &getter) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_getters.find(name) != m_getters.end()) {
        return;
    }
    m_getters[name] = getter;
}

template <typename T>
void VariableRegistry::AddSetter(
    const std::string &name,
    const std::function<void(const std::any &)> &setter) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_setters.find(name) != m_setters.end()) {
        return;
    }
    m_setters[name] = setter;
}

#endif
