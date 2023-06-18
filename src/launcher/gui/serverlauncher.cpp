#include "serverlauncher.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>

ServerLauncher::ServerLauncher(const std::string &config_file_path, const std::string &log_file_path)
    : QObject(nullptr)
    , _config_file_path(config_file_path)
    , _log_file_path(log_file_path)
    , _stop_requested(false)
    , _server_running(false)
{
}

ServerLauncher::~ServerLauncher()
{
}

void ServerLauncher::load_config()
{
    std::ifstream file(_config_file_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open config file '" << _config_file_path << "'." << std::endl;
        return;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    auto json = QJsonDocument::fromJson(ss.str().c_str());

    if (!json.isObject())
    {
        std::cerr << "Invalid JSON format in config file." << std::endl;
        return;
    }

    _config = std::make_unique<QJsonObject>(json.object());
}

bool ServerLauncher::is_running() const
{
    return _server_running;
}

void ServerLauncher::run()
{
    // 如果服务器已经在运行，则退出
    if (_server_running)
    {
        return;
    }

    _server_running = true;
    emit serverStatusChanged(_server_running);

    std::ofstream log_file(_log_file_path, std::ios_base::app);
    if (!log_file.is_open())
    {
        std::cerr << "Failed to open log file '" << _log_file_path << "'." << std::endl;
        _server_running = false;
        emit serverStatusChanged(_server_running);
        return;
    }

    QTcpServer server;
    if (!server.listen(QHostAddress::Any, _config->value("port").toInt()))
    {
        std::cerr << "Failed to listen on port " << _config->value("port").toInt() << "." << std::endl;
        _server_running = false;
        emit serverStatusChanged(_server_running);
        return;
    }

    while (!_stop_requested)
    {
        if (server.hasPendingConnections())
        {
            auto client = server.nextPendingConnection();
            std::stringstream ss;
            ss << "New connection from " << client->peerAddress().toString().toStdString() << ":" << client->peerPort() << std::endl;
            std::cout << ss.str();
            log_file << ss.str();

            client->disconnectFromHost();
        }

        QThread::sleep(1);
    }

    server.close();
    _server_running = false;
    emit serverStatusChanged(_server_running);

    log_file.close();
}

void ServerLauncher::stop()
{
    _stop_requested = true;

    // 等待服务器线程结束
    _thread->wait();

    _stop_requested = false;
}

// ServerLauncher类中的内部类，用于启动服务器
class ServerLauncher::ServerThread : public QThread
{
public:
    ServerThread(ServerLauncher *launcher) : _launcher(launcher) {}

    void run() override { _launcher->run(); }

private:
    ServerLauncher *_launcher;
};
