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

#include "declarativedbusadaptor.h"
#include "declarativedbusinterface.h"
#include "dbus.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QMetaMethod>

#include <qqmlinfo.h>
#include <QtDebug>


/*!
    \qmltype DBusAdaptor
    \inqmlmodule Nemo.DBus
    \brief Provides a service on D-Bus

    The DBusAdaptor object can be used to provide a D-Bus service on the system or session bus. A
    service can be called from other applications on the system as long as the service is active.

    DBusAdaptor is intended to provide a means of exposing simple objects over D-Bus. Property
    values and method arguments are automatically converted between QML/JS and D-Bus. There is
    limited control over this process. For more complex use cases it is recommended to use C++ and
    the Qt DBus module.

    \section2 Exposing an object on D-Bus

    The following code demonstrates how to expose an object on the session bus. The
    \c {com.example.service} service name will be registered and an object at
    \c {/com/example/service} will be registered supporting the \c {com.example.service} interface
    in addition to the common interfaces \c {org.freedesktop.DBus.Properties},
    \c {org.freedesktop.DBus.Introspectable} and \c {org.freedesktop.DBus.Peer}.

    All properties and methods of the DBusAdaptor will be accessible via D-Bus. Only those
    properties and methods declared in the \l xml will be discoverable via D-Bus introspection.

    \code
    import QtQuick 2.0
    import Nemo.DBus 2.0

    Item {
        DBusAdaptor {
            id: dbus

            property bool needUpdate: true

            service: 'com.example.service'
            iface: 'com.example.service'
            path: '/com/example/service'

            quitOnTimeout: !Qt.application.active

            xml: '  <interface name="com.example.service">\n' +
                 '    <method name="update" />\n' +
                 '    <property name="needUpdate" type="b" access="readwrite" />\n' +
                 '  </interface>\n'

            function update() {
                console.log("Update called")
            }
        }
    }
    \endcode
*/

DeclarativeDBusAdaptor::DeclarativeDBusAdaptor(QObject *parent)
    : DeclarativeDBusAbstractObject(parent)
{
}

DeclarativeDBusAdaptor::~DeclarativeDBusAdaptor()
{
}

/*!
    \qmlproperty string DBusAdaptor::service

    This property holds the registered service name. Typically this is in reversed domain-name.

    If an application implements multiple D-Bus interfaces for the same service, this property
    should be left blank and the service should be registered using QDBusConnection::registerService()
    after constructing the QML scene to ensure all objects have been registered first.
*/

/*!
    \qmlproperty string DBusAdaptor::path

    This property holds the object path that this object will be published at.
*/

/*!
    \qmlproperty string DBusAdaptor::iface

    This property holds the interface that this object supports.
*/
QString DeclarativeDBusAdaptor::interface() const
{
    return m_interface;
}

void DeclarativeDBusAdaptor::setInterface(const QString &interface)
{
    if (m_interface != interface) {
        m_interface = interface;
        emit interfaceChanged();
    }
}

/*!
    \qmlproperty string DBusAdaptor::xml

    This property holds the D-Bus introspection metadata snippet for this object/interface.
*/

/*!
    \qmlproperty enum DBusAdaptor::bus

    This property holds whether to use the session or system D-Bus.

    \list
        \li DBus.SessionBus - The D-Bus session bus
        \li DBus.SystemBus - The D-Bus system bus
    \endlist
*/

/*!
    \qmlproperty bool DBusAdaptor::quitOnTimeout

    This property holds whether the adaptor will attempt to quit the application if no windows are
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
    \qmlproperty int DBusAdaptor::quitTimeout

    The property holds the amount of time in seconds from component construction that the adaptor
    will wait before quitting if \l quitOnTimeout is true.
*/

QDBusArgument &operator << (QDBusArgument &argument, const QVariant &value)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    switch (value.typeId()) {
#else
    switch (value.type()) {
#endif
    case QVariant::String:
        return argument << value.toString();
    case QVariant::StringList:
        return argument << value.toStringList();
    case QVariant::Int:
        return argument << value.toInt();
    case QVariant::Bool:
        return argument << value.toBool();
    case QVariant::Double:
        return argument << value.toDouble();
    default:
        if (value.userType() == qMetaTypeId<float>()) {
            return argument << value.value<float>();
        }
        return argument;
    }
}


bool DeclarativeDBusAdaptor::getProperty(
        const QDBusMessage &message,
        const QDBusConnection &connection,
        const QString &interface,
        const QString &name)
{
    Q_UNUSED(interface);

    QString member = name;

    const QMetaObject *const meta = metaObject();
    if (!member.isEmpty() && member.at(0).isUpper())
        member = "rc" + member;

    for (int propertyIndex = meta->propertyOffset();
         propertyIndex < meta->propertyCount();
         ++propertyIndex) {
        QMetaProperty property = meta->property(propertyIndex);

        if (QLatin1String(property.name()) != member)
            continue;

        QVariant value = property.read(this);
        if (value.userType() == qMetaTypeId<QJSValue>())
            value = value.value<QJSValue>().toVariant();

        if (value.userType() == QVariant::List) {
            QVariantList variantList = value.toList();
            if (variantList.count() > 0) {

                QDBusArgument list;
                list.beginArray(variantList.first().userType());
                foreach (const QVariant &listValue, variantList) {
                    list << listValue;
                }
                list.endArray();
                value = QVariant::fromValue(list);
            }
        }

        QDBusMessage reply = message.createReply(QVariantList() << value);
        connection.call(reply, QDBus::NoBlock);

        return true;
    }

    return false;
}

