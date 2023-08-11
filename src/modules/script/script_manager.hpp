#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

namespace chaiscript
{
    class ChaiScript;
}

namespace tl
{
    template <class T, class E>
    class expected;
}

namespace Lithium
{
    class ChaiScriptManager
    {
    public:
        ChaiScriptManager();
        ~ChaiScriptManager();

        static std::shared_ptr<ChaiScriptManager> createShared();

        void Init();
        void InitSubModules();

        tl::expected<bool, std::string> loadScriptFile(const std::string &filename);
        tl::expected<bool, std::string> runCommand(const std::string &command);
        tl::expected<bool, std::string> runMultiCommand(const std::vector<std::string> &commands);
        tl::expected<bool, std::string> runScript(const std::string &filename);

    private:
        std::unique_ptr<chaiscript::ChaiScript> chai_;
    };

}
