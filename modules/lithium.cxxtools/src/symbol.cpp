#include "symbol.hpp"

#include <array>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "yaml-cpp/yaml.h"

using json = nlohmann::json;

constexpr size_t BUFFER_SIZE = 256;
constexpr int MATCH_SIZE = 6;
constexpr int MATCH_INDEX = 5;

// Execute a system command and return its output as a string
auto exec(const std::string& cmd) -> std::string {
    LOG_F(INFO, "Executing command: {}", cmd);
    std::array<char, BUFFER_SIZE> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);

    if (!pipe) {
        LOG_F(ERROR, "popen() failed for command: {}", cmd);
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    LOG_F(INFO, "Command output: {}", result);
    return result;
}

// Parse readelf output and extract symbols
auto parseReadelfOutput(const std::string_view output) -> std::vector<Symbol> {
    std::vector<Symbol> symbols;
    const std::regex SYMBOL_REGEX(
        R"(\s*\d+:\s+(\S+)\s+\d+\s+(\S+)\s+(\S+)\s+(\S+)\s+\S+\s+(\S+))");
    std::string outputStr(output);
    std::sregex_iterator begin(outputStr.begin(), outputStr.end(),
                               SYMBOL_REGEX);
    auto end = std::sregex_iterator();

    for (std::sregex_iterator it = begin; it != end; ++it) {
        const auto& match = *it;
        if (match.size() == MATCH_SIZE) {
            symbols.emplace_back(Symbol{
                match[1].str(), match[2].str(), match[3].str(), match[4].str(),
                match[MATCH_INDEX].str(),
                ""  // Demangled name will be filled later
            });
            LOG_F(INFO,
                  "Parsed Symbol: Address={}, Type={}, Bind={}, Visibility={}, "
                  "Name={}",
                  match[1].str(), match[2].str(), match[3].str(),
                  match[4].str(), match[MATCH_INDEX].str());
        }
    }

    LOG_F(INFO, "Total symbols parsed: {}", symbols.size());
    return symbols;
}

// Multithreaded parsing of symbols
auto parseSymbolsInParallel(const std::string& output,
                            int threadCount) -> std::vector<Symbol> {
    LOG_F(INFO, "Starting parallel parsing with {} threads", threadCount);
    std::vector<std::future<std::vector<Symbol>>> futures;
    std::vector<Symbol> resultSymbols;

    // Split the output into lines
    std::vector<std::string> lines;
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line)) {
        lines.emplace_back(line);
    }

    size_t totalLines = lines.size();
    size_t chunkSize = (totalLines + threadCount - 1) / threadCount;

    for (int i = 0; i < threadCount; ++i) {
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, totalLines);

        futures.emplace_back(std::async(
            std::launch::async, [start, end, &lines]() -> std::vector<Symbol> {
                std::string chunk;
                for (size_t j = start; j < end; ++j) {
                    chunk += lines[j] + "\n";
                }
                return parseReadelfOutput(chunk);
            }));
    }

    for (auto& future : futures) {
        auto symbols = future.get();
        resultSymbols.insert(resultSymbols.end(),
                             std::make_move_iterator(symbols.begin()),
                             std::make_move_iterator(symbols.end()));
    }

    LOG_F(INFO, "Completed parallel parsing. Total symbols collected: {}",
          resultSymbols.size());
    return resultSymbols;
}

// Filter symbols by type
auto filterSymbolsByType(const std::vector<Symbol>& symbols,
                         const std::string& type) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    std::copy_if(symbols.begin(), symbols.end(), std::back_inserter(filtered),
                 [&](const Symbol& symbol) { return symbol.type == type; });
    LOG_F(INFO, "Filtered symbols by type '{}': {} symbols found", type,
          filtered.size());
    return filtered;
}

// Filter symbols by visibility
auto filterSymbolsByVisibility(const std::vector<Symbol>& symbols,
                               const std::string& visibility)
    -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    std::copy_if(
        symbols.begin(), symbols.end(), std::back_inserter(filtered),
        [&](const Symbol& symbol) { return symbol.visibility == visibility; });
    LOG_F(INFO, "Filtered symbols by visibility '{}': {} symbols found",
          visibility, filtered.size());
    return filtered;
}

