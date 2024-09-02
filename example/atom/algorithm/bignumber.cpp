#include "atom/algorithm/bignumber.hpp"

#include <iostream>

int main() {
    {
        atom::algorithm::BigNumber num1("12345678901234567890");
        atom::algorithm::BigNumber num2(9876543210LL);

        std::cout << "num1: " << num1 << std::endl;
        std::cout << "num2: " << num2 << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("12345678901234567890");
        atom::algorithm::BigNumber num2("98765432109876543210");

        atom::algorithm::BigNumber sum = num1 + num2;
        atom::algorithm::BigNumber difference = num2 - num1;

        std::cout << "Sum: " << sum << std::endl;
        std::cout << "Difference: " << difference << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("123456789");
        atom::algorithm::BigNumber num2("1000");

        atom::algorithm::BigNumber product = num1 * num2;
        atom::algorithm::BigNumber quotient = num1 / num2;

        std::cout << "Product: " << product << std::endl;
        std::cout << "Quotient: " << quotient << std::endl;
    }

    {
        atom::algorithm::BigNumber base("2");

        atom::algorithm::BigNumber result = base ^ 10;

        std::cout << "2^10: " << result << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("123456789");
        atom::algorithm::BigNumber num2("123456789");
        atom::algorithm::BigNumber num3("987654321");

        std::cout << std::boolalpha;
        std::cout << "num1 == num2: " << (num1 == num2) << std::endl;
        std::cout << "num1 != num3: " << (num1 != num3) << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("123456789");
        atom::algorithm::BigNumber num2("987654321");

        std::cout << std::boolalpha;
        std::cout << "num1 < num2: " << (num1 < num2) << std::endl;
        std::cout << "num2 > num1: " << (num2 > num1) << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("123456789");

        atom::algorithm::BigNumber negated = num1.negate();

        std::cout << "Negated: " << negated << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("999");

        std::cout << "Before increment: " << num1 << std::endl;
        ++num1;
        std::cout << "After increment: " << num1 << std::endl;

        --num1;
        std::cout << "After decrement: " << num1 << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("123456789");
        atom::algorithm::BigNumber num2("123456788");

        std::cout << "num1 is odd: " << std::boolalpha << num1.isOdd()
                  << std::endl;
        std::cout << "num2 is even: " << std::boolalpha << num2.isEven()
                  << std::endl;
    }

    {
        atom::algorithm::BigNumber num1("0000123456789");

        std::cout << "Before trimming: " << num1 << std::endl;
        num1 = num1.trimLeadingZeros();
        std::cout << "After trimming: " << num1 << std::endl;
    }

    return 0;
}
