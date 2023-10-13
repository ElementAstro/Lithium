/*  LithiumWebManagerApp
    Copyright (C) 2019 Robert Lancaster <rlancaste@gmail.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QMessageBox>
#include <KLocalizedString>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setApplicationVersion(LithiumWebManagerApp_VERSION);

    // This checks to see if you are root and warns you if you are.
    QString name = qgetenv("USER");
    if (name == "root")
    {
        QMessageBox::information(nullptr, "message", i18n("Please do not run LithiumWebManagerApp as sudo or root. This is not supported.  It will cause issues and is not good practice"));
    }

    MainWindow w;
    w.show();
    return a.exec();
}
