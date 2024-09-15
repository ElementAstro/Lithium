#include "constant.hpp"

#ifdef _WIN32
std::vector<std::string> Constants::COMMON_COMPILERS = {"cl.exe", "g++.exe",
                                                        "clang++.exe"};
std::vector<std::string> Constants::COMPILER_PATHS = {
    "C:\\Program Files (x86)\\Microsoft Visual "
    "Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29."
    "30133\\bin\\Hostx64\\x64",
    "C:\\Program Files\\Microsoft Visual "
    "Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.35."
    "32215\\bin\\Hostx64\\x64",
    "C:\\msys64\\mingw64\\bin", "C:\\MinGW\\bin",
    "C:\\Program Files\\LLVM\\bin"};
#elif __APPLE__
std::vector<std::string> Constants::COMMON_COMPILERS = {"clang++", "g++"};
std::vector<std::string> Constants::COMPILER_PATHS = {
    "/usr/bin", "/usr/local/bin", "/opt/local/bin"};
#elif __linux__
std::vector<std::string> Constants::COMMON_COMPILERS = {"g++", "clang++"};
std::vector<std::string> Constants::COMPILER_PATHS = {"/usr/bin",
                                                      "/usr/local/bin"};
#endif
