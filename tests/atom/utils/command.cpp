#include "atom/utils/cmdline.hpp"

int main()
{
    std::string arg1 = "program";
    std::string arg2 = "-o";
    std::string arg3 = "output.txt";
    std::string arg4 = "-f";
    std::string arg5 = "input.txt";
    std::pair<std::string, int> arg6 = {"-n", 10};
    std::vector<std::string> arg7 = {"arg1", "arg2", "arg3"};

    std::string commandLine = joinCommandLine(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    std::cout << "Full command line: " << commandLine << std::endl;

    std::pair<std::string, bool> arg8 = {"--verbose", true};
    std::pair<std::string, double> arg9 = {"--threshold", 0.5};

    std::string commandLine2 = joinCommandLine(arg1, arg8, arg9);
    std::cout << "Full command line 2: " << commandLine2 << std::endl;

    std::map<std::string, std::string> arg10 = {
        {"-n", "10"},
        {"-f", "input.txt"},
        {"-o", "output.txt"},
        {"--verbose", "true"},
        {"--threshold", "0.5"}};
    std::cout << "Full command line 3: " << toString(arg10) << std::endl;

    return 0;
}
