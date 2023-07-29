#include <iostream>
#include <fstream>
#include <filesystem>

#include <toml++/toml.hpp>
#include <loguru/loguru.hpp>

namespace fs = std::filesystem;

static const char *usage = R"EOF(
    Usage:
        toml2json infile.toml [outfile.json]

    If outfile is not specificed then the output will be written to stdout
    )EOF";

void ConvertTomlToJson(const std::string &inputFile, const std::string &outputFile)
{
    try
    {
        const fs::path infile{inputFile};

        if (!fs::exists(infile))
        {
            LOG_F(ERROR, "Input file %s does not exist!", infile.c_str());
            return;
        }

        auto data = toml::parse_file(infile.c_str());

        if (!outputFile.empty())
        {
            std::ofstream out{outputFile};
            if (!out)
            {
                LOG_F(ERROR, "Failed to open output file: %s", outputFile.c_str());
                return;
            }
            out << toml::json_formatter(data) << std::endl;
            LOG_F(INFO, "Conversion completed. Output saved to %s", outputFile.c_str());
        }
        else
        {
            std::cout << toml::json_formatter(data) << std::endl;
            LOG_F(INFO, "Conversion completed. Result printed to stdout");
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "An exception occurred during conversion: %s", e.what());
    }
}

int main(int argc, char **argv)
{
    loguru::init(argc, argv);
    loguru::add_file("toml2json.log", loguru::Append, loguru::Verbosity_INFO);

    if (argc < 2 || argc > 4)
    {
        LOG_F(ERROR, "Error: incorrect number of arguments, got %d, expected 1 or 2.", argc - 1);
        std::cout << usage << std::endl;
        return 1;
    }

    const std::string inputFile{argv[1]};
    std::string outputFile;

    if (argc >= 3)
    {
        outputFile = argv[2];
    }

    ConvertTomlToJson(inputFile, outputFile);

    loguru::remove_all_callbacks();
    return 0;
}
