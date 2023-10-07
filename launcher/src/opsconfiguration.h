/*  LithiumWebManagerApp
    Copyright (C) 2019 Robert Lancaster <rlancaste@gmail.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/


#ifndef OPSCONFIGURATION_H
#define OPSCONFIGURATION_H

#include "mainwindow.h"
#include "ui_opsconfiguration.h"
#include <QWidget>

class KConfigDialog;

class MainWindow;


class OpsConfiguration : public QWidget
{
    Q_OBJECT

public:
    explicit OpsConfiguration(MainWindow *parent = nullptr);
    ~OpsConfiguration();

private:
    void displayInstallationStatus(bool installed);
    void displayGSCInstallationStatus(bool installed);
    MainWindow *parent;
    Ui::OpsConfiguration *ui;

private slots:
    void slotInstallRequirements();
    bool brewInstalled();
    bool gscInstalled();
    void updatePythonAndIndiwebInstallationStatus();
    void updateGSCInstallationStatus();
    void slotGSCInstallerFinished();
    void slotInstallGSC();
    void slotExtractGSC();
    void updateFromCheckBoxes();
};

#endif // OPSCONFIGURATION_H
