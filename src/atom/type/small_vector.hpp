/*
 * small_vector.hpp
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

Description: A Small Vector Implementation

**************************************************/

#pragma once

#include <vector>
#include <algorithm>

template <typename T, size_t N>
class SmallVector
{
public:
    // 构造函数
    SmallVector() : m_size(0), m_capacity(N)
    {
        m_data = m_inlineData;
    }

    // 复制构造函数
    SmallVector(const SmallVector &other) : m_size(other.m_size), m_capacity(other.m_capacity)
    {
        if (other.m_size > N)
        {
            m_data = new T[other.m_capacity];
            std::copy(other.m_data, other.m_data + other.m_size, m_data);
        }
        else
        {
            m_data = m_inlineData;
            std::copy(other.m_data, other.m_data + other.m_size, m_data);
        }
    }

    // 移动构造函数
    SmallVector(SmallVector &&other) noexcept : m_size(other.m_size), m_capacity(other.m_capacity), m_data(other.m_data)
    {
        if (other.m_size > N)
        {
            other.m_data = nullptr;
        }
        else
        {
            std::copy(other.m_inlineData, other.m_inlineData + other.m_size, m_inlineData);
        }
    }

    // 析构函数
    ~SmallVector()
    {
        if (m_data != m_inlineData)
        {
            delete[] m_data;
        }
    }

    // 赋值操作符重载
    SmallVector &operator=(const SmallVector &other)
    {
        if (this != &other)
        {
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            if (other.m_size > N)
            {
                if (m_data != m_inlineData)
                {
                    delete[] m_data;
                }
                m_data = new T[other.m_capacity];
                std::copy(other.m_data, other.m_data + other.m_size, m_data);
            }
            else
            {
                if (m_data != m_inlineData)
                {
                    delete[] m_data;
                }
                m_data = m_inlineData;
                std::copy(other.m_data, other.m_data + other.m_size, m_data);
            }
        }
        return *this;
    }

    // 添加元素
    void push_back(const T &value)
    {
        if (m_size == m_capacity)
        {
            if (m_capacity == N)
            {
                m_capacity *= 2;
                m_data = new T[m_capacity];
                std::copy(m_inlineData, m_inlineData + m_size, m_data);
            }
            else
            {
                m_capacity *= 2;
                T *newData = new T[m_capacity];
                std::copy(m_data, m_data + m_size, newData);
                delete[] m_data;
                m_data = newData;
            }
        }
        m_data[m_size++] = value;
    }

    // 删除最后一个元素
    void pop_back()
    {
        if (m_size > 0)
        {
            --m_size;
        }
    }

    // 清空向量
    void clear()
    {
        m_size = 0;
    }

    // 获取元素个数
    size_t size() const
    {
        return m_size;
    }

    // 获取容量大小
    size_t capacity() const
    {
        return m_capacity;
    }

    // 判断向量是否为空
    bool empty() const
    {
        return m_size == 0;
    }

    // 通过索引访问元素
    T &operator[](size_t index)
    {
        return m_data[index];
    }

private:
    size_t m_size;     // 元素个数
    size_t m_capacity; // 容量大小
    T *m_data;         // 存储数据的指针
    T m_inlineData[N]; // 内置的数据存储空间
};

/*

int main()
{
    SmallVector<int, 4> vec;

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);

    std::cout << "Size: " << vec.size() << std::endl;
    std::cout << "Capacity: " << vec.capacity() << std::endl;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        std::cout << vec[i] << " ";
    }
    std::cout << std::endl;

    vec.pop_back();
    vec.clear();

    std::cout << "Size: " << vec.size() << std::endl;
    std::cout << "Capacity: " << vec.capacity() << std::endl;
    std::cout << "Empty: " << (vec.empty() ? "Yes" : "No") << std::endl;

    return 0;
}

*/