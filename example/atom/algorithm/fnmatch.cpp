#include "atom/algorithm/fnmatch.hpp"

#include <iostream>

int main() {
    {
        std::string pattern = "*.cpp";
        std::string filename = "main.cpp";

        bool match = atom::algorithm::fnmatch(pattern, filename);
        if (match) {
            std::cout << filename << " matches the pattern " << pattern
                      << std::endl;
        } else {
            std::cout << filename << " does not match the pattern " << pattern
                      << std::endl;
        }
    }

    {
        std::vector<std::string> filenames = {"main.cpp", "README.md",
                                              "fnmatch.hpp"};
        std::string pattern = "*.hpp";

        [[maybe_unused]] auto matches =
            atom::algorithm::filter(filenames, pattern);

        std::cout << "Files matching pattern:\n";
    }

    {
        std::vector<std::string> filenames = {"main.cpp", "README.md",
                                              "fnmatch.hpp", "CMakeLists.txt"};
        std::vector<std::string> patterns = {"*.cpp", "*.hpp"};

        std::vector<std::string> matches =
            atom::algorithm::filter(filenames, patterns);

        std::cout << "Files matching patterns:\n";
        for (const auto& file : matches) {
            std::cout << file << std::endl;
        }
    }

    return 0;
}
