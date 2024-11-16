/**
 * @file output_style.hpp
 * @brief Defines various output styles for displaying data.
 */

#ifndef LITHIUM_DEBUG_OUTPUT_STYLE_HPP
#define LITHIUM_DEBUG_OUTPUT_STYLE_HPP

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
     * @brief Prints data using a specific output style.
     * @param data The data to be printed, represented as a 2D vector of
     * strings.
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
     * @brief Constructor for creating a TableOutputStyle object.
     * @param header Whether to display the header.
     */
    explicit TableOutputStyle(bool header = true);

    ~TableOutputStyle() override;

    /**
     * @brief Sets the width of each column.
     * @param widths A vector containing the width of each column.
     */
    void setColumnWidths(const std::vector<size_t>& widths);

    /**
     * @brief Sets the alignment of each column.
     * @param alignments A vector containing the alignment of each column.
     */
    void setAlignment(const std::vector<Alignment>& alignments);

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class CSVOutputStyle
 * @brief Outputs data in CSV format.
 */
class CSVOutputStyle : public OutputStyle {
public:
    CSVOutputStyle();

    ~CSVOutputStyle() override;

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class JSONOutputStyle
 * @brief Outputs data in JSON format.
 */
class JSONOutputStyle : public OutputStyle {
public:
    JSONOutputStyle();

    ~JSONOutputStyle() override;

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class XMLOutputStyle
 * @brief Outputs data in XML format.
 */
class XMLOutputStyle : public OutputStyle {
public:
    XMLOutputStyle();

    ~XMLOutputStyle() override;

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class MarkdownOutputStyle
 * @brief Outputs data in Markdown format.
 */
class MarkdownOutputStyle : public OutputStyle {
public:
    MarkdownOutputStyle();

    ~MarkdownOutputStyle() override;

    void print(
        const std::vector<std::vector<std::string>>& data) const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @class DataPrinter
 * @brief Manages data output using a specified output style.
 */
class DataPrinter {
public:
    /**
     * @brief Constructor for creating a DataPrinter object.
     * @param style The output style to be used for printing data.
     */
    explicit DataPrinter(std::unique_ptr<OutputStyle> style);

    /**
     * @brief Prints data using the current output style.
     * @param data The data to be printed, represented as a 2D vector of
     * strings.
     */
    void print(const std::vector<std::vector<std::string>>& data) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif  // LITHIUM_DEBUG_OUTPUT_STYLE_HPP