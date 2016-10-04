/****************************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Andrew den Exter <andrew.den.exter@jollamobile.com>
** All rights reserved.
**
** You may use this file under the terms of the GNU Lesser General
** Public License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
** 
****************************************************************************************/

#ifndef DECLARATIVEDBUSINTERFACE10_H
#define DECLARATIVEDBUSINTERFACE10_H

#include "declarativedbusinterface.h"

class DeclarativeDBusInterface10 : public DeclarativeDBusInterface
{
    Q_OBJECT

    // Deprecated aliases, only used for compatibility with < 1.0.0 releases
    Q_PROPERTY(QString destination READ service WRITE setService NOTIFY destinationChanged)
    Q_PROPERTY(DeclarativeDBus::BusType busType READ bus WRITE setBus NOTIFY busTypeChanged)

    // Deprecated (since version 1.0.0): Use DeclarativeDBus::BusType instead
    Q_ENUMS(BusType)

public:
    DeclarativeDBusInterface10(QObject *parent = 0);
    ~DeclarativeDBusInterface10();

    // Deprecated, newer versions have typedCall with an optional callback parameters
    Q_INVOKABLE void typedCallWithReturn(const QString &method, const QJSValue &arguments,
            const QJSValue &callback)
    {
        typedCall(method, arguments, callback);
    }

    // Deprecated (since version 1.0.0): Use DeclarativeDBus::BusType instead
    // (in QML, use DBus.SessionBus instead of DBusInterface.SessionBus)
    enum BusType {
        SystemBus = DeclarativeDBus::SystemBus,
        SessionBus = DeclarativeDBus::SessionBus
    };

signals:
    void destinationChanged();
    void busTypeChanged();
};

#endif
