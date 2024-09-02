#include "atom/algorithm/math.hpp"

#include <iostream>

int main() {
    {
        uint64_t a = 100000000000ULL;
        uint64_t b = 200000000000ULL;
        uint64_t result = atom::algorithm::safeAdd(a, b);

        std::cout << "Safe Addition Result: " << result << std::endl;
    }

    {
        uint64_t a = 300000000000ULL;
        uint64_t b = 100000000000ULL;
        uint64_t result = atom::algorithm::safeSub(a, b);

        std::cout << "Safe Subtraction Result: " << result << std::endl;
    }

    {
        uint64_t a = 300000ULL;
        uint64_t b = 100000ULL;
        uint64_t result = atom::algorithm::safeMul(a, b);

        std::cout << "Safe Multiplication Result: " << result << std::endl;
    }

    {
        uint64_t a = 100ULL;
        uint64_t b = 4ULL;
        uint64_t result = atom::algorithm::safeDiv(a, b);

        std::cout << "Safe Division Result: " << result << std::endl;
    }

    {
        uint64_t operant = 10;
        uint64_t multiplier = 20;
        uint64_t divider = 5;

        uint64_t result =
            atom::algorithm::mulDiv64(operant, multiplier, divider);

        std::cout << "Result of (10 * 20) / 5: " << result << std::endl;
    }

    {
        uint64_t n = 0x1234567890ABCDEF;
        unsigned int c = 8;  // Rotate left by 8 bits

        uint64_t result = atom::algorithm::rotl64(n, c);

        std::cout << "Rotate Left Result: " << std::hex << result << std::endl;
    }

    {
        uint64_t n = 0x1234567890ABCDEF;
        unsigned int c = 8;  // Rotate right by 8 bits

        uint64_t result = atom::algorithm::rotr64(n, c);

        std::cout << "Rotate Right Result: " << std::hex << result << std::endl;
    }

    {
        uint64_t x = 0x00F0;

        int leadingZeros = atom::algorithm::clz64(x);

        std::cout << "Leading Zeros in 0x00F0: " << leadingZeros << std::endl;
    }

    {
        uint64_t a = 48;
        uint64_t b = 180;

        uint64_t gcdResult = atom::algorithm::gcd64(a, b);
        uint64_t lcmResult = atom::algorithm::lcm64(a, b);

        std::cout << "GCD of 48 and 180: " << gcdResult << std::endl;
        std::cout << "LCM of 48 and 180: " << lcmResult << std::endl;
    }

    {
        uint64_t n = 16;  // Power of two
        bool result = atom::algorithm::isPowerOfTwo(n);

        std::cout << n << " is a power of two: " << (result ? "true" : "false")
                  << std::endl;
    }

    return 0;
}
