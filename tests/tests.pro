QT += core gui quick testlib
CONFIG += testcase c++17
TEMPLATE = app
TARGET = tst_lettera

INCLUDEPATH += ../src
SOURCES += \
    tst_lettera.cpp \
    ../src/backend.cpp \
    ../src/writingsettings.cpp \
    ../src/systemtheme.cpp
HEADERS += \
    ../src/backend.h \
    ../src/writingsettings.h \
    ../src/systemtheme.h

QT += widgets printsupport quickcontrols2 quickdialogs2 dbus
RESOURCES += ../src/resources.qrc