// Filter symbols by bind
auto filterSymbolsByBind(const std::vector<Symbol>& symbols,
                         const std::string& bind) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    std::copy_if(symbols.begin(), symbols.end(), std::back_inserter(filtered),
                 [&](const Symbol& symbol) { return symbol.bind == bind; });
    LOG_F(INFO, "Filtered symbols by bind '{}': {} symbols found", bind,
          filtered.size());
    return filtered;
}

// Generic filter function
auto filterSymbolsByCondition(const std::vector<Symbol>& symbols,
                              const std::function<bool(const Symbol&)>&
                                  condition) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    std::copy_if(symbols.begin(), symbols.end(), std::back_inserter(filtered),
                 condition);
    LOG_F(INFO, "Filtered symbols by custom condition: {} symbols found",
          filtered.size());
    return filtered;
}

// Print symbol statistics
void printSymbolStatistics(const std::vector<Symbol>& symbols) {
    std::unordered_map<std::string, size_t> typeCount;
    for (const auto& symbol : symbols) {
        typeCount[symbol.type]++;
    }

    LOG_F(INFO, "Symbol type statistics:");
    for (const auto& [type, count] : typeCount) {
        LOG_F(INFO, "Type '{}' : {} occurrences", type, count);
    }
}

// Export symbols to CSV file
void exportSymbolsToFile(const std::vector<Symbol>& symbols,
                         const std::string& filename) {
    LOG_F(INFO, "Exporting symbols to CSV file: {}", filename);
    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open CSV file for writing: {}", filename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open file for writing");
    }

    file << "Address,Type,Bind,Visibility,Name,Demangled Name\n";
    for (const auto& symbol : symbols) {
        file << '"' << symbol.address << "\"," << '"' << symbol.type << "\","
             << '"' << symbol.bind << "\"," << '"' << symbol.visibility << "\","
             << '"' << symbol.name << "\"," << '"' << symbol.demangledName
             << "\"\n";
    }

    LOG_F(INFO, "Successfully exported symbols to CSV file: {}", filename);
}

// Export symbols to JSON file
void exportSymbolsToJson(const std::vector<Symbol>& symbols,
                         const std::string& filename) {
    LOG_F(INFO, "Exporting symbols to JSON file: {}", filename);
    json j;
    for (const auto& symbol : symbols) {
        j.push_back({{"address", symbol.address},
                     {"type", symbol.type},
                     {"bind", symbol.bind},
                     {"visibility", symbol.visibility},
                     {"name", symbol.name},
                     {"demangled_name", symbol.demangledName}});
    }

    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open JSON file for writing: {}", filename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open JSON file for writing");
    }

    file << j.dump(4);  // Pretty print with 4 spaces
    LOG_F(INFO, "Successfully exported symbols to JSON file: {}", filename);
}

