/*
 * math.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Extra Math Library

**************************************************/

#include "math.hpp"

#include <complex>
#include <thread>

namespace Atom::Utils
{
    /*			"+",加法重载部分					*/
    // 友元函数是可以通过成员访问运算符访问私有成员的
    Fraction operator+(const Fraction &f1, const Fraction &f2)
    {
        //	如果声明中加了const，定义中没有加const，可能会出现无权访问私有成员的报错
        // 加法友元重载函数定义
        int retnum = f1.getNumerator() * f2.getDenominator() + f2.getNumerator() * f1.getDenominator();
        int retden = f1.getDenominator() * f2.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (retden < 0 && retnum >= 0)
        {
            ratio *= -1;
        }
        return Fraction(retnum / ratio, retden / ratio); // 在使用的时候编译器会自动调用构造函数Fraction(const Fraction&),用来处理作用域问题（深复制了）
    }

    /*			"-",减法重载部分					*/
    Fraction operator-(const Fraction &f1, const Fraction &f2)
    {
        // 减法友元重载函数定义
        int retnum = f1.getNumerator() * f2.getDenominator() - f2.getNumerator() * f1.getDenominator();
        int retden = f1.getDenominator() * f2.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (retden < 0 && retnum >= 0)
        {
            ratio *= -1;
        }
        return Fraction(retnum / ratio, retden / ratio); // 在使用的时候编译器会自动调用Fraction默认构造函数,用来处理作用域问题（深复制了）
    }

    /*			"*",乘法重载部分					*/
    Fraction operator*(const Fraction &f1, const Fraction &f2)
    {
        // 加法友元重载函数定义
        int retnum = f1.getNumerator() * f2.getNumerator();
        int retden = f1.getDenominator() * f2.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (retden < 0 && retnum >= 0)
        {
            ratio *= -1;
        }
        return Fraction(retnum / ratio, retden / ratio);
    }

    /*          '/'乘法重载                 */
    Fraction operator/(const Fraction &f1, const Fraction &f2)
    {
        // 减法友元重载函数定义
        int retnum = f1.getNumerator() * f2.getDenominator();
        int retden = f1.getDenominator() * f2.getNumerator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (retden < 0 && retnum >= 0)
        {
            ratio *= -1;
        }
        return Fraction(retnum / ratio, retden / ratio);
    }

    // "+="运算符作为成员函数进行重载
    Fraction &Fraction::operator+=(const Fraction &f)
    {
        int retnum = this->numerator * f.getDenominator() + this->denominator * f.getNumerator();
        int retden = this->denominator * f.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        // 保证分母永远大于0
        if (this->denominator < 0 && this->numerator >= 0)
        {
            ratio *= -1;
        }
        this->numerator = retnum / ratio;
        this->denominator = retden / ratio;
        return (*this);
    }

    // "-="运算符作为成员函数进行重载

    Fraction &Fraction::operator-=(const Fraction &f)
    {
        int retnum = this->numerator * f.getDenominator() - this->denominator * f.getNumerator();
        int retden = this->denominator * f.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (this->denominator < 0 && this->numerator >= 0)
        {
            ratio *= -1;
        }
        this->numerator = retnum / ratio;
        this->denominator = retden / ratio;
        return (*this);
    }

    // "*="运算符作为成员函数进行重载
    Fraction &Fraction::operator*=(const Fraction &f)
    {
        int retnum = this->numerator * f.getNumerator();
        int retden = this->denominator * f.getDenominator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (this->denominator < 0 && this->numerator >= 0)
        {
            ratio *= -1;
        }
        this->numerator = retnum / ratio;
        this->denominator = retden / ratio;
        return *this;
    }
    Fraction &Fraction::operator/=(const Fraction &f)
    {
        int retnum = this->numerator * f.getDenominator();
        int retden = this->denominator * f.getNumerator();
        int ratio = Fraction::Euclid(retnum, retden);
        if (this->denominator < 0 && this->numerator >= 0)
        {
            ratio *= -1;
        }
        this->numerator = retnum / ratio;
        this->denominator = retden / ratio;

        return *this;
    }
    Fraction &Fraction::operator=(const Fraction &f)
    {
        this->numerator = f.getNumerator();
        this->denominator = f.getDenominator();
        return *this;
    }

    Fraction &Fraction::operator=(const Fraction &&f)
    {
        this->numerator = f.getNumerator();
        this->denominator = f.getDenominator();
        return *this;
    }

    bool operator==(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num == res_den)
        {
            result = true;
        }
        return result;
    }

    bool operator!=(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num != res_den)
        {
            result = true;
        }
        return result;
    }

    bool operator>(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num > res_den)
        {
            result = true;
        }
        return result;
    }

    bool operator>=(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num >= res_den)
        {
            result = true;
        }
        return result;
    }

    bool operator<(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num < res_den)
        {
            result = true;
        }
        return result;
    }

    bool operator<=(const Fraction &f1, const Fraction &f2)
    {
        int f1_num = f1.getNumerator();
        int f1_den = f1.getDenominator();
        int f2_num = f2.getNumerator();
        int f2_den = f2.getDenominator();
        int res_num = f1_num * f2_den;
        int res_den = f1_den * f2_num;
        bool result = false;
        if (res_num <= res_den)
        {
            result = true;
        }
        return result;
    }

    std::istream &operator>>(std::istream &input, Fraction &f)
    {
        input >> f.numerator;
        char split_c = input.peek();

        if (split_c > '9' || split_c < '0')
        {
            if (split_c == '/')
            {
                input >> split_c;
                input >> f.denominator;
                if (f.denominator == 0)
                {
                    throw Exception::WrongArgument_Error("Got 0 in the denominator of Math::Fraction object!");
                }
                if (f.denominator < 0)
                {
                    f.denominator *= -1;
                    f.numerator *= -1;
                }
                return input;
            }
        }
        f.denominator = 1;
        return input;
    }
    std::ostream &operator<<(std::ostream &output, const Fraction &f)
    {
        output << f.getNumerator() << "/" << f.getDenominator();
        return output;
    }
    std::ostream &operator<<(std::ostream &output, const Fraction &&f)
    {
        output << f.getNumerator() << "/" << f.getDenominator();
        return output;
    }
}
