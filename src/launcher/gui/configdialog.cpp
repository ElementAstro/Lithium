#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // 设置对话框标题
    setWindowTitle("Modify Config");

    // 添加端口号的下拉列表
    for (int i = 3000; i <= 5000; ++i)
    {
        ui->comboBox_port->addItem(QString::number(i));
    }
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::setConfig(std::unique_ptr<QJsonObject> &config)
{
    _config = std::move(config);

    // 将当前的配置数据显示在对话框上
    ui->lineEdit_server_path->setText(_config->value("server_path").toString());
    ui->comboBox_port->setCurrentText(QString::number(_config->value("port").toInt()));
}

std::unique_ptr<QJsonObject> ConfigDialog::getConfig() const
{
    // 返回用户设置后的配置数据
    auto config = std::make_unique<QJsonObject>();
    config->insert("server_path", QJsonValue(ui->lineEdit_server_path->text()));
    config->insert("port", QJsonValue(ui->comboBox_port->currentText().toInt()));
    return std::move(config);
}

void ConfigDialog::on_pushButton_save_clicked()
{
    accept();
}

void ConfigDialog::on_pushButton_cancel_clicked()
{
    reject();
}
