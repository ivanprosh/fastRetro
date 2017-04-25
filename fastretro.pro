TEMPLATE      = app
QT           += gui network qml quick
CONFIG += c++11

HEADERS       = \
    addresstable.h \
    global.h \
    plcsocketclient.h \
    connectionManager.h \
    mainwindow.h \
    dataanalizator.h \
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
    LogView.qml \
    Filedialog.qml

#Файлы QML при определении данного дефайна берутся из каталога QML, а не линкуются статически
#в составе исп. файла
DEFINES += QML_DEBUG
DEFINES += PRO_FILE_PWD=$$sprintf("\"\\\"%1\\\"\"", $$_PRO_FILE_PWD_)

#Вариант стратегий: FORWARD (напрямую insert'ом), NATIVE (родной метод через csv)
DEFINES += NATIVE

contains(DEFINES,FORWARD){
    QT += sql
}

RESOURCES += res.qrc
