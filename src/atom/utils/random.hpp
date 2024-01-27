/*
 * random.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

    [[nodiscard]] std::string generateRandomString(int length);
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
