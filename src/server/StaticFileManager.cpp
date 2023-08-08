#include "StaticFileManager.hpp"

oatpp::String StaticFileManager::getFile(const oatpp::String &filename, bool cache)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_cache.find(filename);
    oatpp::String buffer;
    if (it == m_cache.end())
    {
        std::string filePath = filename->std_str();

        std::string fileExtension = filePath.substr(filePath.find_last_of(".") + 1);
        if (m_allowedExtensions.find(fileExtension) == m_allowedExtensions.end())
        {
            OATPP_LOGD("StaticFileManager", "File type not allowed: %s", fileExtension.c_str());
            return nullptr;
        }

        std::ifstream file(filePath);

        if (!file.is_open())
        {
            OATPP_LOGD("StaticFileManager", "Failed to open file: %s", filePath.c_str());
            return nullptr;
        }

        std::stringstream fileBuffer;
        fileBuffer << file.rdbuf();
        buffer = fileBuffer.str();

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
