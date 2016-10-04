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

#ifndef DECLARATIVEDBUSADAPTOR10_H
#define DECLARATIVEDBUSADAPTOR10_H

#include "declarativedbusadaptor.h"

// This class contains compatibility enums and property aliases for exposing
// the old DBusAdaptor QML API when users import this plug-in as "1.0".
class DeclarativeDBusAdaptor10 : public DeclarativeDBusAdaptor
{
    Q_OBJECT

    // Deprecated alias, only used for compatibility with < 1.0.0 releases
    Q_PROPERTY(DeclarativeDBus::BusType busType READ bus WRITE setBus NOTIFY busTypeChanged)

    // Deprecated (since version 1.0.0): Use DeclarativeDBus::BusType instead
    Q_ENUMS(BusType)

public:
    DeclarativeDBusAdaptor10(QObject *parent = 0);
    ~DeclarativeDBusAdaptor10();

    // Deprecated, newer versions have emitSignal with an optional arguments parameter
    Q_INVOKABLE void emitSignalWithArguments(const QString &name, const QJSValue &arguments)
    {
        emitSignal(name, arguments);
    }

    // Deprecated (since version 1.0.0): Use DeclarativeDBus::BusType instead
    // (in QML, use DBus.SessionBus instead of DBusAdaptor.SessionBus)
    enum BusType {
        SystemBus = DeclarativeDBus::SystemBus,
        SessionBus = DeclarativeDBus::SessionBus
    };

signals:
    void busTypeChanged();
};

#endif
