#ifndef COMPILE_COMMAND_GENERATOR_HPP
#define COMPILE_COMMAND_GENERATOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lithium {

class CompileCommandGenerator {
public:
    CompileCommandGenerator();
    ~CompileCommandGenerator();

    auto setOption(const std::string& key,
                   const std::string& value) -> CompileCommandGenerator&;
    auto addTarget(const std::string& target_name) -> CompileCommandGenerator&;
    auto setTargetOption(const std::string& target_name, const std::string& key,
                         const std::string& value) -> CompileCommandGenerator&;
    auto addConditionalOption(const std::string& condition,
                              const std::string& key, const std::string& value)
        -> CompileCommandGenerator&;
    auto addDefine(const std::string& define) -> CompileCommandGenerator&;
    auto addFlag(const std::string& flag) -> CompileCommandGenerator&;
    auto addLibrary(const std::string& library_path)
        -> CompileCommandGenerator&;
    auto setCommandTemplate(const std::string& template_str)
        -> CompileCommandGenerator&;
    auto setCompiler(const std::string& compiler)
        -> CompileCommandGenerator&;  // 新增方法声明

    void loadConfigFromFile(const std::string& config_path);
    void generate();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium

#endif  // COMPILE_COMMAND_GENERATOR_HPP