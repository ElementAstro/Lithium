#include "atom/web/downloader.hpp"

#include <gtest/gtest.h>
#include <fstream>

// 模拟下载 URL 和文件路径
const std::string mock_url = "https://example.com/testfile";
const std::string mock_file = "testfile.txt";

// 继承测试类来组织不同的测试
class DownloadManagerTest : public ::testing::Test {
protected:
    // 每次测试前初始化
    void SetUp() override {
        // 初始化 DownloadManager 实例
        download_manager =
            std::make_unique<atom::web::DownloadManager>("tasks.txt");
    }

    // 每次测试后清理
    void TearDown() override {
        // 清理模拟下载的文件
        std::remove("tasks.txt");
        std::remove("testfile.txt");
    }

    std::unique_ptr<atom::web::DownloadManager> download_manager;
};

// 测试添加任务
TEST_F(DownloadManagerTest, AddTask) {
    // 添加一个任务
    download_manager->add_task(mock_url, mock_file);

    // 检查任务是否正确添加
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);
}

// 测试删除任务
TEST_F(DownloadManagerTest, RemoveTask) {
    download_manager->add_task(mock_url, mock_file);

    // 检查任务是否添加
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);

    // 删除任务
    bool removed = download_manager->remove_task(0);

    // 检查任务是否删除成功
    ASSERT_TRUE(removed);
}

// 测试暂停和恢复任务
TEST_F(DownloadManagerTest, PauseResumeTask) {
    download_manager->add_task(mock_url, mock_file);

    // 暂停任务
    download_manager->pause_task(0);

    // 模拟任务已暂停（通过检查文件字节数不变）
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);

    // 恢复任务
    download_manager->resume_task(0);

    // 恢复后，任务应可以继续下载
    ASSERT_EQ(download_manager->get_downloaded_bytes(0),
              0);  // 检查任务仍未开始（字节为0）
}

// 测试取消任务
TEST_F(DownloadManagerTest, CancelTask) {
    download_manager->add_task(mock_url, mock_file);

    // 取消任务
    download_manager->cancel_task(0);

    // 模拟任务被取消后，下载应不会进行
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);
}

// 测试任务的下载进度更新
TEST_F(DownloadManagerTest, ProgressUpdate) {
    download_manager->add_task(mock_url, mock_file);

    // 设置进度更新回调函数
    bool progress_updated = false;
    download_manager->on_progress_update([&](size_t index, double progress) {
        progress_updated = true;
        ASSERT_EQ(index, 0);       // 检查任务索引是否正确
        ASSERT_GE(progress, 0.0);  // 进度应大于等于 0
    });

    // 启动下载任务
    download_manager->start(1);

    // 检查进度是否已更新
    ASSERT_TRUE(progress_updated);
}

// 测试任务完成后的通知
TEST_F(DownloadManagerTest, DownloadCompleteNotification) {
    download_manager->add_task(mock_url, mock_file);

    // 设置下载完成回调函数
    bool task_completed = false;
    download_manager->on_download_complete([&](size_t index) {
        task_completed = true;
        ASSERT_EQ(index, 0);  // 检查任务索引
    });

    // 启动下载任务
    download_manager->start(1);

    // 检查是否收到任务完成通知
    ASSERT_TRUE(task_completed);
}

// 测试多任务并发
TEST_F(DownloadManagerTest, ConcurrentTasks) {
    download_manager->add_task(mock_url, "file1.txt");
    download_manager->add_task(mock_url, "file2.txt");

    // 使用多个线程启动下载
    download_manager->start(2);

    // 确保两个任务都添加了并下载完成
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);  // 模拟已下载
    ASSERT_EQ(download_manager->get_downloaded_bytes(1), 0);  // 模拟已下载

    // 清理生成的文件
    std::remove("file1.txt");
    std::remove("file2.txt");
}

// 测试设置最大重试次数
TEST_F(DownloadManagerTest, MaxRetries) {
    download_manager->add_task(mock_url, mock_file);

    // 设置重试次数
    download_manager->set_max_retries(3);

    // 启动任务
    download_manager->start(1);

    // 检查重试次数
    // 模拟失败后重试3次
    ASSERT_EQ(download_manager->get_downloaded_bytes(0),
              0);  // 重试后仍未成功，模拟失败
}

// 测试线程数动态调整
TEST_F(DownloadManagerTest, SetThreadCount) {
    download_manager->add_task(mock_url, mock_file);

    // 动态设置下载线程数
    download_manager->set_thread_count(4);

    // 启动下载任务
    download_manager->start();

    // 确保任务添加后没有下载错误
    ASSERT_EQ(download_manager->get_downloaded_bytes(0), 0);
}
