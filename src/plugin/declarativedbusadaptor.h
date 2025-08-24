/****************************************************************************************
**
** Copyright (C) 2013 - 2021 Jolla Ltd.
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

#ifndef DECLARATIVEDBUSADAPTOR_H
#define DECLARATIVEDBUSADAPTOR_H

#include "declarativedbusabstractobject.h"

class DeclarativeDBusAdaptor : public DeclarativeDBusAbstractObject
{
    Q_OBJECT
    Q_PROPERTY(QString iface READ interface WRITE setInterface NOTIFY interfaceChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit DeclarativeDBusAdaptor(QObject *parent = nullptr);
    ~DeclarativeDBusAdaptor() override;

    QString interface() const;
    void setInterface(const QString &interface);

    Q_INVOKABLE void emitSignal(const QString &name,
                                const QJSValue &arguments = QJSValue::UndefinedValue);

    bool getProperty(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface,
            const QString &member) override;
    bool getProperties(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface) override;
    bool setProperty(
            const QString &interface, const QString &member, const QVariant &value) override;
    bool invoke(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface,
            const QString &name,
            const QVariantList &dbusArguments) override;

signals:
    void interfaceChanged();

private:
    QString m_interface;
};

#endif
