#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QIcon>

#include "addresstable.h"
#include "mainwindow.h"
#include "connectionManager.h"
#include "dataanalizator.h"
#include "global.h"
#include "systemtray.h"

QMutex GLOBAL::globalMutex(QMutex::NonRecursive);
QString GLOBAL::ThreadCheck("");

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName(app.translate("main", "FastRetroApp"));
    //app.setWindowIcon(QIcon(":/icon.png"));
    app.setOrganizationName("System Service Ltd.");
    app.setOrganizationDomain("systserv.spb.su");
    app.setWindowIcon(QIcon(":/images/send.svg"));

    qRegisterMetaType<GlobalError*>();

    //инициализация объекта работы с DB
    DataAnalizator::instance()->initialize();
    QString err = DataAnalizator::instance()->rfile("signals.ini");

    //подключение к базе выполняется при смене имени сервера и/или старте подключения
    //DataAnalizator::instance()->

    AddressTable* AppAddressTable = new AddressTable();
    //QAbstractTableModel* AppAddressTable = new QAbstractTableModel();
    MainWindow* MainClass = new MainWindow(AppAddressTable);

    QQmlApplicationEngine engine;

    // Объявляем и инициализируем объекта класса для работы с системным треем
    SystemTray * systemTray = new SystemTray();

    engine.rootContext()->setContextProperty("systemTray", systemTray);
    engine.rootContext()->setContextProperty("AppAddressTable", AppAddressTable);
    engine.rootContext()->setContextProperty("MainClass", MainClass);

    MainClass->initializeSettings();

#ifdef QML_DEBUG
    engine.load(QString(QString(PRO_FILE_PWD) + "/mainwindow.qml"));
#else
    //engine.load(QUrl(QStringLiteral("qrc:/mainwindow.qml")));
    engine.load(QUrl(QStringLiteral("qrc:/mainwindow.qml")));
#endif

    return app.exec();
}
