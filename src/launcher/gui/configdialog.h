#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void setConfig(std::unique_ptr<QJsonObject> &config); // 设置当前的配置文件数据
    std::unique_ptr<QJsonObject> getConfig() const; // 获取用户设置后的配置文件数据

private slots:
    void on_pushButton_save_clicked(); // 保存设置并退出对话框
    void on_pushButton_cancel_clicked(); // 取消设置并退出对话框

private:
    Ui::ConfigDialog *ui;
    std::unique_ptr<QJsonObject> _config;
};

#endif // CONFIGDIALOG_H
