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

#ifndef DECLARATIVEDBUSABSTRACTOBJECT_H
#define DECLARATIVEDBUSABSTRACTOBJECT_H

#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QUrl>
#include <QJSValue>

#include <QDBusVirtualObject>

#include <QEventLoopLocker>

#include "declarativedbus.h"

class DeclarativeDBusAbstractObject : public QDBusVirtualObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString xml READ xml WRITE setXml NOTIFY xmlChanged)
    Q_PROPERTY(DeclarativeDBus::BusType bus READ bus WRITE setBus NOTIFY busChanged)
    Q_PROPERTY(bool quitOnTimeout READ quitOnTimeout WRITE setQuitOnTimeout NOTIFY quitOnTimeoutChanged)
    Q_PROPERTY(int quitTimeout READ quitTimeout WRITE setQuitTimeout NOTIFY quitTimeoutChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    explicit DeclarativeDBusAbstractObject(QObject *parent = nullptr);
    ~DeclarativeDBusAbstractObject() override;

    QString service() const;
    void setService(const QString &service);

    QString path() const;
    void setPath(const QString &path);

    QString xml() const;
    void setXml(const QString &xml);

    DeclarativeDBus::BusType bus() const;
    void setBus(DeclarativeDBus::BusType bus);

    bool quitOnTimeout() const;
    void setQuitOnTimeout(bool quit);

    int quitTimeout() const;
    void setQuitTimeout(int timeout);

    void classBegin() override;
    void componentComplete() override;

    QString introspect(const QString &path) const override;
    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;

signals:
    void serviceChanged();
    void pathChanged();
    void xmlChanged();
    void busChanged();
    void quitOnTimeoutChanged();
    void quitTimeoutChanged();

protected:
    void timerEvent(QTimerEvent *event) override;

    virtual bool getProperty(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface,
            const QString &member) = 0;
    virtual bool getProperties(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface) = 0;
    virtual bool setProperty(
            const QString &interface, const QString &member, const QVariant &value) = 0;
    virtual bool invoke(
            const QDBusMessage &message,
            const QDBusConnection &connection,
            const QString &interface,
            const QString &name,
            const QVariantList &dbusArguments) = 0;

private:

    QScopedPointer<QEventLoopLocker> m_quitLocker;
    QString m_service;
    QString m_path;
    QString m_xml;
    DeclarativeDBus::BusType m_bus;
    int m_quitTimerId;
    int m_quitTimeout;
    bool m_quitOnTimeout;
    bool m_complete;
};

#endif
