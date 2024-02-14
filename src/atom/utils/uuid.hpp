/*
 * uuid.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#ifndef ATOM_UTILS_UUID_HPP
#define ATOM_UTILS_UUID_HPP

#include <random>
#include <string>
#include <sstream>

namespace Atom::Utils
{
    /**
     * @brief 生成UUID的类
     *
     */
    class UUIDGenerator
    {
    private:
        mutable std::random_device rd_;                           // 随机设备
        mutable std::mt19937 gen_;                                // Mersenne Twister 19937伪随机数生成器
        mutable std::uniform_int_distribution<unsigned int> dis_; // 均匀分布

    public:
        /**
         * @brief 构造函数，初始化随机设备、生成器和均匀分布
         *
         */
        UUIDGenerator();

        /**
         * @brief 设置伪随机数生成器的种子
         *
         * @param seed_value 种子值
         */
        void seed(unsigned int seed_value) const;

        /**
         * @brief 生成一个随机数
         *
         * @return 生成的随机数
         */
        unsigned int getRandomNumber() const;

        /**
         * @brief 生成一个UUID字符串
         *
         * @param use_uppercase 是否使用大写字母
         * @param use_braces 是否使用括号
         * @param use_hyphens 是否使用短横线
         * @return 生成的UUID字符串
         */
        std::string generateUUID(bool use_uppercase = false, bool use_braces = false, bool use_hyphens = true) const;

        /**
         * @brief 重载输出运算符，用于将UUIDGenerator对象输出到流中
         *
         * @param os 输出流对象
         * @param uuid_gen UUIDGenerator对象
         * @return 输出流对象
         */
        friend std::ostream &operator<<(std::ostream &os, const UUIDGenerator &uuid_gen)
        {
            os << uuid_gen.generateUUID();
            return os;
        }
    };
} // namespace Atom::Utils

#endif
