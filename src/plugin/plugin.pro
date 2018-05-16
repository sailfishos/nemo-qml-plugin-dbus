TARGET = nemodbus
PLUGIN_IMPORT_PATH = Nemo/DBus
QT += dbus qml

QT -= gui

TEMPLATE = lib
CONFIG += qt plugin hide_symbols

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

LIBS += -L../nemo-dbus -lnemodbus
INCLUDEPATH += $$PWD/.. $$PWD/../nemo-dbus

qmldir.files += \
        qmldir \
        plugins.qmltypes
qmldir.path +=  $$target.path
INSTALLS += qmldir

qmltypes.commands = qmlplugindump -nonrelocatable Nemo.DBus 2.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

SOURCES += \
    plugin.cpp \
    declarativedbus.cpp \
    declarativedbusadaptor.cpp \
    declarativedbusinterface.cpp \

HEADERS += \
    declarativedbus.h \
    declarativedbusadaptor.h \
    declarativedbusinterface.h \
