#include "StaticFileManager.hpp"

oatpp::String StaticFileManager::getFile(const oatpp::String &filename, bool cache)
{

    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_cache.find(filename);
    oatpp::String buffer;
    if (it == m_cache.end())
    {
        buffer = oatpp::String::loadFromFile(filename->c_str());
        if (buffer && cache)
        {
            m_cache[filename] = buffer;
        }
    }
    else
    {
        buffer = it->second;
    }

    return buffer;
}