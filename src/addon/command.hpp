#ifndef COMPILE_COMMAND_GENERATOR_H
#define COMPILE_COMMAND_GENERATOR_H

#include <string>
#include <vector>

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
    struct Impl;  // Forward declaration of the implementation class
    Impl* pImpl;  // Pointer to the implementation
};

#endif  // COMPILE_COMMAND_GENERATOR_H
