#ifndef LITHIUM_SCRIPT_CHECKER_HPP
#define LITHIUM_SCRIPT_CHECKER_HPP

#include <memory>
#include <string>

#include "atom/type/noncopyable.hpp"

namespace lithium {
class ScriptAnalyzerImpl;  // 前向声明

class ScriptAnalyzer : public NonCopyable {
public:
    explicit ScriptAnalyzer(const std::string& config_file);
    ~ScriptAnalyzer() override;  // 析构函数需要在.cpp中定义

    void analyze(const std::string& script, bool output_json = false);

private:
    std::unique_ptr<ScriptAnalyzerImpl> impl_;  // 指向实现类的智能指针
};
}  // namespace lithium

#endif  // LITHIUM_SCRIPT_CHECKER_HPP
