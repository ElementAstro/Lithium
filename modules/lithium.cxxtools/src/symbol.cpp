#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "macro.hpp"

using json = nlohmann::json;

// 执行系统命令并返回输出
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// 符号结构体
struct Symbol {
    std::string address;
    std::string type;
    std::string bind;
    std::string visibility;
    std::string name;
    std::string demangledName;
} ATOM_ALIGNAS(128);

auto parseReadelfOutput(const std::string& output) -> std::vector<Symbol> {
    std::vector<Symbol> symbols;
    std::regex symbolRegex(
        R"(\s*\d+:\s+(\S+)\s+\d+\s+(\S+)\s+(\S+)\s+(\S+)\s+\S+\s+(\S+))");
    std::smatch match;
    std::string::const_iterator searchStart(output.cbegin());

    while (std::regex_search(searchStart, output.cend(), match, symbolRegex)) {
        symbols.push_back(
            Symbol{match[1], match[2], match[3], match[4], match[5], ""});
        searchStart = match.suffix().first;
    }

    return symbols;
}

auto filterSymbolsByType(const std::vector<Symbol>& symbols,
                         const std::string& type) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    for (const auto& symbol : symbols) {
        if (symbol.type == type) {
            filtered.push_back(symbol);
        }
    }
    return filtered;
}

auto filterSymbolsByVisibility(const std::vector<Symbol>& symbols,
                               const std::string& visibility)
    -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    for (const auto& symbol : symbols) {
        if (symbol.visibility == visibility) {
            filtered.push_back(symbol);
        }
    }
    return filtered;
}

auto filterSymbolsByBind(const std::vector<Symbol>& symbols,
                         const std::string& bind) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    for (const auto& symbol : symbols) {
        if (symbol.bind == bind) {
            filtered.push_back(symbol);
        }
    }
    return filtered;
}

void printSymbolStatistics(const std::vector<Symbol>& symbols) {
    std::unordered_map<std::string, size_t> typeCount;
    for (const auto& symbol : symbols) {
        typeCount[symbol.type]++;
    }
    LOG_F(INFO, "Symbol type statistics:");
    for (const auto& [type, count] : typeCount) {
        LOG_F(INFO, "{} : {}", type, count);
    }
}

void exportSymbolsToFile(const std::vector<Symbol>& symbols,
                         const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open file for writing");
    }

    file << "Address,Type,Bind,Visibility,Name,Demangled Name\n";
    for (const auto& symbol : symbols) {
        file << symbol.address << ',' << symbol.type << ',' << symbol.bind
             << ',' << symbol.visibility << ',' << symbol.name << ','
             << symbol.demangledName << '\n';
    }
}

void exportSymbolsToJson(const std::vector<Symbol>& symbols,
                         const std::string& filename) {
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
        THROW_FAIL_TO_OPEN_FILE("Failed to open JSON file for writing");
    }
    file << j.dump(4);  // Pretty print with 4 spaces
}

auto filterSymbolsByCondition(const std::vector<Symbol>& symbols,
                              const std::function<bool(const Symbol&)>&
                                  condition) -> std::vector<Symbol> {
    std::vector<Symbol> filtered;
    for (const auto& symbol : symbols) {
        if (condition(symbol)) {
            filtered.push_back(symbol);
        }
    }
    return filtered;
}

void analyzeLibrary(const std::string& libraryPath,
                    const std::string& outputFormat) {
    LOG_F(INFO, "Analyzing library: {}", libraryPath);

    // 使用 readelf 获取 ELF 特定信息
    std::string readelfCmd = "readelf -Ws " + libraryPath;
    std::string readelfOutput = exec(readelfCmd.c_str());
    LOG_F(INFO, "Readelf output: {}", readelfOutput);

    // 解析 readelf 输出
    std::vector<Symbol> symbols = parseReadelfOutput(readelfOutput);
    LOG_F(INFO, "Parsing readelf output...");

    // 解码符号名
    for (auto& symbol : symbols) {
        symbol.demangledName =
            atom::meta::DemangleHelper::demangle(symbol.name);
    }
    LOG_F(INFO, "Decoding symbol names...");

    // 符号统计
    printSymbolStatistics(symbols);

    // 导出符号到指定格式的文件
    if (outputFormat == "csv") {
        exportSymbolsToFile(symbols, "symbols.csv");
        LOG_F(INFO, "Exported symbols to symbols.csv.");
    } else if (outputFormat == "json") {
        exportSymbolsToJson(symbols, "symbols.json");
        LOG_F(INFO, "Exported symbols to symbols.json.");
    } else {
        LOG_F(ERROR, "Unsupported output format: {}", outputFormat);
        THROW_INVALID_ARGUMENT("Unsupported output format");
    }
}

auto main(int argc, char* argv[]) -> int {
    if (argc != 3) {
        LOG_F(ERROR, "Invalid number of arguments");
        LOG_F(ERROR, "Usage: {} <path_to_library> <output_format (csv/json)>");
        return 1;
    }

    std::string libraryPath = argv[1];
    std::string outputFormat = argv[2];

    try {
        analyzeLibrary(libraryPath, outputFormat);
    } catch (const std::exception& ex) {
        LOG_F(ERROR, "Error: {}", ex.what());
        return 1;
    }

    return 0;
}