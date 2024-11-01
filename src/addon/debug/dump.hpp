#ifndef COREDUMPANALYZER_H
#define COREDUMPANALYZER_H

#include <memory>
#include <string>

namespace lithium::addon {

/**
 * @brief Analyzes core dump files.
 *
 * The CoreDumpAnalyzer class provides functionality to read and analyze core
 * dump files.
 */
class CoreDumpAnalyzer {
public:
    /**
     * @brief Constructs a CoreDumpAnalyzer object.
     *
     * Initializes the CoreDumpAnalyzer instance.
     */
    CoreDumpAnalyzer();

    /**
     * @brief Destructs the CoreDumpAnalyzer object.
     *
     * Cleans up resources used by the CoreDumpAnalyzer instance.
     */
    ~CoreDumpAnalyzer();

    /**
     * @brief Reads a core dump file.
     *
     * This function reads the specified core dump file and prepares it for
     * analysis.
     *
     * @param filename The path to the core dump file.
     * @return True if the file was successfully read, false otherwise.
     */
    bool readFile(const std::string& filename);

    /**
     * @brief Analyzes the core dump file.
     *
     * This function performs analysis on the previously read core dump file.
     */
    void analyze();

private:
    /**
     * @brief Implementation class for CoreDumpAnalyzer.
     *
     * This class hides the implementation details of the CoreDumpAnalyzer.
     */
    class Impl;

    /**
     * @brief Pointer to the implementation of CoreDumpAnalyzer.
     *
     * This unique pointer manages the lifetime of the implementation object.
     */
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium::addon

#endif  // COREDUMPANALYZER_H
