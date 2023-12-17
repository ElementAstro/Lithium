/*
 * small_hashmap.hpp
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

Date: 2023-12-17

Description: A Small Hash Map Implementation

**************************************************/

#include <unordered_map>

template <typename K, typename V, size_t N>
class SmallHashMap
{
public:
    // 构造函数
    SmallHashMap()
    {
        m_data.reserve(N);
    }

    // 插入键值对
    void insert(const K &key, const V &value)
    {
        m_data[key] = value;
    }

    // 通过键获取值
    V get(const K &key)
    {
        auto it = m_data.find(key);
        if (it != m_data.end())
        {
            return it->second;
        }
        else
        {
            return V();
        }
    }

    // 删除键值对
    void erase(const K &key)
    {
        m_data.erase(key);
    }

    // 清空哈希表
    void clear()
    {
        m_data.clear();
    }

    // 获取键值对个数
    size_t size() const
    {
        return m_data.size();
    }

    // 判断哈希表是否为空
    bool empty() const
    {
        return m_data.empty();
    }

private:
    std::unordered_map<K, V> m_data; // 存储数据的容器
};

/*

int main()
{
    SmallHashMap<std::string, int, 4> hashMap;

    hashMap.insert("apple", 1);
    hashMap.insert("banana", 2);
    hashMap.insert("orange", 3);
    hashMap.insert("grape", 4);
    hashMap.insert("watermelon", 5);

    std::cout << "Value of 'banana': " << hashMap.get("banana") << std::endl;
    std::cout << "Value of 'watermelon': " << hashMap.get("watermelon") << std::endl;

    hashMap.erase("banana");

    std::cout << "Value of 'banana' after erase: " << hashMap.get("banana") << std::endl;

    std::cout << "Size: " << hashMap.size() << std::endl;
    std::cout << "Empty: " << (hashMap.empty() ? "Yes" : "No") << std::endl;

    hashMap.clear();

    std::cout << "Size after clear: " << hashMap.size() << std::endl;
    std::cout << "Empty after clear: " << (hashMap.empty() ? "Yes" : "No") << std::endl;

    return 0;
}

*/