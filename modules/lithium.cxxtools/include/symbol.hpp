#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <functional>
#include <string>
#include <vector>

#include "atom/macro.hpp"

/**
 * @brief Structure representing a symbol in a binary file.
 */
struct Symbol {
    std::string address;        ///< The address of the symbol.
    std::string type;           ///< The type of the symbol.
    std::string bind;           ///< The binding of the symbol.
    std::string visibility;     ///< The visibility of the symbol.
    std::string name;           ///< The name of the symbol.
    std::string demangledName;  ///< The demangled name of the symbol.
} ATOM_ALIGNAS(128);

/**
 * @brief Executes a system command and returns its output as a string.
 *
 * @param cmd The command to execute.
 * @return std::string The output of the command.
 */
auto exec(const char* cmd) -> std::string;

/**
 * @brief Parses the output of the readelf command and extracts symbols.
 *
 * @param output The output of the readelf command.
 * @return std::vector<Symbol> A vector of extracted symbols.
 */
auto parseReadelfOutput(const std::string& output) -> std::vector<Symbol>;

/**
 * @brief Parses symbols in parallel from the readelf output.
 *
 * @param output The output of the readelf command.
 * @param threadCount The number of threads to use for parsing.
 * @return std::vector<Symbol> A vector of parsed symbols.
 */
auto parseSymbolsInParallel(const std::string& output,
                            int threadCount) -> std::vector<Symbol>;

/**
 * @brief Filters symbols by type.
 *
 * @param symbols The vector of symbols to filter.
 * @param type The type to filter by.
 * @return std::vector<Symbol> A vector of symbols that match the specified
 * type.
 */
auto filterSymbolsByType(const std::vector<Symbol>& symbols,
                         const std::string& type) -> std::vector<Symbol>;

/**
 * @brief Filters symbols by visibility.
 *
 * @param symbols The vector of symbols to filter.
 * @param visibility The visibility to filter by.
 * @return std::vector<Symbol> A vector of symbols that match the specified
 * visibility.
 */
auto filterSymbolsByVisibility(const std::vector<Symbol>& symbols,
                               const std::string& visibility)
    -> std::vector<Symbol>;

/**
 * @brief Filters symbols by bind.
 *
 * @param symbols The vector of symbols to filter.
 * @param bind The bind to filter by.
 * @return std::vector<Symbol> A vector of symbols that match the specified
 * bind.
 */
auto filterSymbolsByBind(const std::vector<Symbol>& symbols,
                         const std::string& bind) -> std::vector<Symbol>;

/**
 * @brief Prints statistics about the types of symbols.
 *
 * @param symbols The vector of symbols to analyze.
 */
void printSymbolStatistics(const std::vector<Symbol>& symbols);

/**
 * @brief Exports symbols to a CSV file.
 *
 * @param symbols The vector of symbols to export.
 * @param filename The name of the CSV file to export to.
 */
void exportSymbolsToFile(const std::vector<Symbol>& symbols,
                         const std::string& filename);

/**
 * @brief Exports symbols to a JSON file.
 *
 * @param symbols The vector of symbols to export.
 * @param filename The name of the JSON file to export to.
 */
void exportSymbolsToJson(const std::vector<Symbol>& symbols,
                         const std::string& filename);

/**
 * @brief Exports symbols to a YAML file.
 *
 * @param symbols The vector of symbols to export.
 * @param filename The name of the YAML file to export to.
 */
void exportSymbolsToYaml(const std::vector<Symbol>& symbols,
                         const std::string& filename);

/**
 * @brief Filters symbols based on a custom condition.
 *
 * @param symbols The vector of symbols to filter.
 * @param condition The custom condition to filter by.
 * @return std::vector<Symbol> A vector of symbols that match the custom
 * condition.
 */
auto filterSymbolsByCondition(
    const std::vector<Symbol>& symbols,
    const std::function<bool(const Symbol&)>& condition) -> std::vector<Symbol>;

/**
 * @brief Analyzes a library and exports symbols in the specified format.
 *
 * @param libraryPath The path to the library to analyze.
 * @param outputFormat The format to export the symbols (csv, json, yaml).
 * @param threadCount The number of threads to use for parsing.
 */
void analyzeLibrary(const std::string& libraryPath,
                    const std::string& outputFormat, int threadCount);

/**
 * @brief Main function for the Symbol Analyzer application.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int The exit code of the application.
 */
auto main(int argc, char* argv[]) -> int;

#endif  // SYMBOL_HPP