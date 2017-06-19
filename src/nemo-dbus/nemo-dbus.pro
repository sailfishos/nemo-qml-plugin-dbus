TEMPLATE = lib
TARGET  = nemodbus

CONFIG += \
        c++11 \
        hide_symbols \
        link_pkgconfig \
        create_pc \
        create_prl \
        no_install_prl

QT -= gui
QT += dbus

INCLUDEPATH += ..

DEFINES += \
        NEMODBUS_BUILD_LIBRARY

include(private/private.pri)

SOURCES += \
        connection.cpp \
        dbus.cpp \
        interface.cpp \
        logging.cpp \
        object.cpp \
        response.cpp

PUBLIC_HEADERS += \
        connection.h \
        dbus.h \
        global.h \
        interface.h \
        object.h \
        response.h

HEADERS += \
        $$PRIVATE_HEADERS \
        $$PUBLIC_HEADERS \
        logging.h

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = /usr/include/nemo-dbus

private_headers.files = $$PRIVATE_HEADERS
private_headers.path = /usr/include/nemo-dbus/private

target.path = /usr/lib

QMAKE_PKGCONFIG_NAME = nemodbus
QMAKE_PKGCONFIG_DESCRIPTION = Nemo library for DBus
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = /usr/include
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        private_headers \
        public_headers \
        target
