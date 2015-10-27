/*****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
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
******************************************************************************/

import QtTest 1.0
import QtQuick 2.0
import org.nemomobile.dbus 2.0

TestCase {
    property int failCount : 0
    property int passCount : 0
    property bool finished

    function pass(title, have) {
        passCount += 1
        console.log(title+": got <"+have+">")
    }

    function fail(title, have, want) {
        failCount += 1
        console.warn(title+": got <"+have+"> wanted <"+want+">")
    }

    function finish() {
        console.log("passed:", passCount, "failed:", failCount)
        finished = true
    }

    function timeout() {
        console.warn("tests did not finish in time; quitting")
        finished = true
    }

    function repr(args, cb) {
        if (!testsrv.typedCall("repr",args, cb)) {
            cb('ERR')
        }
    }

    function echo(args, cb) {
        if (!testsrv.typedCall("echo",args, cb)) { cb('ERR') }
    }

    function ping(args, cb) {
        if (!testsrv.typedCall("ping",args, cb)) {
            cb('ERR')
        }
    }

    function seod(x) {
        if (x instanceof RegExp) {
            return x
        }
        if (x instanceof Array) {
            return JSON.stringify(x) + " (json)"
        }
        if (typeof x == "object") {
            var json = JSON.stringify(x)
            if (json != '{}') {
                return json + " (json)"
            }
        }
        return x
    }

    function eq(have, want) {
        if (want instanceof RegExp) {
            return want.test(have)
        }
        return have === want
    }

    function expect(title, have, want) {
        have = seod(have)
        want = seod(want)
        if (eq(have, want)) {
            pass(title,have)
        }
        else {
            fail(title,have,want)
        }
    }

    property var signalData : ''
    property var signalTodo : [
        ping, {type:'y', value:255},                "ping: byte",
        ping, {type:'b', value:true},               "ping: bool",
        ping, {type:'n', value:0x7fff},             "ping: int16",
        ping, {type:'i', value:0x7fffffff},         "ping: int32",
        ping, {type:'x', value:0x7fffffffffffffff}, "ping: int64",
        ping, {type:'q', value:0xffff},             "ping: uint16",
        ping, {type:'u', value:0xffffffff},         "ping: uint32",
        ping, {type:'t', value:0xffffffffffffffff}, "ping: uint64",
        ping, {type:'d', value:1.5},                "ping: double",
        ping, {type:'s', value:'kukkuu'},           "ping: string",
        ping, {type:'o', value:'/foo/bar'},         "ping: objpath",
        ping, {type:'g', value:'tussit'},           "ping: signature",
        ping, {type:'s', value:'COMPLEX1'},         "ping: complex1",
        ping, {type:'s', value:'COMPLEX2'},         "ping: complex2",
        ping, {type:'s', value:'COMPLEX3'},         "ping: complex3",
        ping, {type:'s', value:'COMPLEX4'},         "ping: complex4",
    ]

    function signalEnd(res) {
        var desc = signalTodo.shift()
        expect(desc, signalData, res)
        signalBeg()
    }

    function signalBeg() {
        signalData = ''
        if (signalTodo.length) {
            var func = signalTodo.shift()
            var type = signalTodo.shift()
            func(type, signalEnd)
        }
        else {
            finish()
        }
    }

    property var methodTodo : [
        repr, {type:'b',value:false},             "repr: boolean",               'boolean:false',
        echo, {type:'b',value:false},             "echo: boolean",               false,
        repr, {type:'b',value:true},              "repr: boolean",               'boolean:true',
        echo, {type:'b',value:true},              "echo: boolean",               true,
        repr, {type:'y',value:0},                 "repr: byte",                  'byte:0',
        echo, {type:'y',value:0},                 "echo: byte",                  0,
        repr, {type:'y',value:255},               "repr: byte",                  'byte:255',
        echo, {type:'y',value:255},               "echo: byte",                  255,
        repr, {type:'q',value:0},                 "repr: uint16",                'uint16:0',
        echo, {type:'q',value:0},                 "echo: uint16",                0,
        repr, {type:'q',value:65535},             "repr: uint16",                'uint16:65535',
        echo, {type:'q',value:65535},             "echo: uint16",                65535,
        repr, {type:'u',value:0},                 "repr: uint32",                'uint32:0',
        echo, {type:'u',value:0},                 "echo: uint32",                0,
        repr, {type:'u',value:4294967295},        "repr: uint32",                'uint32:4294967295',
        echo, {type:'u',value:4294967295},        "echo: uint32",                4294967295,
        repr, {type:'t',value:0},                 "repr: uint64",                'uint64:0',
        echo, {type:'t',value:0},                 "echo: uint64",                0,
        repr, {type:'t',value:4294967295},        "repr: uint64",                'uint64:4294967295',
        echo, {type:'t',value:4294967295},        "echo: uint64",                4294967295,
        repr, {type:'t',value:1099511627776},     "repr: uint64",                'uint64:1099511627776',
        echo, {type:'t',value:1099511627776},     "echo: uint64",                1099511627776,
        repr, {type:'n',value:-32768},            "repr: int16",                 'int16:-32768',
        echo, {type:'n',value:-32768},            "echo: int16",                 -32768,
        repr, {type:'n',value:32767},             "repr: int16",                 'int16:32767',
        echo, {type:'n',value:32767},             "echo: int16",                 32767,
        repr, {type:'i',value:-2147483648},       "repr: int32",                 'int32:-2147483648',
        echo, {type:'i',value:-2147483648},       "echo: int32",                 -2147483648,
        repr, {type:'i',value:2147483647},        "repr: int32",                 'int32:2147483647',
        echo, {type:'i',value:2147483647},        "echo: int32",                 2147483647,
        repr, {type:'x',value:-2147483648},       "repr: int64",                 'int64:-2147483648',
        echo, {type:'x',value:-2147483648},       "echo: int64",                 -2147483648,
        repr, {type:'x',value:2147483647},        "repr: int64",                 'int64:2147483647',
        echo, {type:'x',value:2147483647},        "echo: int64",                 2147483647,
        repr, {type:'x',value:-1099511627776},    "repr: int64",                 'int64:-1099511627776',
        echo, {type:'x',value:-1099511627776},    "echo: int64",                 -1099511627776,
        repr, {type:'x',value:1099511627776},     "repr: int64",                 'int64:1099511627776',
        echo, {type:'x',value:1099511627776},     "echo: int64",                 1099511627776,
        repr, {type:'d',value:2.75},              "repr: double",                'double:2.75',
        echo, {type:'d',value:2.75},              "echo: double",                2.75,
        repr, {type:'s',value:"FOO"},             "repr: string",                'string:"FOO"',
        echo, {type:'s',value:"FOO"},             "echo: string",                "FOO",
        repr, {type:'o',value:"/foo/bar"},        "repr: objpath",               'objpath:"/foo/bar"',
        echo, {type:'o',value:"/foo/bar"},        "echo: objpath",               "/foo/bar",
        repr, {type:'g',value:"tussi"},           "repr: signature",             'signature:"tussi"',
        echo, {type:'g',value:"tussi"},           "echo: signature",             "tussi",
        repr, {type:'h',value:2},                 "repr: fd",                    /^fd:[1-9][0-9]*$/,
        echo, {type:'h',value:2},                 "echo: fd",                    /^[1-9][0-9]*$/,
        repr, {type:'ab',value:[true,false,true]},"repr: array boolean",         'array [ boolean:true boolean:false boolean:true ]',
        echo, {type:'ab',value:[true,false,true]},"echo: array byte",            [true,false,true],
        repr, {type:'ay',value:[1,2,3]},          "repr: array byte",            'array [ byte:1 byte:2 byte:3 ]',
        echo, {type:'ay',value:[1,2,3]},          "echo: array byte",            [1,2,3],
        repr, {type:'aq',value:[1,2,3]},          "repr: array uint16",          'array [ uint16:1 uint16:2 uint16:3 ]',
        echo, {type:'aq',value:[1,2,3]},          "echo: array uint16",          [1,2,3],
        repr, {type:'au',value:[1,2,3]},          "repr: array uint32",          'array [ uint32:1 uint32:2 uint32:3 ]',
        echo, {type:'au',value:[1,2,3]},          "echo: array uint32",          [1,2,3],
        repr, {type:'at',value:[1,2,3]},          "repr: array uint64",          'array [ uint64:1 uint64:2 uint64:3 ]',
        echo, {type:'at',value:[1,2,3]},          "echo: array uint64",          [1,2,3],
        repr, {type:'an',value:[1,2,3]},          "repr: array int16",           'array [ int16:1 int16:2 int16:3 ]',
        echo, {type:'an',value:[1,2,3]},          "echo: array int16",           [1,2,3],
        repr, {type:'ai',value:[1,2,3]},          "repr: array int32",           'array [ int32:1 int32:2 int32:3 ]',
        echo, {type:'ai',value:[1,2,3]},          "echo: array int32",           [1,2,3],
        repr, {type:'ax',value:[1,2,3]},          "repr: array int64",           'array [ int64:1 int64:2 int64:3 ]',
        echo, {type:'ax',value:[1,2,3]},          "echo: array int64",           [1,2,3],
        repr, {type:'ad',value:[1.25,1.5,1.75]},  "repr: array double",           'array [ double:1.25 double:1.5 double:1.75 ]',
        echo, {type:'ad',value:[1.25,1.5,1.75]},  "echo: array double",           [1.25,1.5,1.75],
        repr, {type:'as',value:["a","b","c"]},    "repr: array string",          'array [ string:"a" string:"b" string:"c" ]',
        echo, {type:'as',value:["a","b","c"]},    "echo: array string",          ["a","b","c"],
        repr, {type:'v',value:true},              "repr: variant boolean",       'variant boolean:true',
        echo, {type:'v',value:true},              "echo: variant boolean",       true,
        repr, {type:'v',value:123},               "repr: variant int32",         'variant int32:123',
        echo, {type:'v',value:123},               "echo: variant int32",         123,
        repr, {type:'v',value:1.75},              "repr: variant double",        'variant double:1.75',
        echo, {type:'v',value:1.75},              "echo: variant double",        1.75,
        repr, {type:'v',value:"abc"},             "repr: variant string",        'variant string:"abc"',
        echo, {type:'v',value:"abc"},             "echo: variant string",        "abc",
        repr, {type:'v',value:[true,false,true]}, "repr: variant array boolean", 'variant array [ boolean:true boolean:false boolean:true ]',
        echo, {type:'v',value:[true,false,true]}, "echo: variant array boolean", [true,false,true],
        repr, {type:'v',value:[1,2,3]},           "repr: variant array int32",   'variant array [ int32:1 int32:2 int32:3 ]',
        echo, {type:'v',value:[1,2,3]},           "echo: variant array int32",   [1,2,3],
        repr, {type:'v',value:[1.25,1.5,1.75]},   "repr: variant array double",  'variant array [ double:1.25 double:1.5 double:1.75 ]',
        repr, {type:'v',value:["a","b","c"]},     "repr: variant array string",  'variant array [ string:"a" string:"b" string:"c" ]',
        echo, {type:'v',value:["a","b","c"]},     "echo: variant array string",  ["a","b","c"],
        echo, {type:'v',value:[1.25,1.5,1.75]},   "echo: variant array double",  [1.25,1.5,1.75],
        repr, {type:'v',value:[true,"a",1.5]},    "repr: variant array mixed",   'variant array [ variant boolean:true variant string:"a" variant double:1.5 ]',
        echo, {type:'v',value:[true,"a",1.5]},    "echo: variant array mixed",   [true,"a",1.5],
        repr, {type:'s',value:'COMPLEX1'},        "repr: complex1",              'variant int32:42',
        echo, {type:'s',value:'COMPLEX1'},        "echo: complex1",              42,
        repr, {type:'s',value:'COMPLEX2'},        "repr: complex2",              'array [ key string:"foo" val int32:1 key string:"bar" val int32:2 key string:"baf" val int32:3 ]',
        echo, {type:'s',value:'COMPLEX2'},        "echo: complex2",              {foo:1,bar:2,baf:3},
        repr, {type:'s',value:'COMPLEX3'},        "repr: complex3",              'array [ variant int32:4 variant int32:5 variant int32:6 ]',
        echo, {type:'s',value:'COMPLEX3'},        "echo: complex3",              [4,5,6],
        repr, {type:'s',value:'COMPLEX4'},        "repr: complex4",              'struct { byte:255 boolean:true int16:32767 int32:2147483647 int64:9223372036854775807 uint16:65535 uint32:4294967295 uint64:18446744073709551615 double:3.75 string:"string" objpath:"/obj/path" signature:"sointu" }',
        echo, {type:'s',value:'COMPLEX4'},        "echo: complex4",              [255,true,32767,2147483647,9223372036854775807,65535,4294967295,18446744073709551615,3.75,"string","/obj/path","sointu"],
    ]

    function methodEnd(res) {
        var desc = methodTodo.shift()
        var data = methodTodo.shift()
        expect(desc, res, data)
        methodBeg()
    }

    function methodBeg() {
        if (methodTodo.length) {
            var func = methodTodo.shift()
            var type = methodTodo.shift()
            func(type, methodEnd)
        }
        else {
            signalBeg()
        }
    }

    function test_typedCall() {
        methodBeg()

        while (!finished) {
            wait(500)
        }

        compare(failCount, 0)
    }

    function test_integerProperty() {
        tryCompare(testsrv, "integer", 12)

        testsrv.setProperty("Integer", 324)

        tryCompare(testsrv, "integer", 324)
    }

    function test_stringProperty() {
        tryCompare(testsrv, "string", "hello")

        testsrv.setProperty("String", "goodbye")

        tryCompare(testsrv, "string", "goodbye")
    }

    DBusInterface {
        id:              testsrv
        service:         'org.nemomobile.dbustestd'
        path:            '/'
        iface:           'org.nemomobile.dbustestd'
        signalsEnabled:  true
        propertiesEnabled: true

        property int integer
        property string string

        function pong(arg) {
            signalData = arg
        }
    }

    Timer {
        id: exitDelay
        interval: 10000
        running: true
        onTriggered: timeout()
    }
}
