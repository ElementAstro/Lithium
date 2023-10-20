/*
 * script_manager.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: Script Manager

**************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

namespace chaiscript
{
    class ChaiScript;
}

namespace Lithium
{
    class MessageBus;

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
