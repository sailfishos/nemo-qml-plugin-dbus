/*
 * Copyright (C) 2016 Jolla Ltd
 * Contact: Andrew den Exter <andrew.den.exter@jolla.com>
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

#ifndef NEMODBUS_RESPONSE_H
#define NEMODBUS_RESPONSE_H

#include <nemo-dbus/dbus.h>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace NemoDBus
{

class NEMODBUS_EXPORT Response : public QDBusPendingCallWatcher
{
    Q_OBJECT
public:    
    ~Response();

    template <typename... Arguments, typename Handler>
    void onFinished(const Handler &handler)
    {
        connect(this, &Response::success, [handler](const QDBusPendingCall &call) {
            invoke<Handler, Arguments...>(handler, call);
        });
    }

    template <typename T> void onError(const T &handler)
    {
        connect(this, &Response::failure, [handler](const QDBusError &error) {
            handler(error);
        });
    }

    QString service() const { return m_service; }
    QString path() const { return m_path; }
    QString interface() const { return m_interface; }
    QString method() const { return m_method; }

signals:
    void success(const QDBusPendingCall &call);
    void failure(const QDBusError &error);

private:
    friend class ConnectionData;

    template <typename Handler> static void invoke(const Handler &handler, const QDBusPendingCall &) { handler(); }
    template <typename Handler, typename Argument0> static void invoke(
            const Handler &handler, const QDBusPendingCall &call) {
        const QDBusPendingReply<Argument0> reply(call);
        handler(demarshallArgument<Argument0>(reply.argumentAt(0))); }
    template <typename Handler, typename Argument0, typename Argument1> static void invoke(
            const Handler &handler, const QDBusPendingCall &call) {
        const QDBusPendingReply<Argument0, Argument1> reply(call);
        handler(demarshallArgument<Argument0>(reply.argumentAt(0)), demarshallArgument<Argument1>(reply.argumentAt(1))); }

    explicit Response(
            const QDBusPendingCall &call,
            const QString &service,
            const QString &path,
            const QString &interface,
            const QString &method,
            QObject *parent);

    const QString m_service;
    const QString m_path;
    const QString m_interface;
    const QString m_method;
};

}
#endif
