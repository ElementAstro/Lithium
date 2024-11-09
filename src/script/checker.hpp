#ifndef LITHIUM_SCRIPT_CHECKER_HPP
#define LITHIUM_SCRIPT_CHECKER_HPP

#include <memory>
#include <string>

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"

class InvalidFormatException : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_INVALID_FORMAT(...)                                \
    throw InvalidFormatException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__);

namespace lithium {
class ScriptAnalyzerImpl;

enum class ReportFormat { TEXT, JSON, XML };

class ScriptAnalyzer : public NonCopyable {
public:
    explicit ScriptAnalyzer(const std::string& config_file);
    ~ScriptAnalyzer() override;

    void analyze(const std::string& script, bool output_json = false,
                 ReportFormat format = ReportFormat::TEXT);

private:
    std::unique_ptr<ScriptAnalyzerImpl> impl_;  // 指向实现类的智能指针
};
}  // namespace lithium

#endif  // LITHIUM_SCRIPT_CHECKER_HPP
