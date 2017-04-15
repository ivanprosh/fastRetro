#include <QGUIApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "addresstable.h"
#include "mainwindow.h"
#include "connectionManager.h"
#include "dataanalizator.h"
#include "global.h"

QMutex GLOBAL::globalMutex(QMutex::NonRecursive);

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName(app.translate("main", "FastRetroApp"));
    //app.setWindowIcon(QIcon(":/icon.png"));
    app.setOrganizationName("System Service Ltd.");
    app.setOrganizationDomain("systserv.spb.su");
    qRegisterMetaType<GlobalError*>();

    //инициализация объекта работы с DB
    DataAnalizator::instance()->initialize();
    QString err = DataAnalizator::instance()->rfile("signals.ini");
    if(!err.isEmpty()){
        qDebug() << "Не найден конфигур. файл!";
        //GLOBAL::warning(0,0,"Ошибка чтения",err);
        return 0;
    }

    //подключение к базе выполняется при смене имени сервера и/или старте подключения
    //DataAnalizator::instance()->

    AddressTable* AppAddressTable = new AddressTable();
    //QAbstractTableModel* AppAddressTable = new QAbstractTableModel();
    MainWindow* MainClass = new MainWindow(AppAddressTable);

    QTimer::singleShot(500, MainClass, SLOT(initializeSettings()));

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("AppAddressTable", AppAddressTable);
    engine.rootContext()->setContextProperty("MainClass", MainClass);
    //qDebug() << QString(PRO_FILE_PWD);
#ifdef QML_DEBUG
    engine.load(QString(QString(PRO_FILE_PWD) + "/mainwindow.qml"));
    //engine.load(QString("d:/WORK/_QT/SS/FastRetroSocketClient/mainwindow.qml"));
#else
    engine.load(QUrl(QStringLiteral("qrc:/mainwindow.qml")));
#endif
    //tripPlanner.show();
    //DataAnalizator::instance()->startWorkerThread();

    return app.exec();
}
