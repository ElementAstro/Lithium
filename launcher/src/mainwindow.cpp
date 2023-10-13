/*  LithiumWebManagerApp
    Copyright (C) 2019 Robert Lancaster <rlancaste@gmail.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include "opsconfiguration.h"
#include "opsmanager.h"
#include <QSizePolicy>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <KConfigDialog>
#include <KLocalizedString>
#include <QHostInfo>
#include <QWhatsThis>
#include <QSystemTrayIcon>
#include <QWidgetAction>
#include <QListWidget>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    setWindowTitle(i18n("INDI Web Manager App"));
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/media/images/indi_logo.png"));

    QMenu *trayIconMenu = new QMenu(this);

    managerStatusinTray = new QAction("Web Manager: Offline", this);
    managerStatusinTray->setIcon(QIcon(":/media/icons/red.png"));
    managerStatusinTray->setIconVisibleInMenu(true);
    trayIconMenu->addAction(managerStatusinTray);

    serverStatusinTray = new QAction("INDI Server: Offline", this);
    serverStatusinTray->setIcon(QIcon(":/media/icons/red.png"));
    serverStatusinTray->setIconVisibleInMenu(true);
    trayIconMenu->addAction(serverStatusinTray);

    trayIconMenu->addSeparator();

    QAction *hideAction = new QAction("Hide App Window", this);
    trayIconMenu->addAction(hideAction);
    connect(hideAction, &QAction::triggered, this, &QMainWindow::hide);

    QAction *showAction = new QAction("Show App Window", this);
    trayIconMenu->addAction(showAction);
    connect(showAction, &QAction::triggered, this, &MainWindow::showAndRaise);
    trayIconMenu->addSeparator();

    QAction *openAction = new QAction("Open Web Manager", this);
    trayIconMenu->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &MainWindow::openWebManager);

    QAction *quitAction = new QAction("Quit", this);
    trayIconMenu->addAction(quitAction);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/media/images/indi_logo.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r)
            {
        if (r == QSystemTrayIcon::Trigger)
            showAndRaise(); });
    trayIcon->show();

    // Note: This is for the tooltips AND the What's this
    // I tried to use stylesheets, but what's this did not listen.
    QPalette p = qApp->palette();
    p.setColor(QPalette::ColorRole::ToolTipBase, Qt::black);
    p.setColor(QPalette::ColorRole::ToolTipText, Qt::yellow);
// This is strange that the button text color didn't come through!
#ifdef Q_OS_OSX
    p.setColor(QPalette::ColorRole::ButtonText, QApplication::palette().text().color());
#endif
    qApp->setPalette(p);

    // This should make the icons look good no matter what the theme or mode of the system is.
    int backgroundBrightness = this->palette().color(QWidget::backgroundRole()).lightness();
    if (backgroundBrightness < 100)
    {
        ui->stopWebManager->setIcon(QIcon(":/media/icons/media-playback-stop-dark.svg"));
        ui->restartWebManager->setIcon(QIcon(":/media/icons/media-playback-start-dark.svg"));
        ui->configureWebManager->setIcon(QIcon(":/media/icons/configure-dark.svg"));
        ui->stopINDIServer->setIcon(QIcon(":/media/icons/media-playback-stop-dark.svg"));
        ui->startINDIServer->setIcon(QIcon(":/media/icons/media-playback-start-dark.svg"));
        ui->configureINDIServer->setIcon(QIcon(":/media/icons/configure-dark.svg"));
        ui->copyManagerToClipboard->setIcon(QIcon(":/media/icons/edit-copy-dark.svg"));
        ui->copyServerToClipboard->setIcon(QIcon(":/media/icons/edit-copy-dark.svg"));
    }
    else
    {
        ui->stopWebManager->setIcon(QIcon(":/media/icons/media-playback-stop.svg"));
        ui->restartWebManager->setIcon(QIcon(":/media/icons/media-playback-start.svg"));
        ui->configureWebManager->setIcon(QIcon(":/media/icons/configure.svg"));
        ui->stopINDIServer->setIcon(QIcon(":/media/icons/media-playback-stop.svg"));
        ui->startINDIServer->setIcon(QIcon(":/media/icons/media-playback-start.svg"));
        ui->configureINDIServer->setIcon(QIcon(":/media/icons/configure.svg"));
        ui->copyManagerToClipboard->setIcon(QIcon(":/media/icons/edit-copy.svg"));
        ui->copyServerToClipboard->setIcon(QIcon(":/media/icons/edit-copy.svg"));
    }

    // This makes the buttons look nice on OS X.
    /*
    ui->openWebManager->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->whatsThis->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->stopWebManager->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->restartWebManager->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->configureWebManager->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->configureINDIServer->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->startINDIServer->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    ui->stopINDIServer->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    */

    // This will hide the log and capture the window size to make the later algorithm for showing/hiding work properly.
    ui->logViewer->setVisible(false);
    this->adjustSize();
    minimumWindowSize = this->window()->size();

    // These set up the actions of the controls in the Main Window.
    connect(ui->configureWebManager, &QPushButton::clicked, this, &MainWindow::showPreferences);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::showPreferences);
    connect(ui->restartWebManager, &QPushButton::clicked, this, &MainWindow::startWebManager);
    connect(ui->stopWebManager, &QPushButton::clicked, this, &MainWindow::stopWebManager);
    connect(ui->configureINDIServer, &QPushButton::clicked, this, &MainWindow::openWebManager);
    connect(ui->startINDIServer, &QPushButton::clicked, this, &MainWindow::startINDIServer);
    connect(ui->stopINDIServer, &QPushButton::clicked, this, &MainWindow::stopINDIServer);
    connect(ui->showLog, &QPushButton::toggled, this, &MainWindow::setLogVisible);
    connect(ui->actionAbout, &QAction::triggered, this, []()
            {
        QMessageBox about;
        about.setIconPixmap(QPixmap(":/media/images/indi_logo.png"));
        about.setText(i18n("<html>INDI Web Manager App<br>&nbsp;&nbsp;© 2019 Robert Lancaster<br>&nbsp;&nbsp;Version: %1<br>&nbsp;&nbsp;Build: %2<br><br>Please see the Github page:<br><a href=https://github.com/rlancaste/LithiumWebManagerApp>https://github.com/rlancaste/LithiumWebManagerApp</a> <br>for details and source code.</html>").arg(QString(LithiumWebManagerApp_VERSION)).arg(QString(INDI_WEB_MANAGER_APP_BUILD_TS)));
        about.exec(); });
    connect(ui->ipListDisplay, &QListWidget::itemSelectionChanged, this, [this]()
            {
                QString host = ui->ipListDisplay->currentItem()->text();
                Options::setManagerHostNameOrIP(host);
                ui->hostDisplay->setText(host);
                ui->hostDisplay->setCursorPosition(0);
                ui->displayWebManagerPath->setText(getWebManagerURL());
                ui->displayWebManagerPath->setCursorPosition(0);
                ui->ipInformation->setText(ui->ipListDisplay->currentItem()->toolTip());
                checkINDIServerStatus(); // This will update the address for the indi server in the box
            });

    // These two connections enable the clipboard copy buttons
    connect(ui->copyManagerToClipboard, &QPushButton::clicked, this, [this]()
            { qApp->clipboard()->setText(ui->displayWebManagerPath->text()); });
    connect(ui->copyServerToClipboard, &QPushButton::clicked, this, [this]()
            { qApp->clipboard()->setText(ui->displayINDIServerPath->text()); });

    // These set up the actions in the help menu
    connect(ui->actionINDI_Details, &QAction::triggered, this, []()
            { QDesktopServices::openUrl(QUrl("https://www.indilib.org")); });

    connect(ui->actionINDI_Forum, &QAction::triggered, this, []()
            { QDesktopServices::openUrl(QUrl("https://www.indilib.org/forum.html")); });

    connect(ui->actionINDI_Web_Details, &QAction::triggered, this, []()
            { QDesktopServices::openUrl(QUrl("https://github.com/knro/indiwebmanager")); });

    connect(ui->actionINDI_Clients, &QAction::triggered, this, []()
            { QDesktopServices::openUrl(QUrl("https://www.indilib.org/about/clients.html")); });

    connect(ui->actionOS_X_Build_Script, &QAction::triggered, this, []()
            { QDesktopServices::openUrl(QUrl("https://github.com/rlancaste/kstars-on-osx-craft")); });

    connect(ui->actionWhat_s_This_2, &QAction::triggered, this, []()
            { QWhatsThis::enterWhatsThisMode(); });

    connect(ui->whatsThis, &QPushButton::clicked, this, []()
            { QWhatsThis::enterWhatsThisMode(); });

    // This sets up the Web Manager to launch in your favorite browser using either the host name or IP Address
    connect(ui->openWebManager, &QPushButton::clicked, this, &MainWindow::openWebManager);

    // This sets up the INDI Logo to look nice in the app.
    ui->INDILogo->setPixmap(QPixmap(":/media/images/indi_logo.png"));

    // This sets up a timer to check the status of the IP Address List at 10 second intervals when it is running
    serverMonitor.setInterval(10000);
    connect(&serverMonitor, &QTimer::timeout, this, &MainWindow::updateIPAddressList);
    serverMonitor.start();

    // This sets up a timer to check the status of the INDI Server at 1 second intervals when it is running.
    serverMonitor.setInterval(1000);
    connect(&serverMonitor, &QTimer::timeout, this, &MainWindow::checkINDIServerStatus);

    // This shows the main window.  For some reason if you don't do this before the check on the next line, it changes the window layout.
    this->show();

    // This will initially set the status displays to false.
    displayServerStatusOnline(false);
    displayManagerStatusOnline(false);

    // This will check if another Web Manager is running on this computer, and kill it if desired.
    if (isWebManagerOnline())
    {
        if (QMessageBox::question(nullptr, "Message", i18n("Alert, an INDI Webmanager is already running on this computer.  Do you want to quit it?")) == QMessageBox::Yes)
        {
            QProcess killWebManager;
            QStringList killParams;
            killParams << "Python"
                       << "indi-web"
                       << "indiserver";
            killWebManager.start("/usr/bin/killall", killParams);
            killWebManager.waitForFinished(300);
        }
    }

    // This will finish setting up the Web Manager and launch it if the requirements are installed and auto launch is selected.
    if (pythonInstalled() && indiWebInstalled())
    {
        updateSettings();
        if (Options::autoLaunchManager())
            startWebManager();
    }
    else
    {
#ifdef Q_OS_OSX
        QMessageBox::information(nullptr, "message", i18n("Please configure the INDI Web Manager.  The Preferences Dialog will now open. \n\nHomebrew, Python, and INDIWeb need to be installed and configured to use this program.  \n\nJust click the installer button in the Preferences Dialog to get started."));
#else
        QMessageBox::information(nullptr, "message", i18n("Please configure the INDI Web Manager.  The Preferences Dialog will now open. \n\nPython, Pip, and INDIWeb need to be installed and configured to use this program.  \n\nINDIWeb can be installed on Linux using the installer button in the Preferences Dialog, but python and pip must be installed from the command line."));
#endif
        showPreferences();
    }

    // This waits a moment for the options to load, and once they do, if the AutoHide function is selected, it autohides the window.
    QTimer::singleShot(10, this, [this]()
                       {
        if(Options::autoHideManagerApp())
            this->hide(); });
}

