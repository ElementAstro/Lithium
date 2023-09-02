#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

namespace chaiscript
{
    class ChaiScript;
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

        bool loadScriptFile(const std::string &filename);
        bool runCommand(const std::string &command);
        bool runMultiCommand(const std::vector<std::string> &commands);
        bool runScript(const std::string &filename);

    private:
        std::unique_ptr<chaiscript::ChaiScript> chai_;
        std::shared_ptr<MessageBus> m_MessageBus;
    };

}
