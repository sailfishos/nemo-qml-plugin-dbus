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

#include "declarativedbusobject.h"

#include "declarativedbusadaptor.h"

/*!
    \qmltype DBusObject
    \inqmlmodule Nemo.DBus
    \brief Provides a service on D-Bus

    The DBusObject object can be used to register a D-Bus object implementing multiple interfaces
    on the system or session bus.

    Each interface implemented by the object is defined in an individual DBusAdaptor in the body
    of the DBusObject and will be registered as belonging to the same path and the same optional
    service.

    \section2 Exposing an object on D-Bus

    The following code demonstrates how to expose an object on the session bus. The
    \c {com.example.service} service name will be registered and an object at
    \c {/com/example/service} will be registered supporting the \c {com.example.service} and
    \c {org.freedesktop.Application} interfaces in addition to the common interfaces
    \c {org.freedesktop.DBus.Properties}, \c {org.freedesktop.DBus.Introspectable}
    and \c {org.freedesktop.DBus.Peer}.


    \code
    import QtQuick 2.0
    import Nemo.DBus 2.0

    ApplicationWindow {
        id: window

        DBusObject {
            id: dbus

            property bool needUpdate: true

            quitOnTimeout: !Qt.application.active

            service: "com.example.service"
            path: "/com/example/service"

            xml: "
     <interface name='com.example.service'>
        <method name='update' />
        <property name='needUpdate' type='b' access='readwrite' />
     </interface>
     <interface name='org.freedesktop.Application'>
      <method name='Activate'>
        <arg type='a{sv}' name='platform_data' direction='in'/>
      </method>
      <method name='Open'>
        <arg type='as' name='uris' direction='in'/>
        <arg type='a{sv}' name='platform_data' direction='in'/>
      </method>
      <method name='ActivateAction'>
        <arg type='s' name='action_name' direction='in'/>
        <arg type='av' name='parameter' direction='in'/>
        <arg type='a{sv}' name='platform_data' direction='in'/>
      </method>
    </interface>"

            DBusAdaptor {
                 iface: "com.example.service"

                function update() {
                    console.log("Update called")
                }
            }

            DBusAdaptor {
                iface: 'org.freedesktop.Application'

                function rcActivate(platformData) {
                    window.activate()
                }
                function rcOpen(uris, platformData) {
                    if (uris.length > 0) {
                        // open file
                    }
                    window.activate()
                }
                function rcActivateAction(name, parameters, platformData) {
                }
            }

        }
    }
    \endcode
*/

DeclarativeDBusObject::DeclarativeDBusObject(QObject *parent)
    : DeclarativeDBusAbstractObject(parent)
{
}

DeclarativeDBusObject::~DeclarativeDBusObject()
{
}

/*!
    \qmlproperty string DBusObject::service

    This property holds the registered service name. Typically this is in reversed domain-name.

    If an application implements multiple D-Bus interfaces for the same service, this property
    should be left blank and the service should be registered using QDBusConnection::registerService()
    after constructing the QML scene to ensure all objects have been registered first.
*/

/*!
    \qmlproperty string DBusObject::path

    This property holds the object path that this object will be published at.
*/

/*!
    \qmlproperty list<DBusAdaptor> DBusObject::adaptors

    This property holds a list of adaptors for interfaces implemented by the object.
*/

QQmlListProperty<QObject> DeclarativeDBusObject::adaptors()
{
    return QQmlListProperty<QObject>(
                this, 0, adaptor_append, adaptor_count, adaptor_at, adaptor_clear);
}

void DeclarativeDBusObject::componentComplete()
{
    for (QObject *object : m_objects) {
        if (DeclarativeDBusAdaptor *adaptor = qobject_cast<DeclarativeDBusAdaptor *>(object)) {
            const QString interface = adaptor->interface();

            if (!interface.isEmpty() && adaptor->service().isEmpty() && adaptor->path().isEmpty()) {
                adaptor->setPath(path());
                adaptor->setBus(bus());

                m_adaptors.insert(interface, adaptor);
            }
        }
    }

    DeclarativeDBusObject::componentComplete();
}

