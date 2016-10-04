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

#include "declarativedbusinterface10.h"

DeclarativeDBusInterface10::DeclarativeDBusInterface10(QObject *parent)
    : DeclarativeDBusInterface(parent)
{
    connect(this, SIGNAL(serviceChanged()),
            this, SIGNAL(destinationChanged()));
    connect(this, SIGNAL(busChanged()),
            this, SIGNAL(busTypeChanged()));
}

DeclarativeDBusInterface10::~DeclarativeDBusInterface10()
{
}