// This deletes the ui variable on shutdown
MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * This method is used to get the default values for the INDI Web Server settings on different systems.
 * It also is used to get automatically generated values.
 */
QString MainWindow::getDefault(QString option)
{
    QString snap = QProcessEnvironment::systemEnvironment().value("SNAP");
    QString flat = QProcessEnvironment::systemEnvironment().value("FLATPAK_DEST");

    // This is the folder that python is installed or symlinked to.
    if (option == "PythonExecFolder")
    {
#ifdef Q_OS_OSX
        // Note this is the Path where python3 gets symlinked by homebrew.
        return "/usr/local/opt/python/libexec/bin";
#endif
        if (flat.isEmpty() == false)
            return flat + "/bin/";
        else
            return snap + "/usr/bin/";
    }

    // This is the Path to the indiweb executable.  It is where indi-web is installed, typically in the Python Base User Directory.
    else if (option == "indiwebPath")
    {
#ifdef Q_OS_OSX
        return "/usr/local/bin/indi-web";
#endif
        return QDir::homePath() + "/.local/bin/indi-web";
    }

    // This is the Path to the GSC data folder.  It includes gsc at the end.
    else if (option == "GSCPath")
    {
#ifdef Q_OS_OSX
        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(), QStandardPaths::LocateDirectory) + "LithiumWebManagerApp/gsc";
#endif
        if (flat.isEmpty() == false)
            return flat + "/share/GSC";
        else
            return snap + "/usr/share/GSC";
    }

    // This is the path to the INDI Prefix.  This is the App bundle loction on OS X
    if (option == "INDIPrefix")
    {
#ifdef Q_OS_OSX
        QString appPath = QCoreApplication::applicationDirPath();
        return QDir(appPath + "/../../").absolutePath();
#endif
        return "";
    }

    // This is the path to the folder indiserver is present in, not the indiserver itself.
    if (option == "INDIServerPath")
    {
#ifdef Q_OS_OSX
        return QDir(QCoreApplication::applicationDirPath()).absolutePath();
#endif
        if (flat.isEmpty() == false)
            return flat + "/bin/";
        else
            return snap + "/usr/bin/";
    }

    // This is the folder of the indi drivers and xml files.
    else if (option == "INDIDriversPath")
    {
#ifdef Q_OS_OSX
        QString appPath = QCoreApplication::applicationDirPath();
        return QDir(appPath + "/../Resources/DriverSupport").absolutePath();
#elif defined(Q_OS_LINUX)
        if (flat.isEmpty() == false)
            return flat + "/share/indi";
        else
            return snap + "/usr/share/indi";
#else
        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, "indi", QStandardPaths::LocateDirectory);
