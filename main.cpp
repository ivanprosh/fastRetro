//#include <QQuickView>
//#include <QGuiApplication>
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
    //вынужденное использование QApplication вместо Gui, так как используется виджет системного трея
//    QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    app.setApplicationName(app.translate("main", "FastRetro"));
    //app.setWindowIcon(QIcon(":/icon.png"));
    app.setOrganizationName("System Service Ltd.");
    app.setOrganizationDomain("systserv.spb.su");
    app.setWindowIcon(QIcon(":/images/send.svg"));

    qRegisterMetaType<GlobalError*>();

//    QQuickView *view = new QQuickView;
    //инициализация объекта работы с DB
    DataAnalizator::instance()->initialize();
    QString err = DataAnalizator::instance()->rfile("signals.ini");

    AddressTable* AppAddressTable = new AddressTable();
    MainWindow* MainClass = new MainWindow(AppAddressTable);

    QQmlApplicationEngine engine;

    // Объявляем и инициализируем объекта класса для работы с системным треем
    SystemTray * systemTray = new SystemTray();

    engine.rootContext()->setContextProperty("systemTray", systemTray);
    engine.rootContext()->setContextProperty("AppAddressTable", AppAddressTable);
    engine.rootContext()->setContextProperty("MainClass", MainClass);
/*
    view->rootContext()->setContextProperty("systemTray", systemTray);
    view->rootContext()->setContextProperty("AppAddressTable", AppAddressTable);
    view->rootContext()->setContextProperty("MainClass", MainClass);
*/
    MainClass->initializeSettings();
//для отладки предполагалось использовать вместо файлов ресурсов, несжатые qml файлы
#ifdef QML_DEBUG
    engine.load(QString(QString(PRO_FILE_PWD) + "/mainwindow.qml"));
#else
    engine.load(QUrl(QStringLiteral("qrc:/mainwindow.qml")));
#endif

//    view->setSource(QUrl("qrc:/mainwindow.qml"));
//    view->show();
    return app.exec();
}
