#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

#ifdef _WIN32
const std::string PATH_SEPARATOR = "\\";
#else
const std::string PATH_SEPARATOR = "/";
#endif

namespace fs = std::filesystem;

std::string normalize_path(const std::string& path)
{
    std::string normalized_path = path;
    std::replace(normalized_path.begin(), normalized_path.end(), '/', PATH_SEPARATOR.front());
    std::replace(normalized_path.begin(), normalized_path.end(), '\\', PATH_SEPARATOR.front());
    return normalized_path;
}

void traverse_directories(const fs::path& directory, std::vector<std::string>& folders)
{
    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (entry.is_directory())
        {
            std::string folder_path = normalize_path(entry.path().string());
            folders.push_back(folder_path);
            traverse_directories(entry.path(), folders);
        }
    }
}

int main()
{
    fs::path root_directory = "../test";  // 替换为要遍历的目录路径

    std::vector<std::string> folders;
    traverse_directories(root_directory, folders);

    for (const auto& folder : folders)
    {
        std::cout << folder << std::endl;
    }

    return 0;
}
