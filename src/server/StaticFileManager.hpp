#ifndef STATICFILEMANAGER_HPP
#define STATICFILEMANAGER_HPP

#include "oatpp/core/Types.hpp"
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

class StaticFileManager
{
private:
	static const int MAX_FILE_LOADS_PER_SECOND;
	static const int MAX_ERROR_COUNT;

	std::unordered_map<oatpp::String, oatpp::String> m_cache;
	std::unordered_map<oatpp::String, std::chrono::system_clock::time_point> m_lastModifiedTimes;
	std::unordered_map<oatpp::String, int> m_errorCount;
	std::unordered_map<oatpp::String, std::pair<oatpp::String, std::chrono::system_clock::time_point>> m_cacheWithTimestamp;

	std::atomic<int> m_loadingCounter;
	std::chrono::steady_clock::time_point m_lastLoadTime;

	bool isAllowedExtension(const oatpp::String &extension);
	std::chrono::system_clock::time_point getLastModifiedTime(const std::string &filePath);
	bool isHtmlFile(const oatpp::String &filename);
	void preprocessHtmlFile(std::ifstream &fileStream, std::ostringstream &preprocessedStream);

public:
	StaticFileManager();

	oatpp::String getFile(const oatpp::String &filename);
};

#endif // STATICFILEMANAGER_HPP