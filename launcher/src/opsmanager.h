/*  LithiumWebManagerApp
    Copyright (C) 2019 Robert Lancaster <rlancaste@gmail.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#ifndef OPSMANAGER_H
#define OPSMANAGER_H

#include "mainwindow.h"
#include "ui_opsmanager.h"
#include <QWidget>

class KConfigDialog;

namespace Ui
{
    class OpsManager;
}

class OpsManager : public QWidget
{
    Q_OBJECT

public:
    explicit OpsManager(MainWindow *parent = nullptr);
    ~OpsManager();

private:
    Ui::OpsManager *ui;
    MainWindow *parent;
    QString startupFilePath;

private slots:
    void updateFromCheckBoxes();
    void setLaunchAtStartup(bool launchAtStart);
    bool checkLaunchAtStartup();
    void toggleLaunchAtStartup();
};

#endif // OPSMANAGER_H