#endif
    }

    // This is the default path to the INDI config files.
    else if (option == "INDIConfigPath")
        return QDir::homePath() + "/.indi";

    // This is the path to the folder indiserver is present in, not the indiserver itself.
    if (option == "GPhotoIOLIBS")
    {
#ifdef Q_OS_OSX
        QString appPath = QCoreApplication::applicationDirPath();
        return QDir(appPath + "/../Resources/DriverSupport/gphoto/IOLIBS").absolutePath();
#endif
        return "";
    }

    // This is the path to the folder indiserver is present in, not the indiserver itself.
    if (option == "GPhotoCAMLIBS")
    {
#ifdef Q_OS_OSX
        QString appPath = QCoreApplication::applicationDirPath();
        return QDir(appPath + "/../Resources/DriverSupport/gphoto/CAMLIBS").absolutePath();
#endif
        return "";
    }

    // This is the name of the computer on the local network.
    else if (option == "ManagerHostNameOrIP")
        return QHostInfo::localHostName();

    // This is the port the INDI Web Manager will be hosted on.
    else if (option == "ManagerPortNumber")
        return "8624";

    // This is the path for the log file.
    else if (option == "LogFilePath")
        return QDir::homePath() + "/.indi/webmanagerlog.txt";

    return QString();
}

/*
 * This gets the list of addresses available for this computer
 */
