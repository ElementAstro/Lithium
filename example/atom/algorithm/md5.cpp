#include "atom/algorithm/md5.hpp"

#include <iostream>

int main() {
    {
        // Example strings to hash
        std::string test1 = "Hello, World!";
        std::string test2 = "The quick brown fox jumps over the lazy dog";
        std::string test3 = "MD5 Hash Example";

        // Call the encrypt method and output the result
        std::string hash1 = atom::algorithm::MD5::encrypt(test1);
        std::string hash2 = atom::algorithm::MD5::encrypt(test2);
        std::string hash3 = atom::algorithm::MD5::encrypt(test3);

        // Output the results
        std::cout << "MD5(\"" << test1 << "\") = " << hash1 << std::endl;
        std::cout << "MD5(\"" << test2 << "\") = " << hash2 << std::endl;
        std::cout << "MD5(\"" << test3 << "\") = " << hash3 << std::endl;
    }

    return 0;
}