// Export symbols to YAML file
void exportSymbolsToYaml(const std::vector<Symbol>& symbols,
                         const std::string& filename) {
    LOG_F(INFO, "Exporting symbols to YAML file: {}", filename);
    YAML::Emitter out;
    out << YAML::BeginSeq;
    for (const auto& symbol : symbols) {
        out << YAML::BeginMap;
        out << YAML::Key << "address" << YAML::Value << symbol.address;
        out << YAML::Key << "type" << YAML::Value << symbol.type;
        out << YAML::Key << "bind" << YAML::Value << symbol.bind;
        out << YAML::Key << "visibility" << YAML::Value << symbol.visibility;
        out << YAML::Key << "name" << YAML::Value << symbol.name;
        out << YAML::Key << "demangled_name" << YAML::Value
            << symbol.demangledName;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    std::ofstream file(filename);
    if (!file) {
        LOG_F(ERROR, "Failed to open YAML file for writing: {}", filename);
        THROW_FAIL_TO_OPEN_FILE("Failed to open YAML file for writing");
    }
    file << out.c_str();
    LOG_F(INFO, "Successfully exported symbols to YAML file: {}", filename);
}

// Analyze the library and export symbols
void analyzeLibrary(const std::string& libraryPath,
                    const std::string& outputFormat, int threadCount) {
    LOG_F(INFO, "Starting analysis of library: {}", libraryPath);

    // Verify that the library file exists
    if (!std::filesystem::exists(libraryPath)) {
        LOG_F(ERROR, "Library file does not exist: {}", libraryPath);
        THROW_INVALID_ARGUMENT("Library file does not exist");
    }

    // Use readelf to get symbol information
    std::string readelfCmd = "readelf -Ws \"" + libraryPath + "\"";
    LOG_F(INFO, "Executing readelf command: {}", readelfCmd);

    std::string readelfOutput;
    try {
        readelfOutput = exec(readelfCmd);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to execute readelf: {}", e.what());
        throw;
    }

    // Parse readelf output in parallel
    std::vector<Symbol> symbols =
        parseSymbolsInParallel(readelfOutput, threadCount);

    // Demangle symbol names
    LOG_F(INFO, "Demangling symbol names...");
    for (auto& symbol : symbols) {
        try {
            symbol.demangledName =
                atom::meta::DemangleHelper::demangle(symbol.name);
        } catch (const std::exception& e) {
            LOG_F(WARNING, "Demangling failed for symbol '{}': {}", symbol.name,
                  e.what());
            symbol.demangledName = symbol.name;  // Fallback to original name
        }
    }
    LOG_F(INFO, "Completed demangling symbol names.");

    // Print symbol statistics
    printSymbolStatistics(symbols);

    // Export symbols to the specified format
    if (outputFormat == "csv") {
        exportSymbolsToFile(symbols, "symbols.csv");
    } else if (outputFormat == "json") {
        exportSymbolsToJson(symbols, "symbols.json");
    } else if (outputFormat == "yaml") {
        exportSymbolsToYaml(symbols, "symbols.yaml");
    } else {
        LOG_F(ERROR, "Unsupported output format: {}", outputFormat);
        THROW_INVALID_ARGUMENT("Unsupported output format");
    }

    LOG_F(INFO, "Library analysis completed successfully.");
}

auto main(int argc, char* argv[]) -> int {
    loguru::init(argc, argv);
    LOG_F(INFO, "Symbol Analyzer application started.");

    if (argc < 3 || argc > 4) {
        LOG_F(ERROR, "Invalid number of arguments.");
        LOG_F(ERROR,
              "Usage: {} <path_to_library> <output_format (csv/json/yaml)> "
              "[thread_count]",
              argv[0]);
        std::cerr << "Usage: " << argv[0]
                  << " <path_to_library> <output_format (csv/json/yaml)> "
                     "[thread_count]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::string libraryPath = argv[1];
    std::string outputFormat = argv[2];
    int threadCount = static_cast<int>(
        std::thread::hardware_concurrency());  // Default to system's thread
                                               // count

    if (argc == 4) {
        try {
            threadCount = std::stoi(argv[3]);
            if (threadCount <= 0) {
                LOG_F(ERROR, "Thread count must be a positive integer.");
                std::cerr << "Error: Thread count must be a positive integer."
                          << std::endl;
                return EXIT_FAILURE;
            }
            LOG_F(INFO, "Using user-specified thread count: {}", threadCount);
        } catch (const std::invalid_argument& e) {
            LOG_F(ERROR, "Invalid thread count provided: {}", argv[3]);
            std::cerr
                << "Error: Invalid thread count provided. Must be an integer."
                << std::endl;
            return EXIT_FAILURE;
        } catch (const std::out_of_range& e) {
            LOG_F(ERROR, "Thread count out of range: {}", argv[3]);
            std::cerr << "Error: Thread count out of range." << std::endl;
            return EXIT_FAILURE;
        }
    }

    LOG_F(INFO, "Library Path: {}", libraryPath);
    LOG_F(INFO, "Output Format: {}", outputFormat);
    LOG_F(INFO, "Thread Count: {}", threadCount);

    try {
        analyzeLibrary(libraryPath, outputFormat, threadCount);
    } catch (const atom::error::Exception& e) {
        LOG_F(ERROR, "Atom Exception: {}", e.what());
        std::cerr << "Atom Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Standard Exception: {}", e.what());
        std::cerr << "Standard Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        LOG_F(ERROR, "Unknown exception occurred.");
        std::cerr << "Error: Unknown exception occurred." << std::endl;
        return EXIT_FAILURE;
    }

    LOG_F(INFO, "Symbol Analyzer application terminated successfully.");
    return EXIT_SUCCESS;
}