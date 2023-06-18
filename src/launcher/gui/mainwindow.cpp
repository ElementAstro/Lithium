#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建ServerLauncher类的对象
    server_launcher_ = std::make_unique<ServerLauncher>("config.json", "server_log.txt");

    // 加载配置文件
    server_launcher_->load_config();

    // 将ServerLauncher类的对象中的信息显示在窗口上
    ui->label_config_file_path->setText(QString::fromStdString(server_launcher_->_config_file_path));
    ui->label_log_file_path->setText(QString::fromStdString(server_launcher_->_log_file_path));
    ui->checkBox_stop_requested->setChecked((bool)server_launcher_->_stop_requested);
    ui->checkBox_server_running->setChecked((bool)server_launcher_->_server_running);

    // 连接按钮的信号槽
    connect(ui->pushButton_run_server, &QPushButton::clicked, server_launcher_.get(), &ServerLauncher::run);
    connect(ui->pushButton_stop_server, &QPushButton::clicked, server_launcher_.get(), &ServerLauncher::stop);

    // 添加“修改配置”按钮的信号槽
    connect(ui->pushButton_modify_config, &QPushButton::clicked, this, &MainWindow::modifyConfig);

    // 显示服务器状态和日志信息
    ui->label_server_status->setText(QString("Server is %1").arg(server_launcher_->is_running() ? "Running" : "Stopped"));
    ui->textEdit_log->setPlainText(QString::fromStdString(getServerLog()));

    // 在ServerLauncher类中添加一个信号，当服务器状态改变时，触发该信号
    connect(server_launcher_.get(), &ServerLauncher::serverStatusChanged, this, &MainWindow::updateServerStatus);

    // 在ServerLauncher类中添加一个信号，当服务器输出新的日志信息时，触发该信号
    connect(server_launcher_.get(), &ServerLauncher::serverLogUpdated, this, &MainWindow::updateServerLog);

    // 自动隐藏弹窗并创建任务栏小图标
    connect(server_launcher_.get(), &ServerLauncher::serverStatusChanged, this, &MainWindow::updateTrayIcon);
    connect(ui->actionShow, &QAction::triggered, this, &MainWindow::showNormal);

    QSystemTrayIcon *tray_icon = new QSystemTrayIcon(this);
    tray_icon->setIcon(QIcon(":/images/icon.png"));
    tray_icon->setToolTip("Server Launcher");

    QMenu *tray_menu = new QMenu(this);
    tray_menu->addAction(ui->actionStop_Server);

    tray_icon->setContextMenu(tray_menu);
    tray_icon->show();
}

void MainWindow::modifyConfig()
{
    ConfigDialog dialog(this);

    // 将当前的配置文件数据传递给对话框
    dialog.setConfig(server_launcher_->_config);

    // 如果用户点击“OK”，则更新配置文件
    if (dialog.exec() == QDialog::Accepted)
    {
        server_launcher_->_config = dialog.getConfig();

        // 更新配置文件并重新加载
        writeConfigToFile("config.json", server_launcher_->_config);
        server_launcher_->load_config();
    }
}

void MainWindow::updateServerStatus(bool running)
{
    if (running)
    {
        ui->label_server_status->setText("Server is Running");
        QMessageBox::information(this, "Server Status", "Server has started successfully!");
    }
    else
    {
        ui->label_server_status->setText("Server is Stopped");
        QMessageBox::information(this, "Server Status", "Server has stopped successfully.");
    }

    // 隐藏弹窗
    hide();
}

void MainWindow::updateServerLog(const std::string &log)
{
    ui->textEdit_log->moveCursor(QTextCursor::End);
    ui->textEdit_log->insertPlainText(QString::fromStdString(log));
}

void MainWindow::updateTrayIcon(bool running)
{
    QSystemTrayIcon *tray_icon = qobject_cast<QSystemTrayIcon *>(sender());

    if (running)
    {
        tray_icon->setToolTip("Server is Running");
        tray_icon->setIcon(QIcon(":/images/icon_running.png"));
    }
    else
    {
        tray_icon->setToolTip("Server is Stopped");
        tray_icon->setIcon(QIcon(":/images/icon_stopped.png"));
    }
}

void MainWindow::on_actionStop_Server_triggered()
{
    server_launcher_->stop();
}

std::string MainWindow::getServerLog() const
{
    std::ifstream ifs("server_log.txt");
    if (ifs)
    {
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }
    else
    {
        return "Failed to open server log file.";
    }
}