void MainWindow::updateIPAddressList()
{
    QList<QHostAddress> newIPList = QNetworkInterface::allAddresses();
    bool changed = false;
    if (newIPList.count() == oldIPList.count())
    {
        for (int i = 0; i < newIPList.count(); i++)
            if (oldIPList.at(i).toString() != newIPList.at(i).toString())
                changed = true;
    }
    else
        changed = true;

    if (changed)
    {
        oldIPList = newIPList;
        ui->ipListDisplay->clear();
        QListWidgetItem *hostItem = new QListWidgetItem();
        hostItem->setText(QHostInfo::localHostName());
        hostItem->setToolTip("Local Hostname");
        hostItem->setTextColor(Qt::green);
        ui->ipListDisplay->addItem(hostItem);
        for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces())
        {
            QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
            for (const QNetworkAddressEntry &address : addressEntries)
            {
                if (address.ip().protocol() != QAbstractSocket::IPv4Protocol)
                    addressEntries.removeOne(address);
            }
            if (addressEntries.size() > 0)
            {
                QString type = "Unknown";
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
                if (interface.type() == QNetworkInterface::Unknown)
                    type = "Unknown";
                if (interface.type() == QNetworkInterface::Loopback)
                    type = "Localhost";
                if (interface.type() == QNetworkInterface::Ethernet)
                    type = "Ethernet";
                if (interface.type() == QNetworkInterface::Wifi)
                    type = "Wifi";
                if (interface.type() == QNetworkInterface::Virtual)
                    type = "Virtual";
                if (interface.type() == QNetworkInterface::Ppp)
                    type = "Point to Point Protocol";
                if (interface.type() == QNetworkInterface::Slip)
                    type = "Serial Line IP";
#else
                if (interface.name().startsWith("eth"))
                    type = "Ethernet";
                if (interface.name().startsWith("en"))
                    type = "Ethernet";
                if (interface.name().startsWith("wl"))
                    type = "Wifi";
                if (interface.name().startsWith("ww"))
                    type = "Wifi";
                if (interface.name().startsWith("sl"))
                    type = "Serial Line IP";
                if (interface.name().startsWith("lo"))
                    type = "Localhost";
                if (interface.name().startsWith("ppp"))
                    type = "Point to Point Protocol";
                if (interface.name().startsWith("vm"))
                    type = "Virtual Machine IP";
                if (interface.name().startsWith("vbox"))
                    type = "Virtual Machine IP";
#endif
                for (const QNetworkAddressEntry &address : addressEntries)
                {
                    QListWidgetItem *newItem = new QListWidgetItem();
                    newItem->setText(address.ip().toString());
                    newItem->setToolTip(interface.name() + ", " + type);
                    QString firstOctet = address.ip().toString().split(".").first();
                    if (!address.ip().isLoopback() || firstOctet == "172" || firstOctet == "192")
                        newItem->setTextColor(Qt::green);
                    else
                    {
                        newItem->setTextColor(Qt::blue);
                    }
                    ui->ipListDisplay->addItem(newItem);
                }
            }
        }

        QList<QListWidgetItem *> customItemList = ui->ipListDisplay->findItems(Options::customHostNameOrIP(), Qt::MatchExactly);
        if (customItemList.size() == 0 && Options::customHostNameOrIP() != "")
        {
            QListWidgetItem *customItem = new QListWidgetItem();
            customItem->setText(Options::customHostNameOrIP());
            customItem->setToolTip("Custom Host/IP");
            customItem->setTextColor(Qt::blue);
            ui->ipListDisplay->addItem(customItem);
        }

        QList<QListWidgetItem *> currentItemList = ui->ipListDisplay->findItems(Options::managerHostNameOrIP(), Qt::MatchExactly);
        if (currentItemList.count() == 1)
            ui->ipListDisplay->setCurrentItem(currentItemList.first());
        else
        {
            ui->ipListDisplay->setCurrentItem(ui->ipListDisplay->item(0));
        }
    }
}

/*
 * This sets up the Web Manager to launch in your favorite browser using either the host name or IP Address
 */
void MainWindow::openWebManager()
{
    QUrl url(getWebManagerURL());
    QDesktopServices::openUrl(url);
}

void MainWindow::showAndRaise()
{
    show();
    raise();
}

/*
 * This gets the URL to the INDI Web Manager using the users desired method.
 */
QString MainWindow::getWebManagerURL()
{
    return QString("http://%1:%2").arg(Options::managerHostNameOrIP()).arg(Options::managerPortNumber());
}

/*
 * This gets the URL to the INDI Server using the users desired method.
 */
QString MainWindow::getINDIServerURL(QString port)
{
    return QString("%1:%2").arg(Options::managerHostNameOrIP()).arg(port);
}

/*
 * This method detects whether python is installed.
 */
bool MainWindow::pythonInstalled(QString pythonExecFolder)
{

    return QFileInfo(pythonExecFolder + "/python").exists() || QFileInfo(pythonExecFolder + "/python2").exists() || QFileInfo(pythonExecFolder + "/python3").exists();
}

bool MainWindow::pythonInstalled()
{
    return (pythonInstalled(Options::pythonExecFolder()));
}

/*
 * This method detects whether pip is installed.
 */
bool MainWindow::pipInstalled()
{
    // Note, I had to add the last set for /usr/local because some people have python installed in /usr/bin and pip installed in /usr/local/bin
    return QFileInfo(Options::pythonExecFolder() + "/pip").exists() || QFileInfo(Options::pythonExecFolder() + "/pip2").exists() || QFileInfo(Options::pythonExecFolder() + "/pip3").exists() || QFileInfo("/usr/local/bin/pip").exists() || QFileInfo("/usr/local/bin/pip2").exists() || QFileInfo("/usr/local/bin/pip3").exists();
}

