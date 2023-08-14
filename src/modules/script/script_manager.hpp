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

class MessageBus;

namespace Lithium
{
    class ChaiScriptManager
    {
    public:
        ChaiScriptManager(std::shared_ptr<MessageBus> messageBus);
        ~ChaiScriptManager();

        static std::shared_ptr<ChaiScriptManager> createShared(std::shared_ptr<MessageBus> messageBus);

        void Init();
        void InitSubModules();
        void InitMyApp();

        tl::expected<bool, std::string> loadScriptFile(const std::string &filename);
        tl::expected<bool, std::string> runCommand(const std::string &command);
        tl::expected<bool, std::string> runMultiCommand(const std::vector<std::string> &commands);
        tl::expected<bool, std::string> runScript(const std::string &filename);

    private:
        std::unique_ptr<chaiscript::ChaiScript> chai_;
        std::shared_ptr<MessageBus> m_MessageBus;
    };

}
