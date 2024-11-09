#ifndef LITHIUM_TARGET_READER_CSV
#define LITHIUM_TARGET_READER_CSV

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace lithium::target {
// 支持的字符编码
enum class Encoding { UTF8, UTF16 };

// 引用模式
enum class Quoting { MINIMAL, ALL, NONNUMERIC, STRINGS, NOTNULL, NONE };

// CSV 方言配置
struct Dialect {
    char delimiter = ',';
    char quotechar = '"';
    bool doublequote = true;
    bool skip_initial_space = false;
    std::string lineterminator = "\n";
    Quoting quoting = Quoting::MINIMAL;

    Dialect() = default;
    Dialect(char delim, char quote, bool dquote, bool skipspace,
            std::string lineterm, Quoting quote_mode);
};

// 字典读取器
class DictReader {
public:
    DictReader(std::istream& input, const std::vector<std::string>& fieldnames,
               Dialect dialect = Dialect(), Encoding encoding = Encoding::UTF8);

    bool next(std::unordered_map<std::string, std::string>& row);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

// 字典写入器
class DictWriter {
public:
    DictWriter(std::ostream& output, const std::vector<std::string>& fieldnames,
               Dialect dialect = Dialect(), bool quote_all = false,
               Encoding encoding = Encoding::UTF8);

    void writeRow(const std::unordered_map<std::string, std::string>& row);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}  // namespace lithium::target

#endif  // LITHIUM_TARGET_READER_CSV