/*
 * This method detects whether indi-web is installed in either python2 or python3.
 */
bool MainWindow::indiWebInstalled(QString indiWebPath)
{
    return QFileInfo(indiWebPath).exists() && indiWebPath.endsWith("indi-web");
}

bool MainWindow::indiWebInstalled()
{
    return indiWebInstalled(Options::indiwebPath());
}

/*
 *This method brings up the Preferences Dialog with two pages of options.
 */
void MainWindow::showPreferences()
{
    KConfigDialog *preferencesDialog = new KConfigDialog(this, i18n("Preferences Dialog"), Options::self());
    connect(preferencesDialog->button(QDialogButtonBox::Apply), SIGNAL(clicked()), SLOT(updateSettings()));
    connect(preferencesDialog->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(updateSettings()));

    OpsManager *config1 = new OpsManager(this);
    KPageWidgetItem *page1 = preferencesDialog->addPage(config1, i18n("Web Manager Options"));
    page1->setIcon(QIcon(":/media/images/indi_logo.png"));

    OpsConfiguration *config2 = new OpsConfiguration(this);
    KPageWidgetItem *page2 = preferencesDialog->addPage(config2, i18n("Configuration Options"));

    // This should make the icon look good no matter what the theme or mode of the system is.
    int backgroundBrightness = this->palette().color(QWidget::backgroundRole()).lightness();
    if (backgroundBrightness < 100)
        page2->setIcon(QIcon(":/media/icons/configure-dark.svg"));
    else
        page2->setIcon(QIcon(":/media/icons/configure.svg"));

    QPushButton *whatsThis = new QPushButton("?");
    whatsThis->setToolTip("What's This?");

    connect(whatsThis, &QPushButton::clicked, this, []()
            { QWhatsThis::enterWhatsThisMode(); });

    preferencesDialog->addActionButton(whatsThis);
    preferencesDialog->show();
}

/*
 * This method should update all the environment variables and the displayed settings in the main window.
 * It should be run after the configuration settings have been changed.
 */
void MainWindow::updateSettings()
{
    bool webManagerWasRunning = webManagerRunning;
    if (webManagerRunning)
        stopWebManager();

    updateIPAddressList();
    ui->ipListDisplay->adjustSize();
    ui->displayWebManagerPath->setText(getWebManagerURL());
    ui->displayWebManagerPath->setCursorPosition(0);
    if (Options::enableWebManagerLogFile())
    {
        managerLogFile = "";
        qDebug() << Options::logFilePath();
        QFileInfo logFileParentFolderInfo = QFileInfo(QFileInfo(Options::logFilePath()).dir().path());
        if (logFileParentFolderInfo.exists() && logFileParentFolderInfo.isWritable())
            managerLogFile = Options::logFilePath().remove(".txt") + "_" + QDateTime::currentDateTime().toString("yyyy-MM-ddThh-mm") + ".txt";
        else
            QMessageBox::information(nullptr, "message", i18n("The path for the selected log file does not exist or is not writeable.  Please create it or select another path.  The log file will not be saved until you do."));
    }

    // This prints lots of details to the log about the system.
    createManagerLogEntry(i18n("Welcome to INDI Web Manager App ") + QString(LithiumWebManagerApp_VERSION));
    createManagerLogEntry(i18n("Build: ") + QString(INDI_WEB_MANAGER_APP_BUILD_TS));
    createManagerLogEntry(i18n("OS: ") + QSysInfo::productType() + " " + QSysInfo::productVersion());
    createManagerLogEntry(i18n("API: ") + QSysInfo::buildAbi());
    createManagerLogEntry(i18n("Arch: ") + QSysInfo::currentCpuArchitecture());
    createManagerLogEntry(i18n("Kernel Type: ") + QSysInfo::kernelType());
    createManagerLogEntry(i18n("Kernel Version: ") + QSysInfo::kernelVersion());
    createManagerLogEntry(i18n("Qt Version: ") + QString(QT_VERSION_STR));

    configureEnvironmentVariables();

    if (webManagerWasRunning)
        startWebManager();
}

/*
 * This method configures all the environment variables.  It runs in conjuction with the above method.
 * Some of the variables dynamically change with the loation of the App bundle on OS X.
 * All are listed just to make sure none get forgotten.
 */
void MainWindow::configureEnvironmentVariables()
{
    // Note the Python Exec Variable does not dynamically change.
    // Note the GSC Variable does not dynamically change.
    if (Options::iNDIPrefixDefault())
        Options::setINDIServerPath(getDefault("INDIPrefix"));
    if (Options::iNDIServerDefault())
        Options::setINDIServerPath(getDefault("INDIServerPath"));
    if (Options::iNDIDriversDefault())
        Options::setINDIDriversPath(getDefault("INDIDriversPath"));
    if (Options::gPhotoIOLIBSDefault())
        Options::setGPhotoIOLIBS(getDefault("GPhotoIOLIBS"));
    if (Options::gPhotoCAMLIBSDefault())
        Options::setGPhotoCAMLIBS(getDefault("GPhotoCAMLIBS"));

    QString newPATH = Options::pythonExecFolder() + ":" + Options::iNDIServerPath() + ':' + Options::iNDIDriversPath() + ":/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin";
    insertEnvironmentVariable("PATH", newPATH);

// Note that these environment variables only make sense on OS X.
// If they are blank, they are not to be set
#ifdef Q_OS_OSX
    if (Options::iNDIPrefix() != "")
        insertEnvironmentPath("INDIPREFIX", Options::iNDIPrefix());
    if (Options::gPhotoIOLIBS() != "")
        insertEnvironmentPath("IOLIBS", Options::gPhotoIOLIBS());
    if (Options::gPhotoCAMLIBS() != "")
        insertEnvironmentPath("CAMLIBS", Options::gPhotoCAMLIBS());
#endif

    // This sets the path to the GSC Data
    insertEnvironmentPath("GSCDAT", Options::gSCPath());
}