bool DeclarativeDBusAdaptor::getProperties(
        const QDBusMessage &message, const QDBusConnection &connection, const QString &interface)
{
    Q_UNUSED(interface);

    const QMetaObject *const meta = metaObject();

    QDBusArgument map;
    map.beginMap(qMetaTypeId<QString>(), qMetaTypeId<QDBusVariant>());

    for (int propertyIndex = meta->propertyOffset();
         propertyIndex < meta->propertyCount();
         ++propertyIndex) {
        QMetaProperty property = meta->property(propertyIndex);

        QString propertyName = QLatin1String(property.name());
        if (propertyName.startsWith(QLatin1String("rc")))
            propertyName = propertyName.mid(2);

        QVariant value = property.read(this);
        if (value.userType() == qMetaTypeId<QJSValue>())
            value = value.value<QJSValue>().toVariant();

        if (value.userType() == QVariant::List) {
            QVariantList variantList = value.toList();
            if (variantList.count() > 0) {

                QDBusArgument list;
                list.beginArray(variantList.first().userType());
                foreach (const QVariant &listValue, variantList) {
                    list << listValue;
                }
                list.endArray();
                value = QVariant::fromValue(list);
            }
        }

        if (value.isValid()) {
            map.beginMapEntry();
            map << propertyName;
            map << QDBusVariant(value);
            map.endMapEntry();
        }
    }
    map.endMap();

    QDBusMessage reply = message.createReply(QVariantList() << QVariant::fromValue(map));
    connection.call(reply, QDBus::NoBlock);

    return true;
}

bool DeclarativeDBusAdaptor::setProperty(
        const QString &interface, const QString &name, const QVariant &value)
{
    Q_UNUSED(interface);

    QString member = name;

    const QMetaObject *const meta = metaObject();
    if (!member.isEmpty() && member.at(0).isUpper())
        member = "rc" + member;

    for (int propertyIndex = meta->propertyOffset();
         propertyIndex < meta->propertyCount();
         ++propertyIndex) {
        QMetaProperty property = meta->property(propertyIndex);

        if (QLatin1String(property.name()) != member)
            continue;

        return property.write(this, value);
    }

    return false;
}

bool DeclarativeDBusAdaptor::invoke(
        const QDBusMessage &message,
        const QDBusConnection &connection,
        const QString &interface,
        const QString &name,
        const QVariantList &dbusArguments)
{
    Q_UNUSED(interface);

    const QMetaObject *const meta = metaObject();

    QVariant variants[10];
    QGenericArgument arguments[10];

    QString member = name;

    // Support interfaces with method names starting with an uppercase letter.
    // com.example.interface.Foo -> com.example.interface.rcFoo (remote-call Foo).
    if (!member.isEmpty() && member.at(0).isUpper())
        member = "rc" + member;

    for (int methodIndex = meta->methodOffset(); methodIndex < meta->methodCount(); ++methodIndex) {
        const QMetaMethod method = meta->method(methodIndex);
        const QList<QByteArray> parameterTypes = method.parameterTypes();

        if (parameterTypes.count() != dbusArguments.count())
            continue;

        QByteArray sig(method.methodSignature());

        if (!sig.startsWith(member.toLatin1() + "("))
            continue;

        int argumentCount = 0;
        for (; argumentCount < 10 && argumentCount < dbusArguments.count(); ++argumentCount) {
            variants[argumentCount] = NemoDBus::demarshallDBusArgument(dbusArguments.at(argumentCount));
            QVariant &argument = variants[argumentCount];

            const QByteArray parameterType = parameterTypes.at(argumentCount);
            if (parameterType == "QVariant") {
                arguments[argumentCount] = QGenericArgument("QVariant", &argument);
            } else if (parameterType == argument.typeName()) {
                arguments[argumentCount] = QGenericArgument(argument.typeName(), argument.data());
            } else {
                // Type mismatch, there may be another overload.
                break;
            }
        }

        if (argumentCount == dbusArguments.length()) {
            if (method.methodType() == QMetaMethod::Signal) {
                return method.invoke(
                            this,
                            arguments[0],
                            arguments[1],
                            arguments[2],
                            arguments[3],
                            arguments[4],
                            arguments[5],
                            arguments[6],
                            arguments[7],
                            arguments[8],
                            arguments[9]);
            } else {
                QVariant retVal;
                bool success = method.invoke(
                            this,
                            Q_RETURN_ARG(QVariant, retVal),
                            arguments[0],
                            arguments[1],
                            arguments[2],
                            arguments[3],
                            arguments[4],
                            arguments[5],
                            arguments[6],
                            arguments[7],
                            arguments[8],
                            arguments[9]);
                if (success) {
                    QDBusMessage reply = retVal.isValid() ? message.createReply(retVal) : message.createReply();

                    connection.send(reply);
                }
                return success;
            }
        }
    }
    QByteArray signature = message.member().toLatin1() + "(";
    for (int i = 0; i < dbusArguments.count(); ++i) {
        if (i > 0)
            signature += ",";
        signature += dbusArguments.at(i).typeName();
    }
    signature += ")";

    qmlInfo(this) << "No method with the signature " << signature;
    return false;
}

/*!
    \qmlmethod void DBusAdaptor::emitSignal(string name, var arguments)

    Emit a signal with the given \a name and \a arguments. If \a arguments is undefined (the
    default if not specified), then the signal will be emitted without arguments.
*/
void DeclarativeDBusAdaptor::emitSignal(const QString &name, const QJSValue &arguments)
{
    QDBusMessage signal = QDBusMessage::createSignal(path(), m_interface, name);
    QDBusConnection conn = DeclarativeDBus::connection(bus());

    if (!arguments.isUndefined()) {
        signal.setArguments(DeclarativeDBusInterface::argumentsFromScriptValue(arguments));
    }

    if (!conn.send(signal))
        qmlInfo(this) << conn.lastError();
}
