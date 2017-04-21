TEMPLATE      = app
QT           += core network qml quick sql
#CONFIG += c++11

HEADERS       = \
                addresstable.h \
    global.h \
    plcsocketclient.h \
    connectionManager.h \
    mainwindow.h \
    dataanalizator.h \
    worker.h \
    workthread.h \
    globalerror.h \
    logger.h \
    circlequeue.h \
    strategies.h
SOURCES       = main.cpp \
                addresstable.cpp \
    plcsocketclient.cpp \
    connectionManager.cpp \
    mainwindow.cpp \
    dataanalizator.cpp \
    global.cpp \
    logger.cpp \
    strategies.cpp \
    workthread.cpp
#FORMS         = fastretro.ui

deployment.path = $$OUT_PWD/
deployment.files += signals.ini
INSTALLS += deployment

DISTFILES += \
    signals.ini \
    mainwindow.qml \
    MyTextField.qml \
    SettingsDialog.qml \
    LogView.qml

DEFINES += QML_DEBUG
DEFINES += PRO_FILE_PWD=$$sprintf("\"\\\"%1\\\"\"", $$_PRO_FILE_PWD_)

#Вариант стратегий: FORWARD (напрямую insert'ом), NATIVE (родной метод через csv)
DEFINES += NATIVE

RESOURCES += \
    res.qrc
