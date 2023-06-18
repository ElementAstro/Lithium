#ifndef SERVERLAUNCHER_H
#define SERVERLAUNCHER_H

#include <string>
#include <atomic>
#include <memory>
#include <QThread>
#include <QJsonObject>
#include <QJsonValue>

class ServerLauncher : public QObject
{
    Q_OBJECT

public:
    ServerLauncher(const std::string &config_file_path, const std::string &log_file_path);
    ~ServerLauncher();

    void load_config(); // 从文件中加载配置
    bool is_running() const; // 返回服务器是否正在运行

    // 启动和停止服务器
    void run();
    void stop();

    std::unique_ptr<QJsonObject> _config; // 配置文件
    std::string _config_file_path; // 配置文件路径
    std::string _log_file_path; // 日志文件路径
    std::atomic<bool> _stop_requested; // 停止请求
    std::atomic<bool> _server_running; // 服务器是否正在运行

signals:
    void serverStatusChanged(bool running); // 服务器状态改变的信号
    void serverLogUpdated(const std::string &log); // 服务器输出日志信息的信号

private:
    class ServerThread; // 内部类，用于启动服务器

    std::unique_ptr<ServerThread> _thread; // 服务器线程
};
#endif // SERVERLAUNCHER_H
