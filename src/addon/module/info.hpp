#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace Lithium::Component
{
    class ModuleInfo
    {
        // All of the module information
    public:
        std::string m_name;
        std::string m_description;
        std::string m_version;
        std::string m_status;
        std::string m_type;
        std::string m_author;
        std::string m_license;
        std::string m_path;
        std::string m_config_path;
        std::string m_config_file;

        // Module enable status
        std::atomic_bool m_enabled;

        // All of the functions in the module(dynamic loaded)
        std::vector<std::string> m_functions

        // Module handle pointer
        void *handle;
    };
} // namespace Lithium::Component
