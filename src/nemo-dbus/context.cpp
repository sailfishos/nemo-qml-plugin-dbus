/*
 * Copyright (C) 2017 Jolla Ltd
 * Contact: Bea Lam <bea.lam@jolla.com>
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

#include "context.h"

#include <QDBusConnectionInterface>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

namespace NemoDBus
{

Context::Context()
    : QDBusContext()
{
}

Context::~Context()
{
}

bool Context::calledFromPrivilegedProcess() const
{
    uint pid;

    if (calledFromDBus()) {
        QDBusConnectionInterface *iface = connection().interface();
        if (!iface) {
            qWarning() << "No D-Bus connection available!";
            return false;
        }

        QDBusReply<uint> callerServicePid = iface->servicePid(message().service());
        if (!callerServicePid.isValid()) {
            qWarning() << "servicePid() query failed:"
                       << callerServicePid.error().name()
                       << callerServicePid.error().message();
            return false;
        }

        pid = callerServicePid.value();
    } else {
        pid = QCoreApplication::applicationPid();
    }

    // The /proc/<pid> directory is owned by EUID:EGID of the process
    QFileInfo info(QStringLiteral("/proc/%1").arg(pid));
    return info.group() == QLatin1String("privileged") || info.owner() == QLatin1String("root");
}

}

