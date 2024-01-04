/*
 * math.hpp
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

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#ifndef FRACTION_H
#define FRACTION_H

#include <sstream>
#include <vector>
#include "exception.hpp"

namespace Atom::Utils
{
    class Fraction;
    // 运算符，友元函数声明
    Fraction operator+(const Fraction &f1, const Fraction &f2);

    Fraction operator-(const Fraction &f1, const Fraction &f2);

    Fraction operator*(const Fraction &f1, const Fraction &f2);

    Fraction operator/(const Fraction &f1, const Fraction &f2);
    // 接收两个运算算子的引用，返回一个Fraction对象

    bool operator==(const Fraction &f1, const Fraction &f2);

    bool operator!=(const Fraction &f1, const Fraction &f2);

    bool operator>(const Fraction &f1, const Fraction &f2);
    bool operator>=(const Fraction &f1, const Fraction &f2);
    bool operator<(const Fraction &f1, const Fraction &f2);
    bool operator<=(const Fraction &f1, const Fraction &f2);

    std::istream &operator>>(std::istream &input, Fraction &f);
    std::ostream &operator<<(std::ostream &output, const Fraction &f);
    std::ostream &operator<<(std::ostream &output, const Fraction &&f);

    class Fraction
    {
    private:
        int numerator;   // 分子
        int denominator; // 分母
    public:
        static int Euclid(int a, int b)
        {
            // 计算两个数的最大公因数(欧几里得算法)
            // 声明为静态成员函数
            int c;
            while (a % b != 0)
            {
                c = b;
                b = a % b;
                a = c;
            }
            return b;
        };
        // 保留隐式转换，减少编程工作
        Fraction(int num_value, int den_value) : numerator(num_value), denominator(den_value)
        {
            if (this->denominator == 0)
            {
                throw Exception::WrongArgument_Error("Got 0 in the denominator of Math::Fraction object!");
            }
        };
        Fraction(int num_value) : numerator(num_value), denominator(1){};
        Fraction(const char *str)
        {
            this->denominator = 1;
            std::stringstream inistream;
            inistream << str;
            inistream >> (*this);
        }
        Fraction() : numerator(0), denominator(1){};
        Fraction(const Fraction &f) : numerator(f.getNumerator()), denominator(f.getDenominator()){};
        Fraction(Fraction &&f) : numerator(f.numerator), denominator(f.denominator){};

        inline int getNumerator() const
        {
            return numerator;
        }
        // 申明为不可修改数据的函数，const常函数
        inline int getDenominator() const
        {
            return denominator;
        }

        inline void alterValue(int num_value, int den_value)
        {
            this->numerator = num_value;
            this->denominator = den_value;
        }

        inline void alterValue(const Fraction &f)
        {
            this->numerator = f.getNumerator();
            this->denominator = f.getDenominator();
        }

        inline Fraction inverse()
        {
            // 返回这个分数的倒数
            return Fraction(this->getDenominator(), this->getNumerator());
        }

        friend Fraction operator+(const Fraction &f1, const Fraction &f2);

        friend Fraction operator-(const Fraction &f1, const Fraction &f2);

        friend Fraction operator*(const Fraction &f1, const Fraction &f2);

        friend Fraction operator/(const Fraction &f1, const Fraction &f2);

        // 负号运算符，将一个分数变成其相反数
        inline Fraction operator-()
        {
            return Fraction(-this->numerator, this->denominator);
        }
        Fraction &operator+=(const Fraction &f);

        Fraction &operator-=(const Fraction &f);

        Fraction &operator*=(const Fraction &f);

        Fraction &operator/=(const Fraction &f);

        Fraction &operator=(const Fraction &f);

        Fraction &operator=(const Fraction &&f);

        friend bool operator==(const Fraction &f1, const Fraction &f2);
        friend bool operator!=(const Fraction &f1, const Fraction &f2);
        friend bool operator>(const Fraction &f1, const Fraction &f2);
        friend bool operator>=(const Fraction &f1, const Fraction &f2);
        friend bool operator<(const Fraction &f1, const Fraction &f2);
        friend bool operator<=(const Fraction &f1, const Fraction &f2);
        friend std::istream &operator>>(std::istream &input, Fraction &f);
        friend std::ostream &operator<<(std::ostream &output, const Fraction &f);
        friend std::ostream &operator<<(std::ostream &output, const Fraction &&f);
    };

    [[maybe_unused]] std::vector<std::vector<double>> convolve2D(const std::vector<std::vector<double>> &input, const std::vector<std::vector<double>> &kernel, int numThreads);
    [[maybe_unused]] std::vector<std::vector<double>> deconvolve2D(const std::vector<std::vector<double>> &signal, const std::vector<std::vector<double>> &kernel);
}

#endif // FRACTION_H
