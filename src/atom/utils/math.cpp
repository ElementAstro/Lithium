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

    std::vector<std::vector<double>> convolve2D(const std::vector<std::vector<double>> &input, const std::vector<std::vector<double>> &kernel, int numThreads = 1)
    {
        int inputRows = input.size();
        int inputCols = input[0].size();
        int kernelRows = kernel.size();
        int kernelCols = kernel[0].size();

        // 将输入矩阵和卷积核矩阵扩展到相同的大小，使用0填充
        std::vector<std::vector<double>> extendedInput(inputRows + kernelRows - 1, std::vector<double>(inputCols + kernelCols - 1, 0));
        std::vector<std::vector<double>> extendedKernel(inputRows + kernelRows - 1, std::vector<double>(inputCols + kernelCols - 1, 0));

        for (int i = 0; i < inputRows; ++i)
        {
            for (int j = 0; j < inputCols; ++j)
            {
                extendedInput[i + kernelRows / 2][j + kernelCols / 2] = input[i][j];
            }
        }

        for (int i = 0; i < kernelRows; ++i)
        {
            for (int j = 0; j < kernelCols; ++j)
            {
                extendedKernel[i][j] = kernel[i][j];
            }
        }

        // 计算卷积结果
        std::vector<std::vector<double>> output(inputRows, std::vector<double>(inputCols, 0));

        auto computeBlock = [&](int blockStartRow, int blockEndRow)
        {
            for (int i = blockStartRow; i < blockEndRow; ++i)
            {
                for (int j = kernelCols / 2; j < inputCols + kernelCols / 2; ++j)
                {
                    double sum = 0;
                    for (int k = -kernelRows / 2; k <= kernelRows / 2; ++k)
                    {
                        for (int l = -kernelCols / 2; l <= kernelCols / 2; ++l)
                        {
                            sum += extendedInput[i + k][j + l] * extendedKernel[kernelRows / 2 + k][kernelCols / 2 + l];
                        }
                    }
                    output[i - kernelRows / 2][j - kernelCols / 2] = sum;
                }
            }
        };

        if (numThreads == 1)
        {
            computeBlock(kernelRows / 2, inputRows + kernelRows / 2);
        }
        else
        {
            std::vector<std::thread> threads(numThreads);
            int blockSize = (inputRows + numThreads - 1) / numThreads;
            int blockStartRow = kernelRows / 2;
            for (int i = 0; i < numThreads; ++i)
            {
                int blockEndRow = std::min(blockStartRow + blockSize, inputRows + kernelRows / 2);
                threads[i] = std::thread(computeBlock, blockStartRow, blockEndRow);
                blockStartRow = blockEndRow;
            }
            for (auto &thread : threads)
            {
                thread.join();
            }
        }

        return output;
    }

    // 二维离散傅里叶变换（2D DFT）
    std::vector<std::vector<std::complex<double>>> DFT2D(const std::vector<std::vector<double>> &signal)
    {
        int M = signal.size();
        int N = signal[0].size();
        std::vector<std::vector<std::complex<double>>> X(M, std::vector<std::complex<double>>(N, {0, 0}));

        for (int u = 0; u < M; ++u)
        {
            for (int v = 0; v < N; ++v)
            {
                std::complex<double> sum(0, 0);
                for (int m = 0; m < M; ++m)
                {
                    for (int n = 0; n < N; ++n)
                    {
                        double theta = 2 * M_PI * (u * m / static_cast<double>(M) + v * n / static_cast<double>(N));
                        std::complex<double> w(cos(theta), -sin(theta));
                        sum += signal[m][n] * w;
                    }
                }
                X[u][v] = sum;
            }
        }

        return X;
    }

    // 二维离散傅里叶逆变换（2D IDFT）
    std::vector<std::vector<double>> IDFT2D(const std::vector<std::vector<std::complex<double>>> &spectrum)
    {
        int M = spectrum.size();
        int N = spectrum[0].size();
        std::vector<std::vector<double>> x(M, std::vector<double>(N, 0));

        for (int m = 0; m < M; ++m)
        {
            for (int n = 0; n < N; ++n)
            {
                std::complex<double> sum(0, 0);
                for (int u = 0; u < M; ++u)
                {
                    for (int v = 0; v < N; ++v)
                    {
                        double theta = 2 * M_PI * (u * m / static_cast<double>(M) + v * n / static_cast<double>(N));
                        std::complex<double> w(cos(theta), sin(theta));
                        sum += spectrum[u][v] * w;
                    }
                }
                x[m][n] = real(sum) / (M * N);
            }
        }

        return x;
    }

    // 二维反卷积函数
    std::vector<std::vector<double>> deconvolve2D(const std::vector<std::vector<double>> &signal, const std::vector<std::vector<double>> &kernel)
    {
        // 获取信号和卷积核的维度
        int M = signal.size();
        int N = signal[0].size();
        int K = kernel.size();
        int L = kernel[0].size();

        // 将信号和卷积核扩展到相同的大小，使用0填充
        std::vector<std::vector<double>> extendedSignal(M + K - 1, std::vector<double>(N + L - 1, 0));
        std::vector<std::vector<double>> extendedKernel(M + K - 1, std::vector<double>(N + L - 1, 0));

        // 将信号复制到扩展后的信号数组中
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                extendedSignal[i][j] = signal[i][j];
            }
        }

        // 将卷积核复制到扩展后的卷积核数组中
        for (int i = 0; i < K; ++i)
        {
            for (int j = 0; j < L; ++j)
            {
                extendedKernel[i][j] = kernel[i][j];
            }
        }

        // 计算信号和卷积核的二维DFT
        auto X = DFT2D(extendedSignal);
        auto H = DFT2D(extendedKernel);

        // 对卷积核的频谱进行修正
        std::vector<std::vector<std::complex<double>>> G(M, std::vector<std::complex<double>>(N, {0, 0}));
        double alpha = 0.1; // 防止分母为0
        for (int u = 0; u < M; ++u)
        {
            for (int v = 0; v < N; ++v)
            {
                if (std::abs(H[u][v]) > alpha)
                {
                    G[u][v] = std::conj(H[u][v]) / (std::norm(H[u][v]) + alpha);
                }
                else
                {
                    G[u][v] = std::conj(H[u][v]);
                }
            }
        }

        // 计算反卷积结果
        std::vector<std::vector<std::complex<double>>> Y(M, std::vector<std::complex<double>>(N, {0, 0}));
        for (int u = 0; u < M; ++u)
        {
            for (int v = 0; v < N; ++v)
            {
                Y[u][v] = G[u][v] * X[u][v];
            }
        }
        auto y = IDFT2D(Y);

        // 取出结果的前M行、前N列
        std::vector<std::vector<double>> result(M, std::vector<double>(N, 0));
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                result[i][j] = y[i][j];
            }
        }

        return result;
    }

}

/*
int main()
{
    int inputRows = 1000;
    int inputCols = 1000;
    int kernelRows = 3;
    int kernelCols = 3;

    std::vector<std::vector<double>> input(inputRows, std::vector<double>(inputCols, 1));
    std::vector<std::vector<double>> kernel(kernelRows, std::vector<double>(kernelCols, 1));

    auto output = convolve2D(input, kernel, 4);

    for (const auto &row : output)
    {
        for (auto val : row)
        {
            std::cout << val << " ";
        }
        std::cout << '\n';
    }

    return 0;
}

int main()
    {
        std::vector<std::vector<double>> signal = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}, {13, 14, 15}};
        std::vector<std::vector<double>> kernel = {{1, 0, 1}, {0, 1, 0}, {1, 0, 1}};

        auto result = deconvolve2D(signal, kernel);

        for (const auto &row : result)
        {
            for (auto val : row)
            {
                std::cout << val << " ";
            }
            std::cout << '\n';
        }

        return 0;
    }
*/
