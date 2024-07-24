/**
 * @file output_styles.hpp
 * @brief Defines various output styles for displaying data.
 */

#ifndef LITHIUM_DEBUG_OUTPUT_STYLES_HPP
#define LITHIUM_DEBUG_OUTPUT_STYLES_HPP

#include <memory>
#include <string>
#include <vector>

/**
 * @class OutputStyle
 * @brief Base class for different output styles.
 */
class OutputStyle {
public:
    virtual ~OutputStyle() = default;

    /**
     * @brief Print the data using the specific output style.
     * @param data The data to print, represented as a 2D vector of strings.
     */
    virtual void print(
        const std::vector<std::vector<std::string>>& data) const = 0;
};

/**
 * @class TableOutputStyle
 * @brief Outputs data in a table format with complex formatting options.
 */
class TableOutputStyle : public OutputStyle {
public:
    /**
     * @enum Alignment
     * @brief Defines text alignment options for columns.
     */
    enum class Alignment { LEFT, RIGHT, CENTER };

    /**
     * @brief Constructor to create a TableOutputStyle object.
     * @param header Whether to show the header row.
     */
    explicit TableOutputStyle(bool header = true);

    /**
     * @brief Set the width for each column.
     * @param widths A vector containing the width of each column.
     */
    void setColumnWidths(const std::vector<size_t>& widths);

    /**
     * @brief Set the alignment for each column.
     * @param alignments A vector containing the alignment for each column.
     */
    void setAlignment(const std::vector<Alignment>& alignments);

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    bool showHeader;
    std::vector<size_t> colWidths;
    std::vector<Alignment> columnAlignments;

    void printRow(const std::vector<std::string>& row,
                  const std::vector<size_t>& widths) const;
    void printSeparator(const std::vector<size_t>& widths) const;
};

/**
 * @class CSVOutputStyle
 * @brief Outputs data in CSV format.
 */
class CSVOutputStyle : public OutputStyle {
public:
    void print(
        const std::vector<std::vector<std::string>>& data) const override;
};

/**
 * @class JSONOutputStyle
 * @brief Outputs data in JSON format.
 */
class JSONOutputStyle : public OutputStyle {
public:
    void print(
        const std::vector<std::vector<std::string>>& data) const override;
};

/**
 * @class DataPrinter
 * @brief Manages the output of data using a specified output style.
 */
class DataPrinter {
public:
    /**
     * @brief Constructor to create a DataPrinter object.
     * @param style The output style to use for printing data.
     */
    explicit DataPrinter(std::unique_ptr<OutputStyle> style);

    /**
     * @brief Print the data using the current output style.
     * @param data The data to print, represented as a 2D vector of strings.
     */
    void print(const std::vector<std::vector<std::string>>& data) const;

private:
    std::unique_ptr<OutputStyle> style_;
};

#endif  // LITHIUM_DEBUG_OUTPUT_STYLES_HPP
