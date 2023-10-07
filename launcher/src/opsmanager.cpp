/*  LithiumWebManagerApp
    Copyright (C) 2019 Robert Lancaster <rlancaste@gmail.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "opsmanager.h"
#include "ui_opsmanager.h"
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <KLocalizedString>

OpsManager::OpsManager(MainWindow *parent) : QWidget(parent)
{
   this->parent = parent;
   ui = new Ui::OpsManager;
   ui->setupUi(this);

#ifdef Q_OS_OSX
   startupFilePath = QDir::homePath() + "/Library/LaunchAgents/" + "com.INDIWebManager.LaunchAgent.plist";
#else
   startupFilePath = "/etc/systemd/system/LithiumWebManagerApp.service";
#endif

   //This connects all the default checkboxes to the update function.
   connect(ui->kcfg_ManagerPortNumberDefault, &QCheckBox::clicked, this, &OpsManager::updateFromCheckBoxes);
   connect(ui->kcfg_LogFilePathDefault, &QCheckBox::clicked, this, &OpsManager::updateFromCheckBoxes);

   //This connects the launch at startup checkbox to the toggle function
   connect(ui->launchAtStartup, &QCheckBox::clicked, this, &OpsManager::toggleLaunchAtStartup);

   //This waits a moment for the kconfig to load the options, then sets the Line Edits to read only appropriagely
   QTimer::singleShot(100, this, &OpsManager::updateFromCheckBoxes);

}

OpsManager::~OpsManager()
{
    delete ui;
}

/*
 * This method enables the functionality of the default buttons.
 * The line edits are disabled as long as the default button is checked.
 * If the user unchecks the button, it changes to the stored value
 * If the user checks the button, it changes to the default value.
 */
void OpsManager::updateFromCheckBoxes()
{
    ui->kcfg_ManagerPortNumber->setReadOnly(ui->kcfg_ManagerPortNumberDefault->isChecked());
    ui->kcfg_LogFilePath->setReadOnly(ui->kcfg_LogFilePathDefault->isChecked());

    if(ui->kcfg_ManagerPortNumberDefault->isChecked())
         ui->kcfg_ManagerPortNumber->setText(parent->getDefault("ManagerPortNumber"));
    else
         ui->kcfg_ManagerPortNumber->setText(Options::managerPortNumber());

    if(ui->kcfg_LogFilePathDefault->isChecked())
         ui->kcfg_LogFilePath->setText(parent->getDefault("LogFilePath"));
    else
         ui->kcfg_LogFilePath->setText(Options::logFilePath());

    ui->launchAtStartup->setChecked(checkLaunchAtStartup());

}

/*
 * This creates and installs or uninstalls the startup file to the appropriate location for your operating system.
 * For Linux it requires sudo and an admin password, but not for OS X.
 */
void OpsManager::setLaunchAtStartup(bool launchAtStart)
{
    if(launchAtStart)
    {
        QString fileText = "";

    #ifdef Q_OS_OSX
        fileText = "" \
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n" \
        "<plist version=\"1.0\">\n" \
        "<dict>\n" \
        "    <key>Disabled</key>\n" \
        "    <false/>\n" \
        "    <key>Label</key>\n" \
        "    <string>INDI Web Manager App</string>\n" \
        "    <key>ProgramArguments</key>\n" \
        "    <array>\n" \
        "        <string>" + QCoreApplication::applicationFilePath() + "</string>\n" \
        "    </array>\n" \
        "    <key>RunAtLoad</key>\n" \
        "    <true/>\n" \
        "</dict>\n" \
        "</plist>";

        QFile file( startupFilePath );
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            stream << fileText << endl;
        }

    #else
        bool ok = false;
        int delay = QInputDialog::getInt(nullptr, i18n("Get Delay for Startup"),
                                                 i18n("Your system probably needs a delay at startup for the Window Manager to load, how long would you like?:"),
                                                 20, 0, 100, 1, &ok);
        if(!ok)
            delay = 0;
        fileText = QString("" \
        "[Unit]\n" \
        "Description=INDI Web Manager App\n" \
        "After=multi-user.target\n" \
        "\n" \
        "[Service]\n" \
        "ExecStartPre=/bin/sleep %1\n" \
        "Environment=\"DISPLAY=:0\"\n" \
        "Environment=XAUTHORITY=" + QDir::homePath() + "/.Xauthority\n" \
        "Type=idle\n" \
        "User=" + qgetenv("USER") + "\n" \
        "ExecStart=" + QCoreApplication::applicationFilePath() + "\n" \
        "\n" \
        "[Install]\n" \
        "WantedBy=multi-user.target").arg(delay);

        QString tempFile =  QDir::homePath() + "/LithiumWebManagerApp.service";
        QFile file2(tempFile);
        if ( file2.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file2 );
            stream << fileText << endl;
        }
        ok = false;
        QString password = QInputDialog::getText(nullptr, "Get Password",
                                                 i18n("To create the service file and enable the service, we need to use sudo. \nYour admin password please:"), QLineEdit::Normal,
                                                 "", &ok);
        if (ok && !password.isEmpty())
        {
            QProcess loadService;
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S mv " + tempFile + " " + startupFilePath);
            loadService.waitForFinished();
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S chmod 644 " + startupFilePath);
            loadService.waitForFinished();
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S systemctl daemon-reload");
            loadService.waitForFinished();
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S systemctl enable LithiumWebManagerApp.service");
            loadService.waitForFinished();
        }
        else
        {
            QMessageBox::information(nullptr, "message", i18n("Since we cannot get your sudo password, we can't complete your request.  You can try clicking the button again and entering your password, or manually do it using the following steps in a Terminal."));

            QMessageBox::information(nullptr, "message", "sudo mv " + QDir::homePath() + "/LithiumWebManagerApp.service /etc/systemd/system/LithiumWebManagerApp.service\n" \
                                                         "sudo chmod 644 /etc/systemd/system/LithiumWebManagerApp.service\n" \
                                                         "sudo systemctl daemon-reload\n" \
                                                         "sudo systemctl enable LithiumWebManagerApp.service\n");
        }
    #endif

    }
    else
    {
    #ifdef Q_OS_OSX
        QFile::remove(startupFilePath);
    #else
        bool ok;
        QString password = QInputDialog::getText(nullptr, "Get Password",
                                                 i18n("To delete the service file and stop the service, we need to use sudo. \nYour admin password please:"), QLineEdit::Normal,
                                                 "", &ok);
        if (ok && !password.isEmpty())
        {
            QProcess loadService;
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S rm " + startupFilePath);
            loadService.waitForFinished();
            loadService.start("bash", QStringList() << "-c" << "echo " + password + " | sudo -S systemctl daemon-reload");
            loadService.waitForFinished();

        }
        else
        {
            QMessageBox::information(nullptr, "message", i18n("Since we cannot get your sudo password, we can't complete your request.  You can try clicking the button again and entering your password, or manually do it using the following steps in a Terminal."));

            QMessageBox::information(nullptr, "message", "sudo rm /etc/systemd/system/LithiumWebManagerApp.service\n" \
                                                         "sudo systemctl daemon-reload\n");
        }
    #endif
    }
    ui->launchAtStartup->setChecked(checkLaunchAtStartup());
}

/*
 * This toggles installing or uninstalling the startup file, depending on whether it is installed already.
 */
void OpsManager::toggleLaunchAtStartup()
{
    setLaunchAtStartup(!checkLaunchAtStartup());
}

/*
 * This method checks to see if the startup file exists in its default location.
 */
bool OpsManager::checkLaunchAtStartup()
{
    return QFileInfo(startupFilePath).exists();
}
