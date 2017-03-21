#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "addresstable.h"
#include "fastretro.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AddressTable* AppAddressTable = new AddressTable();
    FastRetro* MainClass = new FastRetro(AppAddressTable);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("AppAddressTable", AppAddressTable);
    engine.rootContext()->setContextProperty("MainClass", MainClass);
    engine.load(QUrl(QStringLiteral("qrc:/mainwindow.qml")));
    //tripPlanner.show();
    return app.exec();
}
