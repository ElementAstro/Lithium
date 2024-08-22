#include "atom/algorithm/fraction.hpp"

#include <iostream>
#include <sstream>

int main() {
    {
        // Default constructor
        atom::algorithm::Fraction f1;  // Represents 0/1

        // Parameterized constructor
        atom::algorithm::Fraction f2(3, 4);  // Represents 3/4

        // Printing fractions
        std::cout << "Fraction f1: " << f1.toString()
                  << std::endl;  // Output: "0/1"
        std::cout << "Fraction f2: " << f2.toString()
                  << std::endl;  // Output: "3/4"
    }

    {
        atom::algorithm::Fraction f1(1, 2);  // Represents 1/2
        atom::algorithm::Fraction f2(3, 4);  // Represents 3/4

        // Addition
        auto resultAdd = f1 + f2;  // 1/2 + 3/4 = 5/4
        std::cout << "Addition result: " << resultAdd.toString()
                  << std::endl;  // Output: "5/4"

        // Subtraction
        auto resultSub = f1 - f2;  // 1/2 - 3/4 = -1/4
        std::cout << "Subtraction result: " << resultSub.toString()
                  << std::endl;  // Output: "-1/4"

        // Multiplication
        auto resultMul = f1 * f2;  // 1/2 * 3/4 = 3/8
        std::cout << "Multiplication result: " << resultMul.toString()
                  << std::endl;  // Output: "3/8"

        // Division
        auto resultDiv = f1 / f2;  // 1/2 / 3/4 = 2/3
        std::cout << "Division result: " << resultDiv.toString()
                  << std::endl;  // Output: "2/3"
    }

    {
        atom::algorithm::Fraction f1(1, 2);  // Represents 1/2
        atom::algorithm::Fraction f2(3, 4);  // Represents 3/4

        f1 += f2;  // f1 now represents 5/4
        std::cout << "After addition assignment: " << f1.toString()
                  << std::endl;  // Output: "5/4"

        f1 -= f2;  // f1 now represents 1/2
        std::cout << "After subtraction assignment: " << f1.toString()
                  << std::endl;  // Output: "1/2"

        f1 *= f2;  // f1 now represents 3/8
        std::cout << "After multiplication assignment: " << f1.toString()
                  << std::endl;  // Output: "3/8"

        f1 /= f2;  // f1 now represents 1/2
        std::cout << "After division assignment: " << f1.toString()
                  << std::endl;  // Output: "1/2"
    }

    {
        atom::algorithm::Fraction f(3, 4);  // Represents 3/4

        double d = static_cast<double>(f);  // Converts to double
        std::cout << "Fraction as double: " << d << std::endl;  // Output: 0.75

        float fl = static_cast<float>(f);  // Converts to float
        std::cout << "Fraction as float: " << fl << std::endl;  // Output: 0.75

        int i = static_cast<int>(f);  // Converts to int (truncates to 0)
        std::cout << "Fraction as int: " << i << std::endl;  // Output: 0
    }

    {
        // Output to stream
        atom::algorithm::Fraction f(5, 6);  // Represents 5/6
        std::ostringstream oss;
        oss << f;
        std::cout << "Fraction as stream output: " << oss.str()
                  << std::endl;  // Output: "5/6"

        // Input from stream
        atom::algorithm::Fraction fInput;
        std::istringstream iss("7 8");  // Represents 7/8
        iss >> fInput;
        std::cout << "Fraction after input: " << fInput.toString()
                  << std::endl;  // Output: "7/8"
    }

    return 0;
}
