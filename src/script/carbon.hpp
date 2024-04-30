#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>


namespace lithium {
class CarbonScriptImpl;

class CarbonScript {
public:
    CarbonScript();
    ~CarbonScript();

    static std::shared_ptr<CarbonScript> createShared();

    void Init();
    void InitSubModules();
    void InitMyApp();

    bool LoadScriptFile(const std::string &filename);
    bool RunCommand(const std::string &command);
    bool RunMultiCommand(const std::vector<std::string> &commands);
    bool RunScript(const std::string &filename);

private:
    std::unique_ptr<CarbonScriptImpl> impl_;
};

}  // namespace lithium
