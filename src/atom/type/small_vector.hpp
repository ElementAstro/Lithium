/*
 * small_vector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-17

Description: A Small Vector Implementation

**************************************************/

#ifndef ATOM_TYPE_SMALL_VECTOR_HPP
#define ATOM_TYPE_SMALL_VECTOR_HPP

#include <algorithm>
#include <vector>

template <typename T, size_t N>
class SmallVector {
public:
    // 构造函数
    SmallVector() : m_size(0), m_capacity(N) { m_data = m_inlineData; }

    // 复制构造函数
    SmallVector(const SmallVector &other)
        : m_size(other.m_size), m_capacity(other.m_capacity) {
        if (other.m_size > N) {
            m_data = new T[other.m_capacity];
            std::copy(other.m_data, other.m_data + other.m_size, m_data);
        } else {
            m_data = m_inlineData;
            std::copy(other.m_data, other.m_data + other.m_size, m_data);
        }
    }

    // 移动构造函数
    SmallVector(SmallVector &&other) noexcept
        : m_size(other.m_size),
          m_capacity(other.m_capacity),
          m_data(other.m_data) {
        if (other.m_size > N) {
            other.m_data = nullptr;
        } else {
            std::copy(other.m_inlineData, other.m_inlineData + other.m_size,
                      m_inlineData);
        }
    }

    // 析构函数
    ~SmallVector() {
        if (m_data != m_inlineData) {
            delete[] m_data;
        }
    }

    // 赋值操作符重载
    SmallVector &operator=(const SmallVector &other) {
        if (this != &other) {
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            if (other.m_size > N) {
                if (m_data != m_inlineData) {
                    delete[] m_data;
                }
                m_data = new T[other.m_capacity];
                std::copy(other.m_data, other.m_data + other.m_size, m_data);
            } else {
                if (m_data != m_inlineData) {
                    delete[] m_data;
                }
                m_data = m_inlineData;
                std::copy(other.m_data, other.m_data + other.m_size, m_data);
            }
        }
        return *this;
    }

    // 添加元素
    void push_back(const T &value) {
        if (m_size == m_capacity) {
            if (m_capacity == N) {
                m_capacity *= 2;
                m_data = new T[m_capacity];
                std::copy(m_inlineData, m_inlineData + m_size, m_data);
            } else {
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
    void pop_back() {
        if (m_size > 0) {
            --m_size;
        }
    }

    // 清空向量
    void clear() { m_size = 0; }

    // 获取元素个数
    size_t size() const { return m_size; }

    // 获取容量大小
    size_t capacity() const { return m_capacity; }

    // 判断向量是否为空
    bool empty() const { return m_size == 0; }

    // 通过索引访问元素
    T &operator[](size_t index) { return m_data[index]; }

private:
    size_t m_size;      // 元素个数
    size_t m_capacity;  // 容量大小
    T *m_data;          // 存储数据的指针
    T m_inlineData[N];  // 内置的数据存储空间
};

#endif
