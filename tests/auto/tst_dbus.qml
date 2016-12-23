/****************************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Andrew den Exter <andrew.den.exter@jollamobile.com>
** All rights reserved.
** 
** This file is part of Sailfish Silica UI component package.
**
** You may use this file under the terms of BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Jolla Ltd nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
** ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************************/

import QtTest 1.0
import QtQuick 2.0
import Nemo.DBus 2.0

Item {
    width: 500; height: 500

    DBusAdaptor {
        id: dbusAdaptor

        service: "org.nemomobile.dbus.test"
        iface: "org.nemomobile.dbus.test.Interface"
        path: "/org/nemomobile/dbus/test"

        property string lastFunction
        property double doubleValue
        property int integerValue
        property string stringValue
        property variant variantValue
        property var stringListValue
        property string rcStringValue: "Capital"

        signal noArgs()
        signal doubleArg(double argument)
        signal integerArg(int argument)
        signal stringArg(string argument)
        signal multipleArgs(int argument1, string argument2, variant argument3)
        signal variantArg(variant argument)
        signal multipleTypedArgs(int argument1, double argument2)
        signal rcNoArgs()

        onNoArgs: {
            lastFunction = "noArgs"
        }

        onDoubleArg: {
            lastFunction = "doubleArg"
            doubleValue = argument
        }

        onIntegerArg: {
            lastFunction = "integerArg"
            integerValue = argument
        }

        onStringArg: {
            lastFunction = "stringArg"
            stringValue = argument
        }

        onMultipleArgs: {
            lastFunction = "multipleArgs"
            integerValue = argument1
            stringValue = argument2
            variantValue = argument3
        }

        onVariantArg: {
            lastFunction = "variantArg"
            variantValue = argument
        }

        onMultipleTypedArgs: {
            lastFunction = "multipleTypedArgs"
            integerValue = argument1
            doubleValue = argument2
        }

        onRcNoArgs: {
            lastFunction = "rcNoArgs"
        }

        function reset() {
            lastFunction = ""
            doubleValue = 0
            stringValue = ""
            variantValue = undefined
        }
    }

    DBusInterface {
        id: dbusInterface

        service: "org.nemomobile.dbus.test"
        iface: "org.nemomobile.dbus.test.Interface"
        path: "/org/nemomobile/dbus/test"
    }

    DBusInterface {
        id: dbusProperties

        service: dbusInterface.service
        iface: "org.freedesktop.DBus.Properties"
        path: dbusInterface.path
    }

    resources: TestCase {
        id: testCase

        name: "DBus"

        property bool properties_getAll_done: false

        function test_cleanup() {
            dbusAdaptor.reset()
        }

        function test_noArgs() {
            dbusInterface.call("noArgs", undefined)
            tryCompare(dbusAdaptor, "lastFunction", "noArgs")
        }

        function test_doubleArg() {
            dbusInterface.call("doubleArg", 5.2)
            tryCompare(dbusAdaptor, "lastFunction", "doubleArg")
            compare(dbusAdaptor.doubleValue, 5.2)

            compare(dbusInterface.getProperty("doubleValue"), 5.2)

            dbusInterface.setProperty("doubleValue", 6.3)
            tryCompare(dbusAdaptor, "doubleValue", 6.3)
        }

        function test_integerArg() {
            dbusInterface.call("integerArg", 5)
            tryCompare(dbusAdaptor, "lastFunction", "integerArg")
            compare(dbusAdaptor.integerValue, 5)

            compare(dbusInterface.getProperty("integerValue"), 5)

            dbusInterface.setProperty("integerValue", 6)
            tryCompare(dbusAdaptor, "integerValue", 6)
        }

        function test_stringArg() {
            dbusInterface.call("stringArg", "hello")
            tryCompare(dbusAdaptor, "lastFunction", "stringArg")
            compare(dbusAdaptor.stringValue, "hello")

            compare(dbusInterface.getProperty("stringValue"), "hello")

            dbusInterface.setProperty("stringValue", "goodbye")
            tryCompare(dbusAdaptor, "stringValue", "goodbye")
        }

        function test_stringList() {
            dbusInterface.call("variantArg", [[ "one", "two", "three" ]])
            tryCompare(dbusAdaptor, "lastFunction", "variantArg")
            compare(dbusAdaptor.variantValue.length, 3)
            compare(dbusAdaptor.variantValue, [ "one", "two", "three" ])

            compare(dbusInterface.getProperty("variantValue"), [ "one", "two", "three" ])

            dbusInterface.setProperty("variantValue", [ "four", "five", "six" ])
            tryCompare(dbusAdaptor, "variantValue", [ "four", "five", "six" ])
        }

        function test_multipleArgs() {
            dbusInterface.call("multipleArgs",[12, "bye", [ 1, 2, 3, 4 ]])
            tryCompare(dbusAdaptor, "lastFunction", "multipleArgs")
            compare(dbusAdaptor.integerValue, 12)
            compare(dbusAdaptor.stringValue, "bye")
            compare(dbusAdaptor.variantValue, [ 1, 2, 3, 4 ])
        }

        function test_typedCall() {
            dbusInterface.typedCall("integerArg", { 'type':'i', 'value': 5.2 })
            tryCompare(dbusAdaptor, "lastFunction", "integerArg")
            compare(dbusAdaptor.integerValue, 5)

            dbusInterface.typedCall("multipleTypedArgs", [ { 'type':'i', 'value': 5.2 }, { 'type':'d', 'value': 6.9 } ])
            tryCompare(dbusAdaptor, "lastFunction", "multipleTypedArgs")
            compare(dbusAdaptor.integerValue, 5)
            compare(dbusAdaptor.doubleValue, 6.9)
        }

        function test_object() {
            var object = new Object
            object.prop1 = "one"
            object.prop2 = "2"
            dbusInterface.call("variantArg", object)
            tryCompare(dbusAdaptor, "lastFunction", "variantArg")
            compare(dbusAdaptor.variantValue, object)
        }

        function test_upperCase() {
            dbusInterface.call("NoArgs", undefined)
            tryCompare(dbusAdaptor, "lastFunction", "rcNoArgs")

            compare(dbusInterface.getProperty("StringValue"), "Capital")

            dbusInterface.setProperty("StringValue", "CAPITAL")
            tryCompare(dbusAdaptor, "rcStringValue", "CAPITAL")
        }

        function test_properties_getAll() {
            dbusProperties.typedCall("GetAll", {'type': 's', 'value': dbusInterface.iface},
                                     function(result) {
                verify(true)
                for (var p in result) {
                    if (p[0] === p[0].toUpperCase())
                        compare(result[p], dbusAdaptor["rc" + p])
                    else
                        compare(result[p], dbusAdaptor[p])
                    testCase.properties_getAll_done = true
                }
            }, function() {
                verify(false)
                testCase.properties_getAll_done = true
            })
            tryCompare(testCase, "properties_getAll_done", true)
        }
    }
}