void DeclarativeDBusObject::adaptor_append(QQmlListProperty<QObject> *property, QObject *value)
{
    DeclarativeDBusObject * const object = static_cast<DeclarativeDBusObject *>(property->object);

    object->m_objects.append(value);

    emit object->adaptorsChanged();
}

QObject *DeclarativeDBusObject::adaptor_at(QQmlListProperty<QObject> *property, int index)
{
    return static_cast<DeclarativeDBusObject *>(property->object)->m_objects.value(index);
}

int DeclarativeDBusObject::adaptor_count(QQmlListProperty<QObject> *property)
{
    return static_cast<DeclarativeDBusObject *>(property->object)->m_objects.count();
}

void DeclarativeDBusObject::adaptor_clear(QQmlListProperty<QObject> *property)
{
    DeclarativeDBusObject * const object = static_cast<DeclarativeDBusObject *>(property->object);

    object->m_objects.clear();

    emit object->adaptorsChanged();
}

/*!
    \qmlproperty string DBusObject::xml

    This property holds the D-Bus introspection metadata snippet for this object.
*/

/*!
    \qmlproperty enum DBusObject::bus

    This property holds whether to use the session or system D-Bus.

    \list
        \li DBus.SessionBus - The D-Bus session bus
        \li DBus.SystemBus - The D-Bus system bus
    \endlist
*/

/*!
    \qmlproperty bool DBusObject::quitOnTimeout

    This property holds whether the object will attempt to quit the application if no windows are
    visible when the \l {quitTimeout}{quit timeout} expires.

    Use this if an application may be auto started to handle a DBus method call that may not show
    any windows. The application will be kept alive until the timeout expires and then if there are
    no  visible windows that will also keep it alive it will exit. Even if all the D-Bus methods
    provided do show a window it can still be helpful to enable this as a safety measure in case
    the application is started but no method is invoked because of external errors.

    Setting the this property to false after component construction has completed will cancel the
    timeout and it cannot be reenabled again after that. It is recommeded that the quit be cancelled
    after showing a window as turning the display off or switching to another application can
    temporarily hide the application windows causing it to exit erroneously if the timeout expires
    during this time.
*/

/*!
    \qmlproperty int DBusObject::quitTimeout

    The property holds the amount of time in seconds from component construction that the object
    will wait before quitting if \l quitOnTimeout is true.
*/

bool DeclarativeDBusObject::getProperty(
        const QDBusMessage &message,
        const QDBusConnection &connection,
        const QString &interface,
        const QString &name)
{
    if (DeclarativeDBusAdaptor * const adaptor = m_adaptors.value(interface)) {
        return adaptor->getProperty(message, connection, interface, name);
    } else {
        return false;
    }
}

bool DeclarativeDBusObject::getProperties(
        const QDBusMessage &message, const QDBusConnection &connection, const QString &interface)
{
    if (DeclarativeDBusAdaptor * const adaptor = m_adaptors.value(interface)) {
        return adaptor->getProperties(message, connection, interface);
    } else {
        return false;
    }
}

bool DeclarativeDBusObject::setProperty(
        const QString &interface, const QString &name, const QVariant &value)
{
    if (DeclarativeDBusAdaptor * const adaptor = m_adaptors.value(interface)) {
        return adaptor->setProperty(interface, name, value);
    } else {
        return false;
    }
}

bool DeclarativeDBusObject::invoke(
        const QDBusMessage &message,
        const QDBusConnection &connection,
        const QString &interface,
        const QString &name,
        const QVariantList &dbusArguments)
{
    if (DeclarativeDBusAdaptor * const adaptor = m_adaptors.value(interface)) {
        return adaptor->invoke(message, connection, interface, name, dbusArguments);
    } else {
        return false;
    }
}
