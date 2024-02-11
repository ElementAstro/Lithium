#include "atom/experiment/optional.hpp"

int main()
{
    Optional<int> opt1;
    Optional<int> opt2 = 42;
    Optional<int> opt3 = opt2;

    std::cout << opt1.value_or(0) << std::endl; // Output: 0
    std::cout << opt2.value_or(0) << std::endl; // Output: 42
    std::cout << opt3.value_or(0) << std::endl; // Output: 42

    opt3.reset();
    std::cout << std::boolalpha << static_cast<bool>(opt3) << std::endl; // Output: false

    return 0;
}