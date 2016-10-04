/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Robin Burchell <robin.burchell@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include <QtGlobal>

#include <QtQml>
#include <QQmlExtensionPlugin>

// API Version 1.0
#include "declarativedbusadaptor10.h"
#include "declarativedbusinterface10.h"

// API Version 2.0
#include "declarativedbus.h"
#include "declarativedbusadaptor.h"
#include "declarativedbusinterface.h"

class Q_DECL_EXPORT NemoDBusPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Nemo.DBus")
public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("Nemo.DBus") || uri == QLatin1String("org.nemomobile.dbus"));

        // QML API 1.0 (backwards compatible to 0.0.x versions)
        qmlRegisterType<DeclarativeDBusAdaptor10>(uri, 1, 0, "DBusAdaptor");
        qmlRegisterType<DeclarativeDBusInterface10>(uri, 1, 0, "DBusInterface");

        // QML API 2.0 (with deprecated fields removed)
        qmlRegisterUncreatableType<DeclarativeDBus>(uri, 2, 0, "DBus", "Cannot create DBus objects");
        qmlRegisterType<DeclarativeDBusAdaptor>(uri, 2, 0, "DBusAdaptor");
        qmlRegisterType<DeclarativeDBusInterface>(uri, 2, 0, "DBusInterface");
    }
};

#include "plugin.moc"
