/*
 * random.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-12-25

Description: Simple random number generator

**************************************************/

#pragma once

#include <random>
#include <string>

namespace Atom::Utils
{

    template <class Engine = std::default_random_engine,
              class Distribution = std::uniform_int_distribution<>>
    class random : public Distribution
    {
        using base_t = Distribution;

    public:
        using engine_type = Engine;
        using distribution_type = Distribution;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;

    private:
        engine_type engine_;

    public:
        template <typename... T>
        random(T &&...args)
            : base_t(std::forward<T>(args)...), engine_(std::random_device{}())
        {
        }

        result_type operator()()
        {
            return static_cast<base_t &>(*this)(engine_);
        }

        result_type operator()(const param_type &parm)
        {
            return static_cast<base_t &>(*this)(engine_, parm);
        }
    };

    [[nodiscard]] std::string generateRandomString(int length)
    {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

        std::string randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i)
        {
            randomString.push_back(characters[distribution(generator)]);
        }

        return randomString;
    }
}

/*
int main()
{
    // 使用默认构造函数生成随机数
    random<> rand1;
    int num1 = rand1();
    std::cout << "R1: " << num1 << std::endl;

    // 使用指定范围的均匀分布生成随机数
    random<> rand2(1, 10);
    int num2 = rand2();
    std::cout << "R2: " << num2 << std::endl;

    // 使用指定参数的泊松分布生成随机数
    random<std::default_random_engine, std::poisson_distribution<int>> rand3(5);
    int num3 = rand3();
    std::cout << "R3: " << num3 << std::endl;

    return 0;
}
*/