/*
 * This Method inserts an environment variable and prints the export command to the log so the user can see what was done.
 * The environment variables should not have escaped characters, but the export command should.
 * This is so that the user can copy the command to the terminal if desired.
 */
void MainWindow::insertEnvironmentVariable(QString variable, QString value)
{
    qputenv(variable.toUtf8().data(), value.toLatin1());
    createManagerLogEntry("export " + variable + "=" + value.replace(" ", "\\ "));
}

/*
 * This method works just like the insert environment variable method, exept it checks that the path actually exists before inserting it.
 * If the path does not exist, it prints an error message about it.
 */
void MainWindow::insertEnvironmentPath(QString variable, QString filePath)
{
    if (QFileInfo::exists(filePath))
        insertEnvironmentVariable(variable, filePath);
    else
        createManagerLogEntry(i18n("The Path for the %1 environment variable does not exist on this system.  Please check your settings.  The stated path was: ").arg(variable) + filePath);
}

/*
 * This method starts the INDI Web Manager
 */
void MainWindow::startWebManager()
{
    if (webManagerRunning)
    {
        stopWebManager();
        webManager.clear();
    }
    webManager = new QProcess(this);

    createManagerLogEntry(i18n("INDI Web Manager Started."));
    connect(webManager, SIGNAL(finished(int)), this, SLOT(managerClosed(int)));
    webManager->setProcessChannelMode(QProcess::MergedChannels);
    connect(webManager, SIGNAL(readyReadStandardOutput()), this, SLOT(appendManagerLogEntry()));
    QStringList processArguments;
    if (Options::enableVerboseMode())
        processArguments << "--verbose";
    // This one is not optional
    processArguments << "--xmldir" << QDir(Options::iNDIDriversPath()).absolutePath();
    if (!Options::managerPortNumberDefault())
        processArguments << "--port" << Options::managerPortNumber();
    if (!Options::iNDIConfigPathDefault())
        processArguments << "--conf" << Options::iNDIConfigPath();
    createManagerLogEntry(Options::indiwebPath() + " " + processArguments.join(" "));
    webManager->start(Options::indiwebPath(), processArguments);
    displayManagerStatusOnline(true);

    serverMonitor.start();
    webManagerRunning = true;
}

/*
 * This method shuts down the INDI Web Manager
 */
void MainWindow::stopWebManager()
{
    if (!webManager.isNull())
    {
        disconnect(webManager, SIGNAL(finished(int)), this, SLOT(managerClosed(int)));
        webManager->kill();
    }
    webManagerRunning = false;

    QProcess killINDIServer;
    killINDIServer.start("/usr/bin/killall", QStringList() << "indiserver");
    killINDIServer.waitForFinished(300);
    createManagerLogEntry(i18n("INDI Web Manager Shut down successfully."));
    updateDisplaysforShutDown();
}

/*
 * This method uses the method below it to put output from the running web manager into the log in the Main Window and console.
 */
void MainWindow::appendManagerLogEntry()
{
    appendManagerLogEntry(webManager->readAll().trimmed());
}

/*
 * This method uses the method below it to put output from the running web manager into the log in the Main Window and console.
 */
void MainWindow::createManagerLogEntry(QString text)
{
    QString entry = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss:  ") + text;
    appendManagerLogEntry(entry.trimmed());
}

/*
 * This method puts information into the log in the Main Window.  It also outputs the same message to the console.
 * Note: Should this also append to the web manager log file.  Is this possible?
 */
void MainWindow::appendManagerLogEntry(QString entry)
{
    ui->webManagerLog->appendPlainText(entry);
    qDebug() << entry;

    if (Options::enableWebManagerLogFile() && managerLogFile != "")
    {
        QFile logFile;
        logFile.setFileName(managerLogFile);
        if (logFile.open(QIODevice::ReadWrite | QIODevice::Append))
        {
            QTextStream out(&logFile);
            out << entry << endl;
            logFile.close();
        }
    }
}

/*
 * This method is connnected to the running INDI Web Manager.
 * It will run if for some reason the Web Manager gets terminated prematurely.
 * (But not by clicking the stop button or closing the program)
 * It updates the displays in the Main Window and prints a message to the log about the shut down.
 */
void MainWindow::managerClosed(int result)
{
    webManagerRunning = false;
    if (result == 0)
        createManagerLogEntry(i18n("INDI Web Manager Shut down successfully."));
    else
    {
        createManagerLogEntry(i18n("INDI Web Manager Shut down with error."));
    }
    updateDisplaysforShutDown();
}

/*
 * This updates the displays in the Main Window when the Web Mabager shuts down and stops the monitor.
 */
