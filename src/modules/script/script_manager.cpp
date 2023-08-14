#include "script_manager.hpp"

#include <chaiscript/chaiscript.hpp>

#include "chaiscript/extras/math.hpp"
#include "chaiscript/extras/string_methods.hpp"

#include "server/message_bus.hpp"

#include "liproperty/base64.hpp"
#include "liproperty/iproperty.hpp"
#include "liproperty/uuid.hpp"

#include "modules/system/system.hpp"

#include "modules/io/compress.hpp"
#include "modules/io/io.hpp"

#include "LithiumApp.hpp"

#include "loguru/loguru.hpp"
#include "tl/expected.hpp"
#include "nlohmann/json.hpp"

#include "error/error_code.hpp"

#include "liscript/device.hpp"

namespace Lithium
{
    ChaiScriptManager::ChaiScriptManager(std::shared_ptr<MessageBus> messageBus) : chai_(std::make_unique<chaiscript::ChaiScript>())
    {
        m_MessageBus = messageBus;
        LOG_F(INFO, "ChaiScript Manager initializing ...");
        Init();
        InitSubModules();
        LOG_F(INFO, "ChaiScriptManager initialized");
    }

    ChaiScriptManager::~ChaiScriptManager()
    {
    }

    std::shared_ptr<ChaiScriptManager> ChaiScriptManager::createShared(std::shared_ptr<MessageBus> messageBus)
    {
        return std::make_shared<ChaiScriptManager>(messageBus);
    }

    void ChaiScriptManager::Init()
    {
        // Inject IProperty
        chai_->add(chaiscript::fun(&IProperty::device_name), "device_name");
        chai_->add(chaiscript::fun(&IProperty::device_uuid), "device_uuid");
        chai_->add(chaiscript::fun(&IProperty::message_uuid), "message_uuid");
        chai_->add(chaiscript::fun(&IProperty::name), "name");
        chai_->add(chaiscript::fun(&IProperty::value), "value");
        chai_->add(chaiscript::fun(&IProperty::GetName), "get_name");
        chai_->add(chaiscript::fun(&IProperty::toJson), "to_json");
        chai_->add(chaiscript::fun(&IProperty::toXml), "to_xml");
        chai_->add(chaiscript::fun(&IProperty::GetMessageUUID), "get_message_uuid");
        chai_->add(chaiscript::fun(&IProperty::SetMessageUUID), "set_message_uuid");
        chai_->add(chaiscript::fun(&IProperty::GetDeviceUUID), "get_device_uuid");
        chai_->add(chaiscript::fun(&IProperty::SetDeviceUUID), "set_device_uuid");
        chai_->add(chaiscript::fun(&IProperty::getValue<std::string>), "get_value");
        chai_->add(chaiscript::fun(&IProperty::setValue<std::string>), "set_value");
        chai_->add(chaiscript::constructor<IProperty()>(), "IProperty");
        chai_->add(chaiscript::user_type<IProperty>(), "IProperty");

        // Add Base64 support
        chai_->add(chaiscript::fun(&Base64::base64Decode), "base64_decode");
        chai_->add(chaiscript::fun(&Base64::base64Encode), "base64_encode");

        // Add UUID support
        chai_->add(chaiscript::fun(&UUID::UUIDGenerator::seed), "seed");
        chai_->add(chaiscript::fun(&UUID::UUIDGenerator::generateUUID), "generate_uuid");
        chai_->add(chaiscript::fun(&UUID::UUIDGenerator::generateUUIDWithFormat), "generate_uuid_with_format");
        chai_->add(chaiscript::user_type<UUID::UUIDGenerator>(), "UUIDGenerator");

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
        chai_->add(chaiscript::fun(&LithiumApp::sleepThreadByName), "sleep_thread_by_name");
        chai_->add(chaiscript::fun(&LithiumApp::stopTask), "stop_task");
        chai_->add(chaiscript::fun(&LithiumApp::terminateProcess), "terminate_process");
        chai_->add(chaiscript::fun(&LithiumApp::terminateProcessByName), "terminate_process_by_name");
        chai_->add(chaiscript::constructor<LithiumApp()>(), "LithiumApp");
        chai_->add(chaiscript::user_type<LithiumApp>(), "LithiumApp");

        chai_->add(chaiscript::user_type<LIError>(), "LiError");

        chai_->add(chaiscript::type_conversion<const char *, std::string>());
    }

    void ChaiScriptManager::InitSubModules()
    {
        chai_->add(chaiscript::extras::math::bootstrap());
        chai_->add(chaiscript::extras::string_methods::bootstrap());

        chai_->add(create_chaiscript_device_module());
    }

    void ChaiScriptManager::InitMyApp()
    {
        chai_->add_global(chaiscript::var(MyApp), "app");
    }

    tl::expected<bool, std::string> ChaiScriptManager::loadScriptFile(const std::string &filename)
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
            LOG_F(ERROR, "Failed to open script file: %s", filename.c_str());
            return tl::unexpected("Failed to open script file");
        }
        return true;
    }

    tl::expected<bool, std::string> ChaiScriptManager::runCommand(const std::string &command)
    {
        try
        {
            chai_->eval(command);
        }
        catch (chaiscript::exception::eval_error &e)
        {
            LOG_F(ERROR, "Failed to eval %s : %s", e.filename.c_str(), e.what());
            return tl::unexpected("Failed to eval script command");
        };
        return true;
    }

    tl::expected<bool, std::string> ChaiScriptManager::runScript(const std::string &filename)
    {
        try
        {
            chai_->eval_file(filename);
        }
        catch (chaiscript::exception::eval_error &e)
        {
            LOG_F(ERROR, "Failed to run %s : %s", e.filename.c_str(), e.what());
            return tl::unexpected("Failed to run script file");
        }
        return true;
    }

    tl::expected<bool, std::string> ChaiScriptManager::runMultiCommand(const std::vector<std::string> &commands)
    {
        for (auto command : commands)
        {
            try
            {
                chai_->eval(command);
            }
            catch (chaiscript::exception::eval_error &e)
            {
                LOG_F(ERROR, "Failed to run: %s", e.what());
                return tl::unexpected("Failed to run multi commands");
            }
        }
        return true;
    }
}
