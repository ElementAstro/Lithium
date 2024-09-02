#include "atom/algorithm/hash.hpp"

#include <iostream>

int main() {
    {
        int number = 42;
        std::size_t numberHash = atom::algorithm::computeHash(number);

        std::cout << "Hash of integer 42: " << numberHash << std::endl;
    }

    {
        std::string text = "Hello, World!";
        std::size_t textHash = atom::algorithm::computeHash(text);

        std::cout << "Hash of string \"Hello, World!\": " << textHash
                  << std::endl;
    }

    {
        std::vector<int> values = {1, 2, 3, 4, 5};
        std::size_t vectorHash = atom::algorithm::computeHash(values);

        std::cout << "Hash of vector {1, 2, 3, 4, 5}: " << vectorHash
                  << std::endl;
    }

    {
        auto myTuple = std::make_tuple(1, 2.5, "text");
        std::size_t tupleHash = atom::algorithm::computeHash(myTuple);

        std::cout << "Hash of tuple (1, 2.5, \"text\"): " << tupleHash
                  << std::endl;
    }

    {
        std::array<int, 3> myArray = {10, 20, 30};
        std::size_t arrayHash = atom::algorithm::computeHash(myArray);

        std::cout << "Hash of array {10, 20, 30}: " << arrayHash << std::endl;
    }

    {
        const char* cstr = "example";
        unsigned int hashValue = hash(cstr);

        std::cout << "Hash of C-string \"example\": " << hashValue << std::endl;
    }

    {
        constexpr unsigned int literalHash = "example"_hash;

        std::cout << "Hash of string literal \"example\": " << literalHash
                  << std::endl;
    }

    return 0;
}