void MainWindow::updateDisplaysforShutDown()
{
    displayManagerStatusOnline(false);
    displayServerStatusOnline(false);
    ui->profileBox->clear();
    ui->driversDisplay->clear();
    ui->displayINDIServerPath->clear();
    serverMonitor.stop();
}

/*
 * This detects if the program is being closed so that we don't leave the Web Manger and INDI Server running by accident.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox closeOptionsBox;
    closeOptionsBox.setText(i18n("Do you really want to quit, or just hide the window?"));
    QPushButton *quitButton = closeOptionsBox.addButton(tr("Quit"), QMessageBox::YesRole);
    closeOptionsBox.addButton(tr("Hide"), QMessageBox::NoRole);
    int response = closeOptionsBox.exec();
    if (response)
    {
    }
    if (closeOptionsBox.clickedButton() == quitButton)
    {
        if (webManagerRunning)
            stopWebManager();
        webManager->waitForFinished(300);
        event->accept();
        qApp->exit();
    }
    else
    {
        event->ignore();
        this->hide();
    }
}

/*
 * This method updates the status of the Web Manager in the Main Window.
 * It does not run automatically. It only gets changed when this program changes the Web Manager Status.
 * It does not actually check to see if it is accessible.  This is not optimal, but if there is any shut down of the manager, this should get updated.
 * It cannot use the same scheme I am using for checking the status of the INDI Server since if the web Manager is not running, the monitor is not either.
 */
void MainWindow::displayManagerStatusOnline(bool online)
{
    if (online)
    {
        ui->statusDisplay->setText(i18n("Online"));
        ui->statusDisplay->setStyleSheet("QLineEdit {background-color: green;}");
        managerStatusinTray->setText("Manager: Online");
        managerStatusinTray->setIcon(QIcon(":/media/icons/green.png"));
    }
    else
    {
        ui->statusDisplay->setText(i18n("Offline"));
        ui->statusDisplay->setStyleSheet("QLineEdit {background-color: red;}");
        managerStatusinTray->setText("Manager: Offline");
        managerStatusinTray->setIcon(QIcon(":/media/icons/red.png"));
    }
}

/*
 * This method shows and hides the log in the Main Window.
 */
