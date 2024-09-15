#ifndef COMPILE_COMMAND_GENERATOR_H
#define COMPILE_COMMAND_GENERATOR_H

#include <memory>
#include <string>

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
class CompileCommandGenerator {
public:
    CompileCommandGenerator();
    ~CompileCommandGenerator();


    void setSourceDir(const std::string& dir);
    void setCompiler(const std::string& compiler);
    void setIncludeFlag(const std::string& flag);
    void setOutputFlag(const std::string& flag);
    void setProjectName(const std::string& name);
    void setProjectVersion(const std::string& version);
    void addExtension(const std::string& ext);
    void setOutputPath(const std::string& path);
    void setExistingCommandsPath(const std::string& path);

    void generate();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}  // namespace lithium

#endif  // COMPILE_COMMAND_GENERATOR_H
