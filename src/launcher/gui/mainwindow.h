#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <memory>
#include "ui_mainwindow.h"
#include "serverlauncher.h"
#include "configdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void modifyConfig();                          // 修改配置文件
    void updateServerStatus(bool running);        // 更新服务器状态
    void updateServerLog(const std::string &log); // 更新服务器日志
    void updateTrayIcon(bool running);            // 更新任务栏小图标
    void on_actionStop_Server_triggered();        // 停止服务器

private:
    Ui::MainWindow *ui;
    std::unique_ptr<ServerLauncher> server_launcher_;
    std::string getServerLog() const; // 获取服务器日志
};
#endif // MAINWINDOW_H
