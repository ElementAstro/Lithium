#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "atom/system/command.hpp"
#include "atom/system/crash.hpp"
#include "atom/system/crash_quotes.hpp"
#include "atom/system/device.hpp"
#include "atom/system/env.hpp"
#include "atom/system/lregistry.hpp"
#include "atom/system/network_manager.hpp"
#include "atom/system/pidwatcher.hpp"
#include "atom/system/power.hpp"
#include "atom/system/priority.hpp"
#include "atom/system/process_info.hpp"
#include "atom/system/process_manager.hpp"
#include "atom/system/software.hpp"
#include "atom/system/stat.hpp"
#include "atom/system/user.hpp"
#include "atom/system/wregistry.hpp"

namespace py = pybind11;
using namespace atom::system;
using namespace atom::utils;

PYBIND11_MODULE(system, m) {
    m.def("execute_command", &executeCommand, py::arg("command"),
          py::arg("openTerminal") = false,
          py::arg("processLine") = py::cpp_function([](const std::string &) {}),
          "Execute a command and return the command output as a string.");

    m.def("execute_command_with_input", &executeCommandWithInput,
          py::arg("command"), py::arg("input"),
          py::arg("processLine") = nullptr,
          "Execute a command with input and return the command output as a "
          "string.");

    m.def(
        "execute_command_stream", &executeCommandStream, py::arg("command"),
        py::arg("openTerminal"), py::arg("processLine"), py::arg("status"),
        py::arg("terminateCondition") = py::cpp_function([] { return false; }),
        "Execute a command and return the command output as a string.");

    m.def("execute_commands", &executeCommands, py::arg("commands"),
          "Execute a list of commands.");

    m.def("kill_process_by_name", &killProcessByName, py::arg("processName"),
          py::arg("signal"), "Kill a process by its name.");

    m.def("kill_process_by_pid", &killProcessByPID, py::arg("pid"),
          py::arg("signal"), "Kill a process by its PID.");

    m.def("execute_command_with_env", &executeCommandWithEnv,
          py::arg("command"), py::arg("envVars"),
          "Execute a command with environment variables and return the command "
          "output as a string.");

    m.def("execute_command_with_status", &executeCommandWithStatus,
          py::arg("command"),
          "Execute a command and return the command output along with the exit "
          "status.");

    m.def("execute_command_simple", &executeCommandSimple, py::arg("command"),
          "Execute a command and return a boolean indicating whether the "
          "command was successful.");

    m.def("start_process", &startProcess, py::arg("command"),
          "Start a process and return the process ID and handle.");

    py::class_<Quote>(m, "Quote")
        .def(py::init<std::string, std::string>(), py::arg("text"),
             py::arg("author"))
        .def("get_text", &Quote::getText)
        .def("get_author", &Quote::getAuthor)
        .def("__repr__", [](const Quote &q) {
            return "<Quote text='" + q.getText() + "' author='" +
                   q.getAuthor() + "'>";
        });

    py::class_<QuoteManager>(m, "QuoteManager")
        .def(py::init<>())
        .def("add_quote", &QuoteManager::addQuote)
        .def("remove_quote", &QuoteManager::removeQuote)
#ifdef DEBUG
        .def("display_quotes", &QuoteManager::displayQuotes)
#endif
        .def("shuffle_quotes", &QuoteManager::shuffleQuotes)
        .def("clear_quotes", &QuoteManager::clearQuotes)
        .def("load_quotes_from_json", &QuoteManager::loadQuotesFromJson)
        .def("save_quotes_to_json", &QuoteManager::saveQuotesToJson)
        .def("search_quotes", &QuoteManager::searchQuotes)
        .def("filter_quotes_by_author", &QuoteManager::filterQuotesByAuthor)
        .def("get_random_quote", &QuoteManager::getRandomQuote);

    m.def("save_crash_log", &saveCrashLog, py::arg("error_msg"),
          "Save the crash log with the specified error message.");

    py::class_<DeviceInfo>(m, "DeviceInfo")
        .def(py::init<>())
        .def_readwrite("description", &DeviceInfo::description)
        .def_readwrite("address", &DeviceInfo::address)
        .def("__repr__", [](const DeviceInfo &d) {
            return "<DeviceInfo description='" + d.description + "' address='" +
                   d.address + "'>";
        });

    m.def("enumerate_usb_devices", &enumerateUsbDevices,
          "Enumerate USB devices and return a list of DeviceInfo objects.");

    m.def("enumerate_serial_ports", &enumerateSerialPorts,
          "Enumerate serial ports and return a list of DeviceInfo objects.");

    m.def(
        "enumerate_bluetooth_devices", &enumerateBluetoothDevices,
        "Enumerate Bluetooth devices and return a list of DeviceInfo objects.");

    py::class_<Env, std::shared_ptr<Env>>(m, "Env")
        .def(py::init<>())
        .def(py::init<int, char **>(), py::arg("argc"), py::arg("argv"))
        .def_static("create_shared", &Env::createShared, py::arg("argc"),
                    py::arg("argv"))
        .def_static("environ", &Env::Environ)
        .def("add", &Env::add, py::arg("key"), py::arg("val"))
        .def("has", &Env::has, py::arg("key"))
        .def("del", &Env::del, py::arg("key"))
        .def("get", &Env::get, py::arg("key"), py::arg("default_value") = "")
        .def("set_env", &Env::setEnv, py::arg("key"), py::arg("val"))
        .def("get_env", &Env::getEnv, py::arg("key"),
             py::arg("default_value") = "")
        .def("unset_env", &Env::unsetEnv, py::arg("name"))
        .def_static("list_variables", &Env::listVariables)
#if ATOM_ENABLE_DEBUG
        .def_static("print_all_variables", &Env::printAllVariables)
#endif
        .def("__repr__", [](const Env & /*e*/) { return "<Env>"; });

    py::class_<Registry>(m, "Registry")
        .def(py::init<>())
        .def("load_registry_from_file", &Registry::loadRegistryFromFile)
        .def("create_key", &Registry::createKey, py::arg("keyName"))
        .def("delete_key", &Registry::deleteKey, py::arg("keyName"))
        .def("set_value", &Registry::setValue, py::arg("keyName"),
             py::arg("valueName"), py::arg("data"))
        .def("get_value", &Registry::getValue, py::arg("keyName"),
             py::arg("valueName"))
        .def("delete_value", &Registry::deleteValue, py::arg("keyName"),
             py::arg("valueName"))
        .def("backup_registry_data", &Registry::backupRegistryData)
        .def("restore_registry_data", &Registry::restoreRegistryData,
             py::arg("backupFile"))
        .def("key_exists", &Registry::keyExists, py::arg("keyName"))
        .def("value_exists", &Registry::valueExists, py::arg("keyName"),
             py::arg("valueName"))
        .def("get_value_names", &Registry::getValueNames, py::arg("keyName"))
        .def("__repr__", [](const Registry &r) { return "<Registry>"; });

    py::class_<NetworkConnection>(m, "NetworkConnection")
        .def(py::init<>())
        .def_readwrite("protocol", &NetworkConnection::protocol)
        .def_readwrite("localAddress", &NetworkConnection::localAddress)
        .def_readwrite("remoteAddress", &NetworkConnection::remoteAddress)
        .def_readwrite("localPort", &NetworkConnection::localPort)
        .def_readwrite("remotePort", &NetworkConnection::remotePort)
        .def("__repr__", [](const NetworkConnection &nc) {
            return "<NetworkConnection protocol='" + nc.protocol +
                   "' localAddress='" + nc.localAddress + "' remoteAddress='" +
                   nc.remoteAddress +
                   "' localPort=" + std::to_string(nc.localPort) +
                   " remotePort=" + std::to_string(nc.remotePort) + ">";
        });

    py::class_<NetworkInterface, std::shared_ptr<NetworkInterface>>(
        m, "NetworkInterface")
        .def(py::init<std::string, std::vector<std::string>, std::string,
                      bool>(),
             py::arg("name"), py::arg("addresses"), py::arg("mac"),
             py::arg("isUp"))
        .def("get_name", &NetworkInterface::getName)
        .def("get_addresses",
             py::overload_cast<>(&NetworkInterface::getAddresses, py::const_))
        .def("get_mac", &NetworkInterface::getMac)
        .def("is_up", &NetworkInterface::isUp)
        .def("__repr__", [](const NetworkInterface &ni) {
            return "<NetworkInterface name='" + ni.getName() + "'>";
        });

    py::class_<NetworkManager>(m, "NetworkManager")
        .def(py::init<>())
        .def("get_network_interfaces", &NetworkManager::getNetworkInterfaces)
        .def_static("enable_interface", &NetworkManager::enableInterface)
        .def_static("disable_interface", &NetworkManager::disableInterface)
        .def_static("resolve_dns", &NetworkManager::resolveDNS)
        .def("monitor_connection_status",
             &NetworkManager::monitorConnectionStatus)
        .def("get_interface_status", &NetworkManager::getInterfaceStatus)
        .def_static("get_dns_servers", &NetworkManager::getDNSServers)
        .def_static("set_dns_servers", &NetworkManager::setDNSServers)
        .def_static("add_dns_server", &NetworkManager::addDNSServer)
        .def_static("remove_dns_server", &NetworkManager::removeDNSServer)
        .def("__repr__",
             [](const NetworkManager &nm) { return "<NetworkManager>"; });

    m.def("get_network_connections", &getNetworkConnections, py::arg("pid"),
          "Gets the network connections of a process by its PID.");

    py::class_<PidWatcher>(m, "PidWatcher")
        .def(py::init<>())
        .def("set_exit_callback", &PidWatcher::setExitCallback,
             py::arg("callback"))
        .def("set_monitor_function", &PidWatcher::setMonitorFunction,
             py::arg("callback"), py::arg("interval"))
        .def("get_pid_by_name", &PidWatcher::getPidByName, py::arg("name"))
        .def("start", &PidWatcher::start, py::arg("name"))
        .def("stop", &PidWatcher::stop)
        .def("switch", &PidWatcher::Switch, py::arg("name"))
        .def("__repr__", [](const PidWatcher &pw) { return "<PidWatcher>"; });

    m.def("shutdown", &shutdown, "Shutdown the system.");
    m.def("reboot", &reboot, "Reboot the system.");
    m.def("hibernate", &hibernate, "Hibernate the system.");
    m.def("logout", &logout, "Logout the current user.");
    m.def("lock_screen", &lockScreen, "Lock the screen.");
    m.def("set_screen_brightness", &setScreenBrightness, py::arg("level"),
          "Set the screen brightness level.");

    py::class_<PriorityManager>(m, "PriorityManager")
        .def_static("set_process_priority",
                    &PriorityManager::setProcessPriority, py::arg("level"),
                    py::arg("pid") = 0)
        .def_static("get_process_priority",
                    &PriorityManager::getProcessPriority, py::arg("pid") = 0)
        .def_static("set_thread_priority", &PriorityManager::setThreadPriority,
                    py::arg("level"), py::arg("thread") = 0)
        .def_static("get_thread_priority", &PriorityManager::getThreadPriority,
                    py::arg("thread") = 0)
        .def_static("set_thread_scheduling_policy",
                    &PriorityManager::setThreadSchedulingPolicy,
                    py::arg("policy"), py::arg("thread") = 0)
        .def_static("set_process_affinity",
                    &PriorityManager::setProcessAffinity, py::arg("cpus"),
                    py::arg("pid") = 0)
        .def_static("get_process_affinity",
                    &PriorityManager::getProcessAffinity, py::arg("pid") = 0)
        .def_static("start_priority_monitor",
                    &PriorityManager::startPriorityMonitor, py::arg("pid"),
                    py::arg("callback"),
                    py::arg("interval") = std::chrono::seconds(1));

    py::enum_<PriorityManager::PriorityLevel>(m, "PriorityLevel")
        .value("LOWEST", PriorityManager::PriorityLevel::LOWEST)
        .value("BELOW_NORMAL", PriorityManager::PriorityLevel::BELOW_NORMAL)
        .value("NORMAL", PriorityManager::PriorityLevel::NORMAL)
        .value("ABOVE_NORMAL", PriorityManager::PriorityLevel::ABOVE_NORMAL)
        .value("HIGHEST", PriorityManager::PriorityLevel::HIGHEST)
        .value("REALTIME", PriorityManager::PriorityLevel::REALTIME)
        .export_values();

    py::enum_<PriorityManager::SchedulingPolicy>(m, "SchedulingPolicy")
        .value("NORMAL", PriorityManager::SchedulingPolicy::NORMAL)
        .value("FIFO", PriorityManager::SchedulingPolicy::FIFO)
        .value("ROUND_ROBIN", PriorityManager::SchedulingPolicy::ROUND_ROBIN)
        .export_values();

    py::class_<Process>(m, "Process")
        .def(py::init<>())
        .def_readwrite("pid", &Process::pid)
        .def_readwrite("name", &Process::name)
        .def_readwrite("command", &Process::command)
        .def_readwrite("output", &Process::output)
        .def_readwrite("path", &Process::path)
        .def_readwrite("status", &Process::status)
#if defined(_WIN32)
        .def_readwrite("handle", &Process::handle)
#endif
        .def_readwrite("is_background", &Process::isBackground)
        .def("__repr__", [](const Process &p) {
            return "<Process pid=" + std::to_string(p.pid) + " name='" +
                   p.name + "'>";
        });

    py::class_<PrivilegesInfo>(m, "PrivilegesInfo")
        .def(py::init<>())
        .def_readwrite("username", &PrivilegesInfo::username)
        .def_readwrite("groupname", &PrivilegesInfo::groupname)
        .def_readwrite("privileges", &PrivilegesInfo::privileges)
        .def_readwrite("is_admin", &PrivilegesInfo::isAdmin)
        .def("__repr__", [](const PrivilegesInfo &pi) {
            return "<PrivilegesInfo username='" + pi.username +
                   "' groupname='" + pi.groupname + "'>";
        });

    py::class_<ProcessException, std::exception>(m, "ProcessException")
        .def(py::init<const std::string &>())
        .def("__str__", &ProcessException::what);

    py::class_<ProcessManager, std::shared_ptr<ProcessManager>>(
        m, "ProcessManager")
        .def(py::init<int>(), py::arg("maxProcess") = 20)
        .def_static("create_shared", &ProcessManager::createShared,
                    py::arg("maxProcess") = 20)
        .def("create_process", &ProcessManager::createProcess,
             py::arg("command"), py::arg("identifier"),
             py::arg("isBackground") = false)
        .def("terminate_process", &ProcessManager::terminateProcess,
             py::arg("pid"), py::arg("signal") = 15)
        .def("terminate_process_by_name",
             &ProcessManager::terminateProcessByName, py::arg("name"),
             py::arg("signal") = 15)
        .def("has_process", &ProcessManager::hasProcess, py::arg("identifier"))
        .def("get_running_processes", &ProcessManager::getRunningProcesses)
        .def("get_process_output", &ProcessManager::getProcessOutput,
             py::arg("identifier"))
        .def("wait_for_completion", &ProcessManager::waitForCompletion)
        .def("run_script", &ProcessManager::runScript, py::arg("script"),
             py::arg("identifier"), py::arg("isBackground") = false)
        .def("monitor_processes", &ProcessManager::monitorProcesses)
        .def("get_process_info", &ProcessManager::getProcessInfo,
             py::arg("pid"))
#ifdef _WIN32
        .def("get_process_handle", &ProcessManager::getProcessHandle,
             py::arg("pid"))
#else
        .def_static("get_proc_file_path", &ProcessManager::getProcFilePath, py::arg("pid"), py::arg("file"))
#endif
        .def("__repr__",
             [](const ProcessManager &pm) { return "<ProcessManager>"; });

    py::class_<Process>(m, "Process")
        .def(py::init<>())
        .def_readwrite("pid", &Process::pid)
        .def_readwrite("name", &Process::name)
        .def_readwrite("command", &Process::command)
        .def_readwrite("output", &Process::output)
        .def_readwrite("path", &Process::path)
        .def_readwrite("status", &Process::status)
#if defined(_WIN32)
        .def_readwrite("handle", &Process::handle)
#endif
        .def_readwrite("is_background", &Process::isBackground)
        .def("__repr__", [](const Process &p) {
            return "<Process pid=" + std::to_string(p.pid) + " name='" +
                   p.name + "'>";
        });

    py::class_<PrivilegesInfo>(m, "PrivilegesInfo")
        .def(py::init<>())
        .def_readwrite("username", &PrivilegesInfo::username)
        .def_readwrite("groupname", &PrivilegesInfo::groupname)
        .def_readwrite("privileges", &PrivilegesInfo::privileges)
        .def_readwrite("is_admin", &PrivilegesInfo::isAdmin)
        .def("__repr__", [](const PrivilegesInfo &pi) {
            return "<PrivilegesInfo username='" + pi.username +
                   "' groupname='" + pi.groupname + "'>";
        });

    m.def("check_software_installed", &checkSoftwareInstalled,
          py::arg("software_name"),
          "Check whether the specified software is installed.");
    m.def("get_app_version", &getAppVersion, py::arg("app_path"),
          "Get the version of the specified application.");
    m.def("get_app_path", &getAppPath, py::arg("software_name"),
          "Get the path to the specified application.");
    m.def("get_app_permissions", &getAppPermissions, py::arg("app_path"),
          "Get the permissions of the specified application.");

    py::class_<Stat>(m, "Stat")
        .def(py::init<const std::filesystem::path &>(), py::arg("path"))
        .def("update", &Stat::update, "Updates the file statistics.")
        .def("type", &Stat::type, "Gets the type of the file.")
        .def("size", &Stat::size, "Gets the size of the file.")
        .def("atime", &Stat::atime, "Gets the last access time of the file.")
        .def("mtime", &Stat::mtime,
             "Gets the last modification time of the file.")
        .def("ctime", &Stat::ctime, "Gets the creation time of the file.")
        .def("mode", &Stat::mode, "Gets the file mode/permissions.")
        .def("uid", &Stat::uid, "Gets the user ID of the file owner.")
        .def("gid", &Stat::gid, "Gets the group ID of the file owner.")
        .def("path", &Stat::path, "Gets the path of the file.")
        .def("__repr__", [](const Stat &s) {
            return "<Stat path='" + s.path().string() + "'>";
        });

    py::enum_<std::filesystem::file_type>(m, "FileType")
        .value("none", std::filesystem::file_type::none)
        .value("not_found", std::filesystem::file_type::not_found)
        .value("regular", std::filesystem::file_type::regular)
        .value("directory", std::filesystem::file_type::directory)
        .value("symlink", std::filesystem::file_type::symlink)
        .value("block", std::filesystem::file_type::block)
        .value("character", std::filesystem::file_type::character)
        .value("fifo", std::filesystem::file_type::fifo)
        .value("socket", std::filesystem::file_type::socket)
        .value("unknown", std::filesystem::file_type::unknown)
        .export_values();

    m.def("get_user_groups", &getUserGroups, "Get user groups.");
    m.def("get_username", &getUsername, "Get user name.");
    m.def("get_hostname", &getHostname, "Get host name.");
    m.def("get_user_id", &getUserId, "Get user ID.");
    m.def("get_group_id", &getGroupId, "Get group ID.");
    m.def("get_home_directory", &getHomeDirectory,
          "Get user profile directory.");
    m.def("get_current_working_directory", &getCurrentWorkingDirectory,
          "Get current working directory.");
    m.def("get_login_shell", &getLoginShell, "Get login shell.");
    m.def("get_login", &getLogin, "Retrieve the login name of the user.");
    m.def("is_root", &isRoot,
          "Check if the current user has root/administrator privileges.");

#ifdef _WIN32
    m.def("get_user_profile_directory", &getUserProfileDirectory,
          "Get user profile directory (Windows only).");
#endif

// Expose HKEY constants if on Windows
#ifdef _WIN32
    py::enum_<HKEY>(m, "HKEY")
        .value("HKEY_CLASSES_ROOT", HKEY_CLASSES_ROOT)
        .value("HKEY_CURRENT_USER", HKEY_CURRENT_USER)
        .value("HKEY_LOCAL_MACHINE", HKEY_LOCAL_MACHINE)
        .value("HKEY_USERS", HKEY_USERS)
        .value("HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG)
        .export_values();
#endif
#ifdef _WIN32
    // Binding for getRegistrySubKeys
    m.def(
        "get_registry_sub_keys",
        [](HKEY hRootKey,
           const std::string &subKey) -> std::vector<std::string> {
            std::vector<std::string> subKeys;
            bool success = getRegistrySubKeys(hRootKey, subKey, subKeys);
            if (!success) {
                throw std::runtime_error("Failed to get registry sub keys.");
            }
            return subKeys;
        },
        py::arg("hRootKey"), py::arg("subKey"),
        "Get all subkey names under the specified registry key.");

    // Binding for getRegistryValues
    m.def(
        "get_registry_values",
        [](HKEY hRootKey, const std::string &subKey)
            -> std::vector<std::pair<std::string, std::string>> {
            std::vector<std::pair<std::string, std::string>> values;
            bool success = getRegistryValues(hRootKey, subKey, values);
            if (!success) {
                throw std::runtime_error("Failed to get registry values.");
            }
            return values;
        },
        py::arg("hRootKey"), py::arg("subKey"),
        "Get all value names and data under the specified registry key.");

    // Binding for modifyRegistryValue
    m.def(
        "modify_registry_value",
        [](HKEY hRootKey, const std::string &subKey,
           const std::string &valueName, const std::string &newValue) -> bool {
            bool success =
                modifyRegistryValue(hRootKey, subKey, valueName, newValue);
            if (!success) {
                throw std::runtime_error("Failed to modify registry value.");
            }
            return success;
        },
        py::arg("hRootKey"), py::arg("subKey"), py::arg("valueName"),
        py::arg("newValue"), "Modify the data of a specified registry value.");

    // Binding for deleteRegistrySubKey
    m.def(
        "delete_registry_sub_key",
        [](HKEY hRootKey, const std::string &subKey) -> bool {
            bool success = deleteRegistrySubKey(hRootKey, subKey);
            if (!success) {
                throw std::runtime_error("Failed to delete registry subkey.");
            }
            return success;
        },
        py::arg("hRootKey"), py::arg("subKey"),
        "Delete a specified registry subkey and all its subkeys.");

    // Binding for deleteRegistryValue
    m.def(
        "delete_registry_value",
        [](HKEY hRootKey, const std::string &subKey,
           const std::string &valueName) -> bool {
            bool success = deleteRegistryValue(hRootKey, subKey, valueName);
            if (!success) {
                throw std::runtime_error("Failed to delete registry value.");
            }
            return success;
        },
        py::arg("hRootKey"), py::arg("subKey"), py::arg("valueName"),
        "Delete a specified registry value under the given subkey.");

    // Binding for recursivelyEnumerateRegistrySubKeys
    m.def(
        "recursively_enumerate_registry_sub_keys",
        [](HKEY hRootKey, const std::string &subKey) {
            recursivelyEnumerateRegistrySubKeys(hRootKey, subKey);
        },
        py::arg("hRootKey"), py::arg("subKey"),
        "Recursively enumerate all subkeys and values under the specified "
        "registry key.");

    // Binding for backupRegistry
    m.def(
        "backup_registry",
        [](HKEY hRootKey, const std::string &subKey,
           const std::string &backupFilePath) -> bool {
            bool success = backupRegistry(hRootKey, subKey, backupFilePath);
            if (!success) {
                throw std::runtime_error("Failed to backup registry.");
            }
            return success;
        },
        py::arg("hRootKey"), py::arg("subKey"), py::arg("backupFilePath"),
        "Backup the specified registry key and all its subkeys and values to a "
        "REG file.");

    // Binding for findRegistryKey
    m.def(
        "find_registry_key",
        [](HKEY hRootKey, const std::string &subKey,
           const std::string &searchKey) {
            findRegistryKey(hRootKey, subKey, searchKey);
        },
        py::arg("hRootKey"), py::arg("subKey"), py::arg("searchKey"),
        "Recursively find subkeys containing the specified string.");

    // Binding for findRegistryValue
    m.def(
        "find_registry_value",
        [](HKEY hRootKey, const std::string &subKey,
           const std::string &searchValue) {
            findRegistryValue(hRootKey, subKey, searchValue);
        },
        py::arg("hRootKey"), py::arg("subKey"), py::arg("searchValue"),
        "Recursively find values containing the specified string.");
#endif
}
