/****************************************************************************************
**
** Copyright (c) 2013 - 2021 Jolla Ltd.
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

#include "declarativedbusabstractobject.h"

#include "dbus.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusConnection>

#include <QTimerEvent>

#include <qqmlinfo.h>

DeclarativeDBusAbstractObject::DeclarativeDBusAbstractObject(QObject *parent)
    : QDBusVirtualObject(parent)
    , m_bus(DeclarativeDBus::SessionBus)
    , m_quitTimerId(-1)
    , m_quitTimeout(30)
    , m_quitOnTimeout(false)
    , m_complete(true)
{
}

DeclarativeDBusAbstractObject::~DeclarativeDBusAbstractObject()
{
    QDBusConnection conn = DeclarativeDBus::connection(m_bus);
    conn.unregisterObject(m_path);

    // In theory an application could have multiple AbstractObject items for different paths or
    // interfaces and destroying one would unregister the whole service.
    // If problems arise we could introduce refcounting for services so that
    // the last service would be responsible for unregisting the service upon
    // destruction. Unregisration should also take into account unregistration happening
    // from C++ side.
    if (!m_service.isEmpty()) {
        if (!conn.unregisterService(m_service)) {
            qmlInfo(this) << "Failed to unregister service " << m_service;
            qmlInfo(this) << conn.lastError().message();
        }
    }
}

QString DeclarativeDBusAbstractObject::service() const
{
    return m_service;
}

void DeclarativeDBusAbstractObject::setService(const QString &service)
{
    if (m_service != service) {
        m_service = service;
        emit serviceChanged();
    }
}

QString DeclarativeDBusAbstractObject::path() const
{
    return m_path;
}

void DeclarativeDBusAbstractObject::setPath(const QString &path)
{
    if (m_path != path) {
        m_path = path;
        emit pathChanged();
    }
}

/*
    The XML service description. This could be derived from the meta-object but since it's unlikely
    to be needed most of the time this and type conversion is non-trivial it's not, and there is
    this property instead if it is needed.
*/

QString DeclarativeDBusAbstractObject::xml() const
{
    return m_xml;
}

void DeclarativeDBusAbstractObject::setXml(const QString &xml)
{
    if (m_xml != xml) {
        m_xml = xml;
        emit xmlChanged();
    }
}

DeclarativeDBus::BusType DeclarativeDBusAbstractObject::bus() const
{
    return m_bus;
}

void DeclarativeDBusAbstractObject::setBus(DeclarativeDBus::BusType bus)
{
    if (m_bus != bus) {
        m_bus = bus;
        emit busChanged();
    }
}

int DeclarativeDBusAbstractObject::quitTimeout() const
{
    return m_quitTimeout;
}

void DeclarativeDBusAbstractObject::setQuitTimeout(int timeout)
{
    if (m_quitTimeout != timeout) {
        m_quitTimeout = timeout;

        emit quitTimeoutChanged();
    }
}


bool DeclarativeDBusAbstractObject::quitOnTimeout() const
{
    return m_quitOnTimeout;
}

void DeclarativeDBusAbstractObject::setQuitOnTimeout(bool quit)
{
    if (m_quitOnTimeout != quit && (!m_complete || !quit)) {
        m_quitOnTimeout = quit;

        if (m_quitTimerId != -1) {
            killTimer(m_quitTimerId);
            m_quitTimerId = -1;
        }

        emit quitOnTimeoutChanged();

        m_quitLocker.reset();
    }
}

void DeclarativeDBusAbstractObject::classBegin()
{
    m_complete = false;
}

void DeclarativeDBusAbstractObject::componentComplete()
{
    m_complete = true;

    QDBusConnection conn = DeclarativeDBus::connection(m_bus);

    // It is still valid to publish an object on the bus without first registering a service name,
    // a remote process would have to connect directly to the DBus address.
    if (!m_path.isEmpty() && !conn.registerVirtualObject(m_path, this)) {
        qmlInfo(this) << "Failed to register object " << m_path;
        qmlInfo(this) << conn.lastError().message();
    }

    // Register service name only if it has been set.
    if (!m_service.isEmpty()) {
        if (!conn.registerService(m_service)) {
            qmlInfo(this) << "Failed to register service " << m_service;
            qmlInfo(this) << conn.lastError().message();
        }
    }

    if (m_quitOnTimeout && m_quitTimeout > 0) {
        m_quitLocker.reset(new QEventLoopLocker);

        m_quitTimerId = startTimer(m_quitTimeout * 1000);
    }
}

QString DeclarativeDBusAbstractObject::introspect(const QString &) const
{
    return m_xml;
}

bool DeclarativeDBusAbstractObject::handleMessage(
        const QDBusMessage &message, const QDBusConnection &connection)
{
    const QVariantList dbusArguments = message.arguments();

    QString member = message.member();
    QString interface = message.interface();

    // Don't try to handle introspect call. It will be handled
    // internally for QDBusVirtualObject derived classes.
    if (interface == QLatin1String("org.freedesktop.DBus.Introspectable")) {
        return false;
    } else if (interface == QLatin1String("org.freedesktop.DBus.Properties")) {
        if (member == QLatin1String("Get")) {
            return getProperty(
                        message,
                        connection,
                        dbusArguments.value(0).toString(),
                        dbusArguments.value(1).toString());
        } else if (member == QLatin1String("GetAll")) {
            return getProperties(
                        message,
                        connection,
                        dbusArguments.value(0).toString());
        } else if (member == QLatin1String("Set")) {
            return setProperty(
                        dbusArguments.value(0).toString(),
                        dbusArguments.value(1).toString(),
                        NemoDBus::demarshallDBusArgument(dbusArguments.value(2)));
        }
    } else {
        return invoke(message, connection, interface, member, dbusArguments);
    }

    return false;
}

void DeclarativeDBusAbstractObject::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_quitTimerId) {
        killTimer(m_quitTimerId);

        m_quitTimerId = -1;

        m_quitLocker.reset();
    } else {
        QDBusVirtualObject::timerEvent(event);
    }
}
