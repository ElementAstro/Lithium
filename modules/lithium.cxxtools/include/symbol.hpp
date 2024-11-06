#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <functional>
#include <string>
#include <vector>

struct Symbol {
    std::string address;
    std::string type;
    std::string bind;
    std::string visibility;
    std::string name;
    std::string demangledName;
};

auto exec(const char* cmd) -> std::string;

auto parseReadelfOutput(const std::string& output) -> std::vector<Symbol>;

auto parseSymbolsInParallel(const std::string& output,
                            int threadCount) -> std::vector<Symbol>;

auto filterSymbolsByType(const std::vector<Symbol>& symbols,
                         const std::string& type) -> std::vector<Symbol>;

auto filterSymbolsByVisibility(const std::vector<Symbol>& symbols,
                               const std::string& visibility)
    -> std::vector<Symbol>;

auto filterSymbolsByBind(const std::vector<Symbol>& symbols,
                         const std::string& bind) -> std::vector<Symbol>;

void printSymbolStatistics(const std::vector<Symbol>& symbols);

void exportSymbolsToFile(const std::vector<Symbol>& symbols,
                         const std::string& filename);

void exportSymbolsToJson(const std::vector<Symbol>& symbols,
                         const std::string& filename);

void exportSymbolsToYaml(const std::vector<Symbol>& symbols,
                         const std::string& filename);

auto filterSymbolsByCondition(
    const std::vector<Symbol>& symbols,
    const std::function<bool(const Symbol&)>& condition) -> std::vector<Symbol>;

void analyzeLibrary(const std::string& libraryPath,
                    const std::string& outputFormat, int threadCount);

#endif  // SYMBOL_HPP
