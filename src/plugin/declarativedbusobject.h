/****************************************************************************************
**
** Copyright (c) 2021 Jolla Ltd.
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

#ifndef DECLARATIVEDBUSOBJECT_H
#define DECLARATIVEDBUSOBJECT_H

#include "declarativedbusabstractobject.h"

#include <QQmlListProperty>
#include <QVector>

class DeclarativeDBusAdaptor;

class DeclarativeDBusObject : public DeclarativeDBusAbstractObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> adaptors READ adaptors NOTIFY adaptorsChanged)
    Q_INTERFACES(QQmlParserStatus)
    Q_CLASSINFO("DefaultProperty", "adaptors")

public:
    explicit DeclarativeDBusObject(QObject *parent = nullptr);
    ~DeclarativeDBusObject() override;

    QQmlListProperty<QObject> adaptors();

    void componentComplete() override;

signals:
    void adaptorsChanged();

protected:
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

private:
    static void adaptor_append(QQmlListProperty<QObject> *property, QObject *value);
    static QObject *adaptor_at(QQmlListProperty<QObject> *property, int index);
    static int adaptor_count(QQmlListProperty<QObject> *property);
    static void adaptor_clear(QQmlListProperty<QObject> *property);

    QHash<QString, DeclarativeDBusAdaptor *> m_adaptors;
    QVector<QObject *> m_objects;
};

#endif
