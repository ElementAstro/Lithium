#ifndef LITHIUM_ADDON_DEBUG_DYNAMIC_HPP
#define LITHIUM_ADDON_DEBUG_DYNAMIC_HPP

#include <memory>
#include <string>

namespace lithium::addon {
class DynamicLibraryParser {
public:
    DynamicLibraryParser(const std::string &executable);
    ~DynamicLibraryParser();

    void setJsonOutput(bool json_output);
    void setOutputFilename(const std::string &filename);
    void parse();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium::addon

#endif  // LITHIUM_ADDON_DEBUG_DYNAMIC_HPP