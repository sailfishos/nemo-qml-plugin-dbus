TARGET = nemodbus
PLUGIN_IMPORT_PATH = org/nemomobile/dbus
QT += dbus qml

QT -= gui

TEMPLATE = lib
CONFIG += qt plugin hide_symbols

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += \
        $$_PRO_FILE_PWD_/qmldir \
        $$_PRO_FILE_PWD_/plugins.qmltypes
qmldir.path +=  $$target.path
INSTALLS += qmldir

SOURCES += \
    plugin.cpp \
    declarativedbus.cpp \
    declarativedbusadaptor.cpp \
    declarativedbusadaptor10.cpp \
    declarativedbusinterface.cpp \
    declarativedbusinterface10.cpp

HEADERS += \
    declarativedbus.h \
    declarativedbusadaptor.h \
    declarativedbusadaptor10.h \
    declarativedbusinterface.h \
    declarativedbusinterface10.h
