TEMPLATE      = app
QT           += network qml quick widgets
HEADERS       = fastretro.h \
                addresstable.h \
    global.h
SOURCES       = main.cpp \
                fastretro.cpp \
                addresstable.cpp
FORMS         = fastretro.ui

DISTFILES += \
    mainwindow.qml \
    MyTextField.qml

RESOURCES += \
    res.qrc
