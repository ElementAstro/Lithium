#ifndef COREDUMPANALYZER_H
#define COREDUMPANALYZER_H

#include <memory>
#include <string>

namespace lithium::addon {
class CoreDumpAnalyzer {
public:
    CoreDumpAnalyzer();
    ~CoreDumpAnalyzer();

    bool readFile(const std::string& filename);
    void analyze();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};
}  // namespace lithium::addon

#endif  // COREDUMPANALYZER_H