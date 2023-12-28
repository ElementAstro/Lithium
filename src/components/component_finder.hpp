#include <filesystem>
#include <vector>

class DirContainer
{
public:
    explicit DirContainer(const std::filesystem::path &path) : m_path(path)
    {
        if (std::filesystem::exists(m_path) && std::filesystem::is_directory(m_path))
        {
            for (const auto &entry : std::filesystem::directory_iterator(m_path))
            {
                if (entry.is_directory())
                {
                    m_subdirs.emplace_back(entry.path());
                }
                else
                {
                    m_files.emplace_back(entry.path());
                }
            }
        }
    }

    const std::filesystem::path &getPath() const
    {
        return m_path;
    }

    const std::vector<DirContainer> &getSubdirs() const
    {
        return m_subdirs;
    }

    const std::vector<std::filesystem::path> &getFiles() const
    {
        return m_files;
    }

    std::filesystem::path m_path;
    std::vector<DirContainer> m_subdirs;
    std::vector<std::filesystem::path> m_files;
};

void traverseDir(const std::filesystem::path &path, DirContainer &container)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            DirContainer subdir(entry.path());
            traverseDir(entry.path(), subdir);
            container.m_subdirs.emplace_back(std::move(subdir));
        }
        else
        {
            container.m_files.emplace_back(entry.path());
        }
    }
}

void printDir(const DirContainer &dir, int level = 0)
{
    for (int i = 0; i < level; ++i)
    {
        std::cout << "  ";
    }
    std::cout << "+ " << dir.getPath().filename() << std::endl;
    for (const auto &subdir : dir.getSubdirs())
    {
        printDir(subdir, level + 1);
    }
    for (const auto &file : dir.getFiles())
    {
        for (int i = 0; i < level + 1; ++i)
        {
            std::cout << "  ";
        }
        std::cout << "- " << file.filename() << std::endl;
    }
}

/*
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
        return -1;
    }

    std::filesystem::path path(argv[1]);
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        std::cerr << "Invalid path: " << argv[1] << std::endl;
        return -1;
    }

    DirContainer container(path);
    traverseDir(path, container);

    // Print the directory structure
    printDir(container);

    return 0;
}
*/