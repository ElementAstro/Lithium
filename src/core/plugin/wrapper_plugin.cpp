#include "wrapper_plugin.hpp"

#ifdef _WIN32
const std::string CHECK_COMMAND = "where";
#else
const std::string CHECK_COMMAND = "which";
#endif

WrapperPlugin::WrapperPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : Plugin(path, version, author, description)
{

}