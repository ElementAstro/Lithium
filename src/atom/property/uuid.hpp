/*
 * uuid.hpp
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

Date: 2023-4-5

Description: UUID Generator

**************************************************/

#pragma once

#include <random>
#include <sstream>
#include <iomanip>

namespace Atom::Property
{
    /**
     * @class UUIDGenerator
     * @brief 用于生成 UUID 的类
     */
    class UUIDGenerator
    {
    public:
        /**
         * @brief 构造函数
         */
        UUIDGenerator();

        /**
         * @brief 设置随机数种子
         * @param seed_value 随机数种子值
         */
        void seed(unsigned int seed_value);

        /**
         * @brief 生成一个 UUID
         * @return 生成的 UUID 字符串
         */
        std::string generateUUID();

        /**
         * @brief 使用指定格式生成一个 UUID
         * @param use_braces 是否使用花括号包围
         * @param use_hyphens 是否使用连字符分隔
         * @return 生成的 UUID 字符串
         */
        static std::string generateUUIDWithFormat(bool use_braces = true, bool use_hyphens = true);

        static std::string generateEnhancedUUID();

        /**
         * @brief 生成一个随机数
         * @return 生成的随机数
         */
        static unsigned int getRandomNumber();

    private:
        std::mt19937 gen_;                       ///< 随机数生成器
        std::uniform_int_distribution<int> dis_; ///< 均匀分布器
    };
} // namespace Lithium::UUID
