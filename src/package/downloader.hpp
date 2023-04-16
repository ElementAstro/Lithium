#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <vector>

struct DownloadTask
{
    std::string url;
    std::string filepath;
    bool completed = false;
};

class DownloadManager
{
public:
    DownloadManager(const std::string &task_file);
    ~DownloadManager();
    void add_task(const std::string &url, const std::string &filepath);
    bool remove_task(size_t index);
    void start();

private:
    std::string task_file_;
    std::vector<DownloadTask> tasks_;

    static void download_task(DownloadManager *manager, size_t index);
    void download_task(size_t index);
};

#endif // !DOWNLOAD_MANAGER_H