void MainWindow::setLogVisible(bool visible)
{
    ui->logViewer->setVisible(visible);
    if (visible)
        this->adjustSize();
    else
        this->setFixedSize(minimumWindowSize);

    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

//*******************  The Following methods are for communicating with and updating the status of a runnning INDI Server.

/*
 * This method checks on the Status of the INDI Server, gets the active profile, and any running drivers.
 * It then updates the status in the main window.
 * It should be running once a second when the Web Manager is active and running.
 */
void MainWindow::checkINDIServerStatus()
{
    QString activeProfile = "";
    bool INDIServerOnline = isINDIServerOnline(activeProfile);
    displayServerStatusOnline(INDIServerOnline);
    QStringList profiles = getProfiles();
    if (oldProfiles.join(",") != profiles.join(",") || ui->profileBox->count() == 0 || oldActiveProfile != activeProfile)
    {
        ui->profileBox->clear();
        ui->profileBox->addItems(profiles);
        ui->profileBox->setCurrentText(activeProfile);
        oldProfiles = profiles;
        oldActiveProfile = activeProfile;
    }
    ui->profileBox->setDisabled(INDIServerOnline);
    QString port = "";
    port = getINDIServerPort();
    ui->displayINDIServerPath->setText(getINDIServerURL(port));
    ui->displayINDIServerPath->setCursorPosition(0);
    if (INDIServerOnline)
    {
        QString webManagerDrivers = "";
        getRunningDrivers(webManagerDrivers);
        if (oldDrivers != webManagerDrivers || ui->driversDisplay->count() == 0)
        {
            ui->driversDisplay->clear();
            QStringList activeDrivers = webManagerDrivers.split("\n");
            foreach (QString driver, activeDrivers)
            {
                ui->driversDisplay->addItem(new QListWidgetItem(QIcon(":/media/icons/green.png"), driver));
            }
            oldDrivers = webManagerDrivers;
        }
    }
    else
    {
        ui->driversDisplay->clear();
        oldDrivers.clear();
    }
}

/*
 * This method updates the status of the INDI Server in the Main Window.
 */
void MainWindow::displayServerStatusOnline(bool online)
{
    if (online)
    {
        ui->serverStatusDisplay->setText(i18n("Online"));
        ui->serverStatusDisplay->setStyleSheet("QLineEdit {background-color: green;}");
        serverStatusinTray->setText("INDI Server: Online");
        serverStatusinTray->setIcon(QIcon(":/media/icons/green.png"));
    }
    else
    {
        ui->serverStatusDisplay->setText(i18n("Offline"));
        ui->serverStatusDisplay->setStyleSheet("QLineEdit {background-color: red;}");
        serverStatusinTray->setText("INDI Server: Offline");
        serverStatusinTray->setIcon(QIcon(":/media/icons/red.png"));
    }
}

/*
 * This method is intended to figure out if ANY Web Manager is running at the default port to give the user an option to stop it.
 * It checks both the user configured url and the default url.
 */
bool MainWindow::isWebManagerOnline()
{
    QUrl url1(QString("http://localhost:8624/api/server/status"));
    QJsonDocument json;
    bool test1 = getWebManagerResponse(url1, &json);

    QUrl url2(QString(getWebManagerURL() + "/api/server/status"));
    QJsonDocument json2;
    bool test2 = getWebManagerResponse(url2, &json2);

    if (test1 || test2)
        return true;
    else
        return false;
}

/*
 * This method determines if an INDIServer hosted by this program and accessed using the desired method is online.
 */
bool MainWindow::isINDIServerOnline(QString &activeProfile)
{
    QUrl url(QString(getWebManagerURL() + "/api/server/status"));

    QJsonDocument json;
    if (getWebManagerResponse(url, &json))
    {
        QJsonArray array = json.array();
        if (array.isEmpty())
            return false;

        QJsonObject jsonObj = array.first().toObject();
        QString status = jsonObj["status"].toString();
        if (status == "True")
        {
            activeProfile = jsonObj["active_profile"].toString();
            return true;
        }
    }
    return false;
}

/*
 * This method tries to start an INDIServer using the specified profile.
 */
void MainWindow::startINDIServer()
{
    QString profileToStart = ui->profileBox->currentText();
    QUrl url(QString(getWebManagerURL() + "/api/server/start/" + profileToStart));
    sendWebManagerCommand(url);
}

/*
 * This method tries to stop the currently running INDIServer
 */
void MainWindow::stopINDIServer()
{
    QUrl url(QString(getWebManagerURL() + "/api/server/stop"));
    sendWebManagerCommand(url);
}

/*
 * This method determines the port of the active server if one is running.
 */
QString MainWindow::getINDIServerPort()
{
    QUrl url(QString(getWebManagerURL() + "/api/profiles"));

    QJsonDocument json;
    if (getWebManagerResponse(url, &json))
    {
        QJsonArray array = json.array();

        if (array.isEmpty())
            return "";

        for (auto value : array)
        {
            QJsonObject profile = value.toObject();
            if (profile["name"].toString() == ui->profileBox->currentText())
                return QString::number(profile["port"].toInt());
        }
    }
    return "";
}

/*
 * This method determines what drivers are active on an INDIServer hosted by this program and accessed using the desired method.
 */
QStringList MainWindow::getProfiles()
{
    QStringList profilesList;
    QUrl url(QString(getWebManagerURL() + "/api/profiles"));

    QJsonDocument json;
    if (getWebManagerResponse(url, &json))
    {
        QJsonArray array = json.array();

        if (array.isEmpty())
            return profilesList;

        for (auto value : array)
        {
            QJsonObject profile = value.toObject();
            profilesList << profile["name"].toString();
        }
        return profilesList;
    }
    return profilesList;
}

/*
 * This method determines what drivers are active on an INDIServer hosted by this program and accessed using the desired method.
 */
bool MainWindow::getRunningDrivers(QString &webManagerDrivers)
{
    QUrl url(QString(getWebManagerURL() + "/api/server/drivers"));

    QJsonDocument json;
    if (getWebManagerResponse(url, &json))
    {
        QJsonArray array = json.array();

        if (array.isEmpty())
            return false;

        QStringList webManagerDriversList;
        for (auto value : array)
        {
            QJsonObject driver = value.toObject();
            if (driver["label"].toString() != "")
                webManagerDriversList << driver["label"].toString();
            else
            {
                webManagerDriversList << driver["name"].toString();
            }
        }
        webManagerDrivers = webManagerDriversList.join("\n");
        return true;
    }
    return false;
}

/*
 * This method is how POST commands are sent to the INDI Server
 */
void MainWindow::sendWebManagerCommand(const QUrl &url)
{
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *response = manager.post(request, QByteArray());

    QTimer timeout(this);
    timeout.setInterval(5000);
    timeout.setSingleShot(true);
    timeout.start();
    while (!response->isFinished())
    {
        if (!timeout.isActive())
        {
            response->deleteLater();
            createManagerLogEntry(i18n("Timeout while waiting for response from INDI Server"));
            return;
        }
        qApp->processEvents();
    }
    timeout.stop();

    if (response->error() != QNetworkReply::NoError)
    {
        createManagerLogEntry(i18n("INDI: Error communicating with INDI Web Manager: ") + response->errorString());
    }
}

/*
 * This method is a how the above methods communicate with the web manager and get a response.
 */

bool MainWindow::getWebManagerResponse(const QUrl &url, QJsonDocument *reply)
{
    QNetworkAccessManager manager;
    QNetworkReply *response = nullptr;
    QNetworkRequest request;

    request.setUrl(url);
    response = manager.get(request);

    // Wait synchronously
    QEventLoop event;
    QObject::connect(response, SIGNAL(finished()), &event, SLOT(quit()));
    event.exec();

    if (response->error() == QNetworkReply::NoError)
    {
        if (reply)
        {
            QJsonParseError parseError;
            *reply = QJsonDocument::fromJson(response->readAll(), &parseError);

            if (parseError.error != QJsonParseError::NoError)
            {
                if (webManagerRunning) // This way it doesn't print errors when checking to see if the Web Manager is running at all
                    createManagerLogEntry(i18n("INDI: JSon error during parsing ") + parseError.errorString());
                return false;
            }
        }
        return true;
    }
    else
    {
        if (webManagerRunning) // This way it doesn't print errors when checking to see if the Web Manager is running at all
            createManagerLogEntry(i18n("INDI: Error communicating with INDI Web Manager: ") + response->errorString());
        return false;
    }
}
