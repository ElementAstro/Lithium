#include "StaticFileManager.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

#include <thread>
#include <fstream>

#include "bustache/render/ostream.hpp"
#include "MustacheTemplate.hpp"

const int StaticFileManager::MAX_FILE_LOADS_PER_SECOND = 1000;
const int StaticFileManager::MAX_ERROR_COUNT = 3;

StaticFileManager::StaticFileManager()
    : m_loadingCounter(0)
{
    m_lastLoadTime = std::chrono::steady_clock::now();
}

bool StaticFileManager::isAllowedExtension(const oatpp::String &extension)
{
    // Add allowed extensions as per your requirement
    static const std::unordered_set<oatpp::String> m_allowedExtensions = {"txt", "html", "css", "js"};
    return m_allowedExtensions.find(extension) != m_allowedExtensions.end();
}

std::chrono::system_clock::time_point StaticFileManager::getLastModifiedTime(const std::string &filePath)
{
    struct stat result;
    if (stat(filePath.c_str(), &result) == 0)
    {
        return std::chrono::system_clock::from_time_t(result.st_mtime);
    }
    return std::chrono::system_clock::time_point();
}

void StaticFileManager::preprocessHtmlFile(std::ifstream &fileStream, std::ostringstream &preprocessedStream)
{
    std::string fileContent((std::istreambuf_iterator<char>(fileStream)),
                            std::istreambuf_iterator<char>());

    html::context context;
    html::object data
    {
        {"basic_header",R"(
            <meta charset="utf-8">
            <meta http-equiv="X-UA-Compatible" content="IE=edge">
            <meta name="viewport" content="width=device-width, initial-scale=1,user-scalable=no">
        )"}
    };

    bustache::format format(fileContent);
    preprocessedStream << format(data).context(context).escape(bustache::escape_html);
}

oatpp::String StaticFileManager::getFile(const oatpp::String &filename)
{

    // Check cache
    auto it = m_cache.find(filename);
    if (it != m_cache.end())
    {
        return it->second;
    }

    // Check error count
    if (m_errorCount[filename] >= MAX_ERROR_COUNT)
    {
        OATPP_LOGD("StaticFileManager", "Failed to load file multiple times: %s", filename->c_str());
        return nullptr;
    }

    // Check loading rate limit
    if (m_loadingCounter >= MAX_FILE_LOADS_PER_SECOND)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastLoadTime).count();

        if (elapsed < 1000)
        {
            // Reached loading rate limit. Wait for some time.
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - elapsed));
            m_lastLoadTime = std::chrono::steady_clock::now();
        }
        m_loadingCounter = 0;
    }

    // Convert filename to std::string
    std::string fileNameStr = filename->c_str();

    // Check if the file name contains directories
    std::size_t lastSlashPos = fileNameStr.find_last_of("/");
    std::string filePath, fileDir;
    if (lastSlashPos != std::string::npos)
    {
        fileDir = fileNameStr.substr(0, lastSlashPos);
        filePath = "./static/" + fileDir;

// Create directory if it doesn't exist
#ifdef _WIN32
        _mkdir(filePath.c_str());
#else
        mkdir(filePath.c_str(), 0755);
#endif
    }

    // Append the file name to the file path
    filePath += "/" + fileNameStr.substr(lastSlashPos + 1);

    // Check if file exists
    if (access(filePath.c_str(), F_OK) == -1)
    {
        m_cache.erase(filename);
        return nullptr;
    }

    // Check if file was modified
    std::chrono::system_clock::time_point lastModifiedTime = getLastModifiedTime(filePath);
    if (lastModifiedTime > m_lastModifiedTimes[filename])
    {
        // File was modified. Re-load file.
        m_cache.erase(filename);
        m_lastModifiedTimes[filename] = lastModifiedTime;
    }

    // Read file content
    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
    if (!fileStream.is_open())
    {
        OATPP_LOGD("StaticFileManager", "Failed to open file: %s", filename->c_str());
        m_errorCount[filename]++;
        return nullptr;
    }
    oatpp::String fileContent;
    if (isHtmlFile(filename))
    {
        std::ostringstream preprocessedStream;
        preprocessHtmlFile(fileStream, preprocessedStream);
        fileContent = preprocessedStream.str();
        m_cache[filename] = fileContent;
    }
    else
    {
        std::ostringstream fileContentStream;
        fileContentStream << fileStream.rdbuf();
        fileStream.close();

        fileContent = fileContentStream.str();
        m_cache[filename] = fileContent;
    }
    m_loadingCounter++;

    return fileContent;
}

bool StaticFileManager::isHtmlFile(const oatpp::String &filename)
{
    // Get the extension of the filename
    std::string extension = filename->substr(filename->find_last_of(".") + 1);

    // Check if the extension is "html"
    return extension == "html";
}