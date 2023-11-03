/*
 * script_manager.cpp
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

#include "script_manager.hpp"

#include <chaiscript/chaiscript.hpp>

#include "chaiscript/extras/math.hpp"
#include "chaiscript/extras/string_methods.hpp"

#include "server/message_bus.hpp"

#include "core/property/base64.hpp"
#include "core/property/iproperty.hpp"
#include "core/property/uuid.hpp"

#include "modules/system/system.hpp"

#include "modules/io/compress.hpp"
#include "modules/io/io.hpp"

#include "LithiumApp.hpp"

#include "loguru/loguru.hpp"
#include "nlohmann/json.hpp"
#include "error/error_code.hpp"

#include "script/lithium_script.hpp"

namespace Lithium
{
    ScriptManager::ScriptManager(std::shared_ptr<MessageBus> messageBus) : chai_(std::make_unique<chaiscript::ChaiScript>())
    {
        m_MessageBus = messageBus;
    }

    ScriptManager::~ScriptManager()
    {
    }

    std::shared_ptr<ScriptManager> ScriptManager::createShared(std::shared_ptr<Lithium::MessageBus> messageBus)
    {
        return std::make_shared<ScriptManager>(messageBus);
    }

    void ScriptManager::Init()
    {
        // Add Base64 support
        chai_->add(chaiscript::fun(&Base64::base64Decode), "base64_decode");
        chai_->add(chaiscript::fun(&Base64::base64Encode), "base64_encode");
        chai_->add(chaiscript::fun(&Base64::base64EncodeEnhance), "base64encode_e");
        chai_->add(chaiscript::fun(&Base64::base64DecodeEnhance), "base64decode_e");

        // Add UUID support
        chai_->add(chaiscript::fun(&LITHIUM::UUID::UUIDGenerator::seed), "seed");
        chai_->add(chaiscript::fun(&LITHIUM::UUID::UUIDGenerator::generateUUID), "generate_uuid");
        chai_->add(chaiscript::fun(&LITHIUM::UUID::UUIDGenerator::generateUUIDWithFormat), "generate_uuid_with_format");
        chai_->add(chaiscript::user_type<LITHIUM::UUID::UUIDGenerator>(), "UUIDGenerator");

        chai_->add(chaiscript::fun(&File::compress_file), "compress_file");
        chai_->add(chaiscript::fun(&File::decompress_file), "decompress_file");
        chai_->add(chaiscript::fun(&File::compress_folder), "compress_folder");
        chai_->add(chaiscript::fun(&File::create_zip), "create_zip");
        chai_->add(chaiscript::fun(&File::extract_zip), "extract_zip");

        chai_->add(chaiscript::fun(&File::create_directory), "create_directory");
        chai_->add(chaiscript::fun(&File::remove_directory), "remove_directory");
        chai_->add(chaiscript::fun(&File::rename_directory), "rename_directory");
        chai_->add(chaiscript::fun(&File::move_directory), "move_directory");
        chai_->add(chaiscript::fun(&File::copy_file), "copy_file");
        chai_->add(chaiscript::fun(&File::move_file), "move_file");
        chai_->add(chaiscript::fun(&File::remove_file), "remove_file");
        chai_->add(chaiscript::fun(&File::rename_file), "rename_file");

        chai_->add(chaiscript::fun(&LithiumApp::addDevice), "add_device");
        chai_->add(chaiscript::fun(&LithiumApp::addDeviceLibrary), "add_device_library");
        chai_->add(chaiscript::fun(&LithiumApp::addDeviceObserver), "add_device_observer");
        chai_->add(chaiscript::fun(&LithiumApp::addTask), "add_task");
        chai_->add(chaiscript::fun(&LithiumApp::addThread), "add_thread");
        chai_->add(chaiscript::fun(&LithiumApp::createProcess), "create_process");
        chai_->add(chaiscript::fun(&LithiumApp::deleteTask), "delete_task");
        chai_->add(chaiscript::fun(&LithiumApp::deleteTaskByName), "delete_task_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::executeAllTasks), "execute_all_tasks");
        chai_->add(chaiscript::fun(&LithiumApp::executeTaskByName), "execute_task_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::findDevice), "find_device");
        chai_->add(chaiscript::fun(&LithiumApp::findDeviceByName), "find_device_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::GetConfig), "get_config");
        chai_->add(chaiscript::fun(&LithiumApp::getDevice), "get_device");
        chai_->add(chaiscript::fun(&LithiumApp::getDeviceList), "get_device_list");
        chai_->add(chaiscript::fun(&LithiumApp::getProcessOutput), "get_process_output");
        chai_->add(chaiscript::fun(&LithiumApp::getRunningProcesses), "get_running_processes");
        chai_->add(chaiscript::fun(&LithiumApp::getTask), "get_task");
        chai_->add(chaiscript::fun(&LithiumApp::getTaskList), "get_task_list");
        chai_->add(chaiscript::fun(&LithiumApp::insertTask), "insert_task");
        chai_->add(chaiscript::fun(&LithiumApp::isThreadRunning), "is_thread_running");
        chai_->add(chaiscript::fun(&LithiumApp::joinAllThreads), "join_all_threads");
        chai_->add(chaiscript::fun(&LithiumApp::joinThreadByName), "join_thread_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::modifyTask), "modify_task");
        chai_->add(chaiscript::fun(&LithiumApp::modifyTaskByName), "modify_task_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::queryTaskByName), "query_task_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::removeDevice), "remove_device");
        chai_->add(chaiscript::fun(&LithiumApp::removeDeviceLibrary), "remove_device_library");
        chai_->add(chaiscript::fun(&LithiumApp::removeDevicesByName), "remove_device_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::runScript), "run_script");
        chai_->add(chaiscript::fun(&LithiumApp::saveTasksToJson), "save_tasks_to_json");
        chai_->add(chaiscript::fun(&LithiumApp::SetConfig), "set_config");
        chai_->add(chaiscript::fun(&LithiumApp::stopTask), "stop_task");
        chai_->add(chaiscript::fun(&LithiumApp::terminateProcess), "terminate_process");
        chai_->add(chaiscript::fun(&LithiumApp::terminateProcessByName), "terminate_process_by_name");
        chai_->add(chaiscript::constructor<LithiumApp()>(), "LithiumApp");
        chai_->add(chaiscript::user_type<LithiumApp>(), "LithiumApp");

        chai_->add(chaiscript::user_type<LIError>(), "LiError");

        chai_->add(chaiscript::type_conversion<const char *, std::string>());
    }

    void ScriptManager::InitSubModules()
    {
        chai_->add(chaiscript::extras::math::bootstrap());
        chai_->add(chaiscript::extras::string_methods::bootstrap());

        chai_->add(create_chaiscript_device_module());
    }

    void ScriptManager::InitMyApp()
    {
        DLOG_F(INFO, "ChaiScript Manager initializing ...");
        Init();
        InitSubModules();
        DLOG_F(INFO, "ScriptManager initialized");
        chai_->add_global(chaiscript::var(MyApp), "app");
    }

    bool ScriptManager::loadScriptFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (file)
        {
            std::string script((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            chai_->eval(script);
        }
        else
        {
            LOG_F(ERROR, "Failed to open script file: {}", filename);
            return false;
        }
        return true;
    }

    bool ScriptManager::runCommand(const std::string &command)
    {
        try
        {
            chai_->eval(command);
        }
        catch (chaiscript::exception::eval_error &e)
        {
            LOG_F(ERROR, "Failed to eval {} : {}", e.filename, e.what());
            return false;
        };
        return true;
    }

    bool ScriptManager::runScript(const std::string &filename)
    {
        try
        {
            chai_->eval_file(filename);
        }
        catch (chaiscript::exception::eval_error &e)
        {
            LOG_F(ERROR, "Failed to run {} : {}", e.filename, e.what());
            return false;
        }
        return true;
    }

    bool ScriptManager::runMultiCommand(const std::vector<std::string> &commands)
    {
        for (auto command : commands)
        {
            try
            {
                chai_->eval(command);
            }
            catch (chaiscript::exception::eval_error &e)
            {
                LOG_F(ERROR, "Failed to run: {}", e.what());
                return false;
            }
        }
        return true;
    }
}
