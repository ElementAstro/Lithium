# Symbol Analyzer Documentation

## Overview

The `Symbol Analyzer` is a tool designed to analyze the symbols within a shared library or executable. It uses the `readelf` command to extract symbol information and provides functionalities to parse, filter, and export these symbols in various formats such as CSV, JSON, and YAML. The tool is designed to be highly efficient, leveraging multithreading for parsing large outputs.

## Dependencies

- **loguru**: A logging library used for logging operations.
- **atom/error/exception.hpp**: Custom exception handling.
- **atom/function/abi.hpp**: ABI-related functionalities.
- **atom/type/json.hpp**: JSON handling using `nlohmann::json`.
- **yaml-cpp/yaml.h**: YAML handling using `yaml-cpp`.

## Constants

- **BUFFER_SIZE**: Size of the buffer used for reading command output.
- **MATCH_SIZE**: Number of matches expected in the symbol regex.
- **MATCH_INDEX**: Index of the symbol name in the regex match.

## Functions

### `exec(const std::string& cmd) -> std::string`

Executes a system command and returns its output as a string.

- **Parameters:**

  - `cmd`: The command to execute.

- **Returns:** The output of the command as a string.

- **Example:**
  ```cpp
  std::string output = exec("ls -l");
  std::cout << "Command output: " << output << std::endl;
  ```

### `parseReadelfOutput(const std::string_view output) -> std::vector<Symbol>`

Parses the output of the `readelf` command and extracts symbols.

- **Parameters:**

  - `output`: The output of the `readelf` command.

- **Returns:** A vector of `Symbol` objects.

- **Example:**
  ```cpp
  std::string readelfOutput = exec("readelf -Ws /path/to/library");
  std::vector<Symbol> symbols = parseReadelfOutput(readelfOutput);
  ```

### `parseSymbolsInParallel(const std::string& output, int threadCount) -> std::vector<Symbol>`

Parses symbols in parallel using multiple threads.

- **Parameters:**

  - `output`: The output of the `readelf` command.
  - `threadCount`: The number of threads to use for parsing.

- **Returns:** A vector of `Symbol` objects.

- **Example:**
  ```cpp
  std::string readelfOutput = exec("readelf -Ws /path/to/library");
  std::vector<Symbol> symbols = parseSymbolsInParallel(readelfOutput, 4);
  ```

### `filterSymbolsByType(const std::vector<Symbol>& symbols, const std::string& type) -> std::vector<Symbol>`

Filters symbols by their type.

- **Parameters:**

  - `symbols`: The vector of symbols to filter.
  - `type`: The type of symbols to filter by.

- **Returns:** A vector of filtered `Symbol` objects.

- **Example:**
  ```cpp
  std::vector<Symbol> functionSymbols = filterSymbolsByType(symbols, "FUNC");
  ```

### `filterSymbolsByVisibility(const std::vector<Symbol>& symbols, const std::string& visibility) -> std::vector<Symbol>`

Filters symbols by their visibility.

- **Parameters:**

  - `symbols`: The vector of symbols to filter.
  - `visibility`: The visibility of symbols to filter by.

- **Returns:** A vector of filtered `Symbol` objects.

- **Example:**
  ```cpp
  std::vector<Symbol> globalSymbols = filterSymbolsByVisibility(symbols, "GLOBAL");
  ```

### `filterSymbolsByBind(const std::vector<Symbol>& symbols, const std::string& bind) -> std::vector<Symbol>`

Filters symbols by their bind type.

- **Parameters:**

  - `symbols`: The vector of symbols to filter.
  - `bind`: The bind type of symbols to filter by.

- **Returns:** A vector of filtered `Symbol` objects.

- **Example:**
  ```cpp
  std::vector<Symbol> weakSymbols = filterSymbolsByBind(symbols, "WEAK");
  ```

### `filterSymbolsByCondition(const std::vector<Symbol>& symbols, const std::function<bool(const Symbol&)>& condition) -> std::vector<Symbol>`

Filters symbols based on a custom condition.

- **Parameters:**

  - `symbols`: The vector of symbols to filter.
  - `condition`: A function that returns `true` for symbols that should be included.

- **Returns:** A vector of filtered `Symbol` objects.

- **Example:**
  ```cpp
  auto isGlobalFunction = [](const Symbol& symbol) {
      return symbol.type == "FUNC" && symbol.visibility == "GLOBAL";
  };
  std::vector<Symbol> globalFunctions = filterSymbolsByCondition(symbols, isGlobalFunction);
  ```

### `printSymbolStatistics(const std::vector<Symbol>& symbols)`

Prints statistics about the symbols, such as the count of each symbol type.

- **Parameters:**

  - `symbols`: The vector of symbols to analyze.

- **Example:**
  ```cpp
  printSymbolStatistics(symbols);
  ```

### `exportSymbolsToFile(const std::vector<Symbol>& symbols, const std::string& filename)`

Exports symbols to a CSV file.

- **Parameters:**

  - `symbols`: The vector of symbols to export.
  - `filename`: The name of the CSV file to create.

- **Example:**
  ```cpp
  exportSymbolsToFile(symbols, "symbols.csv");
  ```

### `exportSymbolsToJson(const std::vector<Symbol>& symbols, const std::string& filename)`

Exports symbols to a JSON file.

- **Parameters:**

  - `symbols`: The vector of symbols to export.
  - `filename`: The name of the JSON file to create.

- **Example:**
  ```cpp
  exportSymbolsToJson(symbols, "symbols.json");
  ```

### `exportSymbolsToYaml(const std::vector<Symbol>& symbols, const std::string& filename)`

Exports symbols to a YAML file.

- **Parameters:**

  - `symbols`: The vector of symbols to export.
  - `filename`: The name of the YAML file to create.

- **Example:**
  ```cpp
  exportSymbolsToYaml(symbols, "symbols.yaml");
  ```

### `analyzeLibrary(const std::string& libraryPath, const std::string& outputFormat, int threadCount)`

Analyzes the library and exports the symbols to the specified format.

- **Parameters:**

  - `libraryPath`: The path to the library file.
  - `outputFormat`: The format to export the symbols (csv, json, yaml).
  - `threadCount`: The number of threads to use for parsing.

- **Example:**
  ```cpp
  analyzeLibrary("/path/to/library", "json", 4);
  ```

### `main(int argc, char* argv[]) -> int`

The main function that initializes the application, parses command-line arguments, and starts the library analysis.

- **Parameters:**

  - `argc`: The number of command-line arguments.
  - `argv`: The array of command-line arguments.

- **Returns:** An integer representing the exit status.

- **Example:**
  ```bash
  ./symbol_analyzer /path/to/library json 4
  ```

## Usage Example

```cpp
#include "symbol.hpp"

int main(int argc, char* argv[]) {
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
```

## Notes

- The tool uses multithreading to improve performance, especially when dealing with large libraries.
- The `readelf` command is used to extract symbol information, which is then parsed and processed.
- Symbols can be filtered by type, visibility, bind, or a custom condition.
- The tool supports exporting symbols to CSV, JSON, and YAML formats.
- Logging is used extensively to provide detailed information about the operations being performed.
