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

#define TESTSRV_SERVICE   "org.nemomobile.dbustestd"
#define TESTSRV_INTERFACE "org.nemomobile.dbustestd"

#define TESTSRV_OBJ_ROOT "/"

#define TESTSRV_REQ_REPR "repr"
#define TESTSRV_REQ_ECHO "echo"
#define TESTSRV_REQ_PING "ping"
#define TESTSRV_REQ_QUIT "quit"

#define TESTSRV_SIG_PONG "pong"

#define TESTSRV_PROP_INTEGER "Integer"
#define TESTSRV_PROP_STRING "String"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>

#include <dbus/dbus-glib-lowlevel.h>

#define log_emit(LEV,FMT,ARGS...) syslog(LEV, FMT, ## ARGS)

/* ========================================================================= *
 * TYPES & FUNCTIONS
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * DBUS_HELPERS
 * ------------------------------------------------------------------------- */

typedef union
{
    dbus_int16_t   i16;
    dbus_int32_t   i32;
    dbus_int64_t   i64;

    dbus_uint16_t  u16;
    dbus_uint32_t  u32;
    dbus_uint64_t  u64;

    dbus_bool_t    b;
    unsigned char  o;
    const char    *s;
    double         d;
    int            fd;

} xdbus_any_t;

#define XDBUS_ANY_INIT { .u64 = 0 }

static bool  xdbus_message_repr_sub          (FILE *file, DBusMessageIter *iter);
static char *xdbus_message_repr              (DBusMessage *const msg);
static char *xdbus_message_iter_get_signature(DBusMessageIter *iter);
static bool  xdbus_message_copy_sub          (DBusMessageIter *dest, DBusMessageIter *srce);

/* ------------------------------------------------------------------------- *
 * SERVICE
 * ------------------------------------------------------------------------- */

typedef DBusMessage *(*service_handler_t)(DBusMessage *req);
typedef void (*service_property_getter_t)(DBusMessageIter *iter);
typedef void (*service_property_setter_t)(DBusMessageIter *iter);

typedef struct
{
  const char        *sm_interface;
  const char        *sm_member;
  service_handler_t  sm_handler;
} service_method_t;

typedef struct
{
  const char                *sp_interface;
  const char                *sp_member;
  service_property_getter_t  sp_getter;
  service_property_setter_t  sp_setter;
} service_property_t;

static void               service_inject_dict_entry    (DBusMessageIter *arr, const char *key, int val);
static void               service_inject_dict          (DBusMessageIter *body);
static void               service_inject_array_item    (DBusMessageIter *arr, int val);
static void               service_inject_array         (DBusMessageIter *body);
static void               service_inject_struct        (DBusMessageIter *body);
static void               service_inject_variant_int32 (DBusMessageIter *body);
static DBusMessage       *service_inject_fake_args     (DBusMessage *req);

static DBusMessage       *service_handle_introspect_req(DBusMessage *req);
static DBusMessage       *service_handle_repr_req      (DBusMessage *req);
static DBusMessage       *service_handle_echo_req      (DBusMessage *req);
static DBusMessage       *service_handle_ping_req      (DBusMessage *req);
static DBusMessage       *service_handle_quit_req      (DBusMessage *req);

static service_handler_t  service_get_handler          (const char *interface, const char *member);

static DBusHandlerResult  service_filter_cb            (DBusConnection *con, DBusMessage *msg, gpointer aptr);

static bool               service_init                 (void);
static void               service_quit                 (void);

/* ------------------------------------------------------------------------- *
 * MAINLOOP
 * ------------------------------------------------------------------------- */

static void mainloop_exit(int xc);
static int  mainloop_run (void);

/* ------------------------------------------------------------------------- *
 * STAYALIVE
 * ------------------------------------------------------------------------- */

static gboolean stayalive_timer_cb(gpointer aptr);
static void     stayalive_renew   (void);
static void     stayalive_quit    (void);

/* ------------------------------------------------------------------------- *
 * ENTRY_POINT
 * ------------------------------------------------------------------------- */

int main(int ac, char **av);

/* ========================================================================= *
 * DBUS_HELPERS
 * ========================================================================= */

static bool
xdbus_message_repr_sub(FILE *file, DBusMessageIter *iter)
{
    xdbus_any_t     val = XDBUS_ANY_INIT;
    DBusMessageIter sub;

    switch( dbus_message_iter_get_arg_type(iter) ) {
    case DBUS_TYPE_INVALID:
        return false;
    default:
        fprintf(file, " unknown");
        break;
    case DBUS_TYPE_UNIX_FD:
        dbus_message_iter_get_basic(iter, &val.fd);
        fprintf(file, " fd:%d", val.fd);
        break;
    case DBUS_TYPE_BYTE:
        dbus_message_iter_get_basic(iter, &val.o);
        fprintf(file, " byte:%d", val.o);
        break;
    case DBUS_TYPE_BOOLEAN:
        dbus_message_iter_get_basic(iter, &val.b);
        fprintf(file, " boolean:%s", val.b ? "true" : "false");
        break;
    case DBUS_TYPE_INT16:
        dbus_message_iter_get_basic(iter, &val.i16);
        fprintf(file, " int16:%d", val.i16);
        break;
    case DBUS_TYPE_INT32:
        dbus_message_iter_get_basic(iter, &val.i32);
        fprintf(file, " int32:%d", val.i32);
        break;
    case DBUS_TYPE_INT64:
        dbus_message_iter_get_basic(iter, &val.i64);
        fprintf(file, " int64:%lld", (long long)val.i64);
        break;
    case DBUS_TYPE_UINT16:
        dbus_message_iter_get_basic(iter, &val.u16);
        fprintf(file, " uint16:%u", val.u16);
        break;
    case DBUS_TYPE_UINT32:
        dbus_message_iter_get_basic(iter, &val.u32);
        fprintf(file, " uint32:%u", val.u32);
        break;
    case DBUS_TYPE_UINT64:
        dbus_message_iter_get_basic(iter, &val.u64);
        fprintf(file, " uint64:%llu", (unsigned long long)val.u64);
        break;
    case DBUS_TYPE_DOUBLE:
        dbus_message_iter_get_basic(iter, &val.d);
        fprintf(file, " double:%g", val.d);
        break;
    case DBUS_TYPE_STRING:
        dbus_message_iter_get_basic(iter, &val.s);
        fprintf(file, " string:\"%s\"", val.s);
        break;
    case DBUS_TYPE_OBJECT_PATH:
        dbus_message_iter_get_basic(iter, &val.s);
        fprintf(file, " objpath:\"%s\"", val.s);
        break;
    case DBUS_TYPE_SIGNATURE:
        dbus_message_iter_get_basic(iter, &val.s);
        fprintf(file, " signature:\"%s\"", val.s);
        break;
    case DBUS_TYPE_ARRAY:
        dbus_message_iter_recurse(iter, &sub);
        fprintf(file, " array [");
        while( xdbus_message_repr_sub(file, &sub) ) {}
        fprintf(file, " ]");
        break;
    case DBUS_TYPE_VARIANT:
        dbus_message_iter_recurse(iter, &sub);
        fprintf(file, " variant");
        xdbus_message_repr_sub(file, &sub);
        break;
    case DBUS_TYPE_STRUCT:
        dbus_message_iter_recurse(iter, &sub);
        fprintf(file, " struct {");
        while( xdbus_message_repr_sub(file, &sub) ) {}
        fprintf(file, " }");
        break;
    case DBUS_TYPE_DICT_ENTRY:
        dbus_message_iter_recurse(iter, &sub);
        fprintf(file, " key");
        xdbus_message_repr_sub(file, &sub);
        fprintf(file, " val");
        xdbus_message_repr_sub(file, &sub);
        break;
    }

    return dbus_message_iter_next(iter);
}

static char *
xdbus_message_repr(DBusMessage *const msg)
{
    size_t  size = 0;
    char   *data = 0;
    FILE   *file = open_memstream(&data, &size);

    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);
    while( xdbus_message_repr_sub(file, &iter) ) {}

    fclose(file);

    if( data && *data == ' ' )
        memmove(data, data+1, strlen(data));

    return data;
}

static char *
xdbus_message_iter_get_signature(DBusMessageIter *iter)
{
    DBusMessageIter sub;
    dbus_message_iter_recurse(iter, &sub);
    return dbus_message_iter_get_signature(&sub);
}

static bool
xdbus_message_copy_sub(DBusMessageIter *dest, DBusMessageIter *srce)
{
    bool        res = false;
    xdbus_any_t val = XDBUS_ANY_INIT;
    int         at  = dbus_message_iter_get_arg_type(srce);
    char       *sgn = 0;

    DBusMessageIter sub_srce, sub_dest;

    switch( at ) {
    case DBUS_TYPE_INVALID:
        goto EXIT;
    default:
        goto NEXT;
    case DBUS_TYPE_UNIX_FD:
    case DBUS_TYPE_BYTE:
    case DBUS_TYPE_BOOLEAN:
    case DBUS_TYPE_INT16:
    case DBUS_TYPE_INT32:
    case DBUS_TYPE_INT64:
    case DBUS_TYPE_UINT16:
    case DBUS_TYPE_UINT32:
    case DBUS_TYPE_UINT64:
    case DBUS_TYPE_DOUBLE:
    case DBUS_TYPE_STRING:
    case DBUS_TYPE_OBJECT_PATH:
    case DBUS_TYPE_SIGNATURE:
        dbus_message_iter_get_basic(srce, &val);
        dbus_message_iter_append_basic(dest, at, &val);
        break;
    case DBUS_TYPE_ARRAY:
    case DBUS_TYPE_VARIANT:
        sgn = xdbus_message_iter_get_signature(srce);
        // fall through
    case DBUS_TYPE_STRUCT:
    case DBUS_TYPE_DICT_ENTRY:
        dbus_message_iter_recurse(srce, &sub_srce);
        if( !dbus_message_iter_open_container(dest, at, sgn, &sub_dest) )
            goto EXIT;
        while( xdbus_message_copy_sub(&sub_dest, &sub_srce) ) {}
        if( !dbus_message_iter_close_container(dest, &sub_dest) )
            goto EXIT;
        break;
    }

NEXT:
    res = dbus_message_iter_next(srce);

EXIT:
    dbus_free(sgn);

    return res;
}

static void
xdbus_message_iter_open_variant_map(DBusMessageIter *dst, DBusMessageIter *arr)
{
    dbus_message_iter_open_container(dst,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING
                                     DBUS_TYPE_VARIANT_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                                     arr);
}

static void
xdbus_message_iter_append_variant(DBusMessageIter *dst, int type, const char *type_string, const void *value)
{
    DBusMessageIter var;

    dbus_message_iter_open_container(dst, DBUS_TYPE_VARIANT, type_string, &var);
    dbus_message_iter_append_basic(&var, type, value);
    dbus_message_iter_close_container(dst, &var);
}

static void
xdbus_signal_property_changed(DBusConnection *conn, const char *interface, const char *member, int type, const char *type_string, const void *value)
{
    DBusMessage *sig = 0;
    DBusMessageIter dst, arr, ent;

    sig = dbus_message_new_signal(TESTSRV_OBJ_ROOT,
                                  "org.freedesktop.DBus.Properties",
                                  "PropertiesChanged");
    if( !sig )
        return;

    dbus_message_iter_init_append(sig, &dst);
    dbus_message_iter_append_basic(&dst, DBUS_TYPE_STRING, &interface);

    xdbus_message_iter_open_variant_map(&dst, &arr);
    dbus_message_iter_open_container(&arr,
                                     DBUS_TYPE_DICT_ENTRY,
                                     0,
                                     &ent);
    dbus_message_iter_append_basic(&ent, DBUS_TYPE_STRING, &member);
    xdbus_message_iter_append_variant(&ent, type, type_string, value);
    dbus_message_iter_close_container(&arr, &ent);
    dbus_message_iter_close_container(&dst, &arr);

    dbus_message_iter_open_container(&dst,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_TYPE_STRING_AS_STRING,
                                     &arr);
    dbus_message_iter_close_container(&dst, &arr);

    dbus_connection_send(conn, sig, 0);
    dbus_message_unref(sig);
}

static void
xdbus_signal_property_invalidated(DBusConnection *conn, const char *interface, const char *member)
{
    DBusMessage *sig = 0;
    DBusMessageIter dst, arr;

    sig = dbus_message_new_signal(TESTSRV_OBJ_ROOT,
                                  "org.freedesktop.DBus.Properties",
                                  "PropertiesChanged");
    if( !sig )
        return;

    dbus_message_iter_init_append(sig, &dst);
    dbus_message_iter_append_basic(&dst, DBUS_TYPE_STRING, &interface);

    xdbus_message_iter_open_variant_map(&dst, &arr);
    dbus_message_iter_close_container(&dst, &arr);

    dbus_message_iter_open_container(&dst,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_TYPE_STRING_AS_STRING,
                                     &arr);
    dbus_message_iter_append_basic(&arr, DBUS_TYPE_STRING, &member);
    dbus_message_iter_close_container(&dst, &arr);

    dbus_connection_send(conn, sig, 0);
    dbus_message_unref(sig);
}

/* ========================================================================= *
 * SERVICE
 * ========================================================================= */

static const char service_xml[] =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\""
" \"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg direction=\"out\" name=\"data\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.DBus.Peer\">\n"
"    <method name=\"Ping\"/>\n"
"    <method name=\"GetMachineId\">\n"
"      <arg direction=\"out\" name=\"machine_uuid\" type=\"s\" />\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.DBus.Properties\">\n"
"    <method name=\"Properties\">\n"
"      <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"property\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"value\" direction=\"out\" type=\"v\"/>\n"
"    </method>\n"
"    <method name=\"GetAll\">\n"
"      <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"properties\" direction=\"out\" type=\"a{sv}\"/>\n"
"    </method>\n"
"    <method name=\"Set\">\n"
"      <arg name=\"interface\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"property\" direction=\"in\" type=\"s\"/>\n"
"      <arg name=\"value\" direction=\"in\" type=\"v\"/>\n"
"    </method>\n"
"    <signal name=\"PropertiesChanged\">\n"
"      <arg type=\"s\" name=\"interface\"/>\n"
"      <arg type=\"a{sv}\" name=\"changed_properties\"/>\n"
"      <arg type=\"as\" name=\"invalidated_properties\"/>\n"
"    </signal>\n"
"  </interface>\n"
"  <interface name=\""TESTSRV_INTERFACE"\">\n"
"    <method name=\""TESTSRV_REQ_REPR"\">\n"
"      <arg direction=\"out\" name=\"args_as_string\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\""TESTSRV_REQ_ECHO"\">\n"
"      <arg direction=\"out\" name=\"args_as_is\"/>\n"
"    </method>\n"
"    <method name=\""TESTSRV_REQ_PING"\">\n"
"      <arg direction=\"out\" name=\"args_as_is\" />\n"
"    </method>\n"
"    <method name=\""TESTSRV_REQ_QUIT"\"/>\n"
"    <signal name=\""TESTSRV_SIG_PONG"\">\n"
"      <arg name=\"args_to_ping_as_is\" />\n"
"    </signal>\n"
"    <property name=\""TESTSRV_PROP_INTEGER"\" type=\"i\" access=\"readwrite\"/>\n"
"    <property name=\""TESTSRV_PROP_STRING"\" type=\"s\" access=\"readwrite\"/>\n"
"  </interface>\n"
"</node>\n"
;

static DBusConnection *service_con = 0;

static void
service_inject_dict_entry(DBusMessageIter *arr, const char *key, int val)
{
    xdbus_any_t arg = XDBUS_ANY_INIT;
    DBusMessageIter ent;

    dbus_message_iter_open_container(arr,
                                     DBUS_TYPE_DICT_ENTRY,
                                     0,
                                     &ent);
    arg.s = key;
    dbus_message_iter_append_basic(&ent, DBUS_TYPE_STRING, &arg);

    arg.i32 = val;
    dbus_message_iter_append_basic(&ent, DBUS_TYPE_INT32, &arg);

    dbus_message_iter_close_container(arr, &ent);

}

static void
service_inject_dict(DBusMessageIter *body)
{
    DBusMessageIter arr;

    dbus_message_iter_open_container(body,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_STRING_AS_STRING
                                     DBUS_TYPE_INT32_AS_STRING
                                     DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                                     &arr);

    service_inject_dict_entry(&arr, "foo", 1);
    service_inject_dict_entry(&arr, "bar", 2);
    service_inject_dict_entry(&arr, "baf", 3);

    dbus_message_iter_close_container(body, &arr);
}

static void
service_inject_array_item(DBusMessageIter *arr, int val)
{
    xdbus_any_t arg = XDBUS_ANY_INIT;
    DBusMessageIter var;

    dbus_message_iter_open_container(arr,
                                     DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_INT32_AS_STRING,
                                     &var);
    arg.i32 = val;
    dbus_message_iter_append_basic(&var, DBUS_TYPE_INT32, &arg);

    dbus_message_iter_close_container(arr, &var);

}

static void
service_inject_array(DBusMessageIter *body)
{
    DBusMessageIter arr;

    dbus_message_iter_open_container(body,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_TYPE_VARIANT_AS_STRING,
                                     &arr);
    service_inject_array_item(&arr, 4);
    service_inject_array_item(&arr, 5);
    service_inject_array_item(&arr, 6);
    dbus_message_iter_close_container(body, &arr);
}

static void
service_inject_struct(DBusMessageIter *body)
{
    xdbus_any_t arg = XDBUS_ANY_INIT;
    DBusMessageIter sub;

    dbus_message_iter_open_container(body,
                                     DBUS_TYPE_STRUCT,
                                     0,
                                     &sub);

    arg.o = 255;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_BYTE, &arg);

    arg.b = true;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_BOOLEAN, &arg);

    arg.i16 = 0x7fff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_INT16, &arg);

    arg.i32 = 0x7fffffff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_INT32, &arg);

    arg.i64 = 0x7fffffffffffffff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_INT64, &arg);

    arg.u16 = 0xffff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_UINT16, &arg);

    arg.u32 = 0xffffffff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_UINT32, &arg);

    arg.u64 = 0xffffffffffffffff;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_UINT64, &arg);

    arg.d = 3.75;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_DOUBLE, &arg);

    arg.s = "string";
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_STRING, &arg);

    arg.s = "/obj/path";
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_OBJECT_PATH, &arg);

    arg.s = "sointu";
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_SIGNATURE, &arg);

    dbus_message_iter_close_container(body, &sub);
}

static void
service_inject_variant_int32(DBusMessageIter *body)
{
    xdbus_any_t val = { .i32 = 42 };
    DBusMessageIter var;
    dbus_message_iter_open_container(body,
                                     DBUS_TYPE_VARIANT,
                                     "i", &var);
    dbus_message_iter_append_basic(&var, DBUS_TYPE_INT32, &val);
    dbus_message_iter_close_container(body, &var);
}

static DBusMessage *
service_inject_fake_args(DBusMessage *req)
{
    DBusMessage *fake = 0;
    DBusMessage *work = 0;
    char        *name = 0;
    DBusMessageIter iter;

    dbus_message_iter_init(req, &iter);

    if( dbus_message_iter_get_arg_type(&iter) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&iter, &name);

    work = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_CALL);
    if( !work )
        goto EXIT;

    dbus_message_iter_init_append(work, &iter);

    if( !strcmp(name, "COMPLEX1") )
        service_inject_variant_int32(&iter);
    else if(!strcmp(name, "COMPLEX2") )
        service_inject_dict(&iter);
    else if(!strcmp(name, "COMPLEX3") )
        service_inject_array(&iter);
    else if(!strcmp(name, "COMPLEX4") )
        service_inject_struct(&iter);
    else
        goto EXIT;

    fake = work, work = 0;

EXIT:
    if( work )
        dbus_message_unref(work);

    return fake;
}

static DBusMessage *
service_handle_introspect_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    const char  *str = service_xml;

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    dbus_message_append_args(rsp,
                             DBUS_TYPE_STRING, &str,
                             DBUS_TYPE_INVALID);

EXIT:
    return rsp;
}

static DBusMessage *
service_handle_repr_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    char        *str = 0;
    DBusMessage *inj = service_inject_fake_args(req);;

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    if( !(str = xdbus_message_repr(inj ?: req)) )
        goto EXIT;

    dbus_message_append_args(rsp,
                             DBUS_TYPE_STRING, &str,
                             DBUS_TYPE_INVALID);

EXIT:
    free(str);

    if( inj )
        dbus_message_unref(inj);

    return rsp;
}

static DBusMessage *
service_handle_echo_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    DBusMessage *inj = service_inject_fake_args(req);;

    DBusMessageIter src, dst;

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    dbus_message_iter_init(inj ?: req, &src);
    dbus_message_iter_init_append(rsp, &dst);
    while( xdbus_message_copy_sub(&dst, &src) ) {}

EXIT:
    if( inj )
        dbus_message_unref(inj);

    return rsp;
}

static DBusMessage *
service_handle_ping_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    DBusMessage *sig = 0;
    DBusMessage *inj = service_inject_fake_args(req);;

    DBusMessageIter src, dst;

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    sig = dbus_message_new_signal(TESTSRV_OBJ_ROOT,
                                  TESTSRV_INTERFACE,
                                  TESTSRV_SIG_PONG);
    if( !sig )
        goto EXIT;

    dbus_message_iter_init(inj ?: req, &src);
    dbus_message_iter_init_append(sig, &dst);
    while( xdbus_message_copy_sub(&dst, &src) ) {}

    dbus_message_iter_init(inj ?: req, &src);
    dbus_message_iter_init_append(rsp, &dst);
    while( xdbus_message_copy_sub(&dst, &src) ) {}

EXIT:
    // send signal 1st, then reply -> client should
    // have gotten the sig when they get the reply
    if( sig )  {
        dbus_connection_send(service_con, sig, 0);
        dbus_message_unref(sig);
    }

    if( inj )
        dbus_message_unref(inj);

    return rsp;
}


static DBusMessage *
service_handle_quit_req(DBusMessage *req)
{
    DBusMessage *rsp = dbus_message_new_method_return(req);

    mainloop_exit(EXIT_SUCCESS);

    return rsp;
}

static int service_integer_property = 12;

static void
service_get_integer_property(DBusMessageIter *dst)
{
    xdbus_message_iter_append_variant(dst,
                                      DBUS_TYPE_INT32,
                                      DBUS_TYPE_INT32_AS_STRING,
                                      &service_integer_property);
}

static void
service_set_integer_property(DBusMessageIter *src)
{
    if( dbus_message_iter_get_arg_type(src) !=  DBUS_TYPE_INT32 ) {
        log_emit(LOG_NOTICE, "property type is not int32");
        return;
    }

    dbus_message_iter_get_basic(src, &service_integer_property);

    xdbus_signal_property_changed(service_con,
                                  TESTSRV_INTERFACE,
                                  TESTSRV_PROP_INTEGER,
                                  DBUS_TYPE_INT32,
                                  DBUS_TYPE_INT32_AS_STRING,
                                  &service_integer_property);
}

static char service_string_property[255] = "hello";

static void
service_get_string_property(DBusMessageIter *dst)
{
    char *temp = service_string_property;
    xdbus_message_iter_append_variant(dst,
                                     DBUS_TYPE_STRING,
                                     DBUS_TYPE_STRING_AS_STRING,
                                     &temp);
}

static void
service_set_string_property(DBusMessageIter *src)
{
    char *temp = 0;

    if( dbus_message_iter_get_arg_type(src) !=  DBUS_TYPE_STRING ) {
        log_emit(LOG_NOTICE, "property type is not string");
        return;
    }

    dbus_message_iter_get_basic(src, &temp);

    strncpy(service_string_property, temp, G_N_ELEMENTS(service_string_property) - 1);

    xdbus_signal_property_invalidated(service_con, TESTSRV_INTERFACE, TESTSRV_PROP_STRING);
}

static const service_property_t service_property_lut[] =
{
  {
    .sp_interface = TESTSRV_INTERFACE,
    .sp_member    = TESTSRV_PROP_INTEGER,
    .sp_getter    = service_get_integer_property,
    .sp_setter    = service_set_integer_property,
  },
  {
    .sp_interface = TESTSRV_INTERFACE,
    .sp_member    = TESTSRV_PROP_STRING,
    .sp_getter    = service_get_string_property,
    .sp_setter    = service_set_string_property,
  },
};

static const service_property_t *
service_get_property(const char *interface, const char *member)
{
    const service_property_t * property = 0;

    for( size_t i = 0; i < G_N_ELEMENTS(service_property_lut); ++i ) {
        // test member name first because they are shorter and more
        // likely to be unique than interface names
        if( strcmp(service_property_lut[i].sp_member, member) )
            continue;

        if( strcmp(service_property_lut[i].sp_interface, interface) )
            continue;

        property = &service_property_lut[i];
        break;
    }

    return property;
}

static DBusMessage *
service_handle_get_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    char        *interface = 0;
    char        *member = 0;
    const service_property_t *property;
    DBusMessageIter src, dst;

    dbus_message_iter_init(req, &src);

    if( dbus_message_iter_get_arg_type(&src) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&src, &interface);

    if( !dbus_message_iter_next(&src) || dbus_message_iter_get_arg_type(&src) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&src, &member);

    property = service_get_property(interface, member);
    if( !property )
        goto EXIT;

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    dbus_message_iter_init_append(rsp, &dst);

    (*property->sp_getter)(&dst);

EXIT:
    return rsp;
}

static DBusMessage *
service_handle_get_all_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    char        *interface = 0;
    DBusMessageIter src, dst, arr, ent;

    dbus_message_iter_init(req, &src);

    if( dbus_message_iter_get_arg_type(&src) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&src, &interface);

    if( !(rsp = dbus_message_new_method_return(req)) )
        goto EXIT;

    dbus_message_iter_init_append(rsp, &dst);

    xdbus_message_iter_open_variant_map(&dst, &arr);

    for( size_t i = 0; i < G_N_ELEMENTS(service_property_lut); ++i ) {
        if( strcmp(service_property_lut[i].sp_interface, interface) )
            continue;

        dbus_message_iter_open_container(&arr,
                                         DBUS_TYPE_DICT_ENTRY,
                                         0,
                                         &ent);

        dbus_message_iter_append_basic(&ent,
                                       DBUS_TYPE_STRING,
                                       &service_property_lut[i].sp_member);

        (*service_property_lut[i].sp_getter)(&ent);

        dbus_message_iter_close_container(&arr, &ent);
    }

    dbus_message_iter_close_container(&dst, &arr);

EXIT:
    return rsp;
}

static DBusMessage *
service_handle_set_req(DBusMessage *req)
{
    DBusMessage *rsp = 0;
    char        *interface = 0;
    char        *member = 0;
    const service_property_t *property;
    DBusMessageIter src, var;

    dbus_message_iter_init(req, &src);

    if( dbus_message_iter_get_arg_type(&src) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&src, &interface);

    if( !dbus_message_iter_next(&src) || dbus_message_iter_get_arg_type(&src) !=  DBUS_TYPE_STRING )
        goto EXIT;

    dbus_message_iter_get_basic(&src, &member);

    property = service_get_property(interface, member);
    if( !property )
        goto EXIT;

    if( !dbus_message_iter_next(&src) || dbus_message_iter_get_arg_type(&src) != DBUS_TYPE_VARIANT )
        goto EXIT;

    dbus_message_iter_recurse(&src, &var);

    (*property->sp_setter)(&var);

    rsp = dbus_message_new_method_return(req);

EXIT:
    return rsp;
}

static const service_method_t service_method_lut[] =
{
  {
    .sm_interface = "org.freedesktop.DBus.Introspectable",
    .sm_member    = "Introspect",
    .sm_handler   = service_handle_introspect_req,
  },
  {
    .sm_interface = TESTSRV_INTERFACE,
    .sm_member    = TESTSRV_REQ_REPR,
    .sm_handler   = service_handle_repr_req,
  },
  {
    .sm_interface = TESTSRV_INTERFACE,
    .sm_member    = TESTSRV_REQ_ECHO,
    .sm_handler   = service_handle_echo_req,
  },
  {
    .sm_interface = TESTSRV_INTERFACE,
    .sm_member    = TESTSRV_REQ_PING,
    .sm_handler   = service_handle_ping_req,
  },
  {
    .sm_interface = TESTSRV_INTERFACE,
    .sm_member    = TESTSRV_REQ_QUIT,
    .sm_handler   = service_handle_quit_req,
  },
  {
    .sm_interface = "org.freedesktop.DBus.Properties",
    .sm_member    = "Get",
    .sm_handler   = service_handle_get_req,
  },
  {
    .sm_interface = "org.freedesktop.DBus.Properties",
    .sm_member    = "GetAll",
    .sm_handler   = service_handle_get_all_req,
  },
  {
    .sm_interface = "org.freedesktop.DBus.Properties",
    .sm_member    = "Set",
    .sm_handler   = service_handle_set_req,
  },
};

static service_handler_t
service_get_handler(const char *interface, const char *member)
{
    service_handler_t handler = 0;

    for( size_t i = 0; i < G_N_ELEMENTS(service_method_lut); ++i ) {
        // test member name first because they are shorter and more
        // likely to be unique than interface names
        if( strcmp(service_method_lut[i].sm_member, member) )
            continue;

        if( strcmp(service_method_lut[i].sm_interface, interface) )
            continue;

        handler = service_method_lut[i].sm_handler;
        break;
    }

    return handler;
}

static DBusHandlerResult
service_filter_cb(DBusConnection *con, DBusMessage *msg, gpointer aptr)
{
    (void)aptr;

    DBusHandlerResult  res = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    DBusMessage       *rsp = 0;

    int type = dbus_message_get_type(msg);

    if( type != DBUS_MESSAGE_TYPE_METHOD_CALL )
        goto EXIT;

    const char *interface = dbus_message_get_interface(msg);

    if( !interface )
        goto EXIT;

    const char *member = dbus_message_get_member(msg);

    if( !member )
        goto EXIT;

    service_handler_t handler = service_get_handler(interface, member);

    if( !handler )
        goto EXIT;

    log_emit(LOG_NOTICE, "handle %s.%s()", interface, member);

    if( !(rsp = handler(msg)) )
        rsp = dbus_message_new_error(msg, DBUS_ERROR_FAILED, "internal error");

    stayalive_renew();

EXIT:
    if( rsp ) {
        res = DBUS_HANDLER_RESULT_HANDLED;
        if( !dbus_message_get_no_reply(msg) )
            dbus_connection_send(con, rsp, 0);
        dbus_message_unref(rsp);
    }

    return res;
}

static bool
service_init(void)
{
    bool        res = false;
    DBusBusType bus = DBUS_BUS_STARTER;
    DBusError   err = DBUS_ERROR_INIT;

    if( !(service_con = dbus_bus_get(bus, &err)) ) {
        log_emit(LOG_CRIT, "bus connect failed: %s: %s",
                 err.name, err.message);
        goto EXIT;
    }

    dbus_connection_setup_with_g_main(service_con, NULL);

    if( !dbus_connection_add_filter(service_con, service_filter_cb, 0,0) ) {
        log_emit(LOG_CRIT, "add message filter failed");
        goto EXIT;
    }

    int rc = dbus_bus_request_name(service_con, TESTSRV_SERVICE, 0, &err);

    if( dbus_error_is_set(&err) ) {
        log_emit(LOG_CRIT, "acquire name failed: %s: %s",
                 err.name, err.message);
        goto EXIT;
    }

    if( rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER ) {
        log_emit(LOG_CRIT, "acquire name failed: %s",
                 "not primary owner");
        goto EXIT;
    }

    res = true;

EXIT:
    dbus_error_free(&err);

    return res;
}

static void
service_quit(void)
{
    if( service_con ) {
        dbus_connection_remove_filter(service_con,
                                      service_filter_cb, 0);

        dbus_connection_flush(service_con);

        dbus_connection_unref(service_con),
            service_con = NULL;
    }
}

/* ========================================================================= *
 * MAINLOOP
 * ========================================================================= */

static GMainLoop *mainloop_hnd = 0;

static int mainloop_res = EXIT_SUCCESS;

static void
mainloop_exit(int xc)
{
    if( mainloop_res < xc )
        mainloop_res = xc;

    if( !mainloop_hnd )
        exit(mainloop_res);

    g_main_loop_quit(mainloop_hnd);
}

static int
mainloop_run(void)
{
    mainloop_res = EXIT_SUCCESS;
    mainloop_hnd = g_main_loop_new(0, 0);

    g_main_loop_run(mainloop_hnd);

    g_main_loop_unref(mainloop_hnd),
        mainloop_hnd = 0;

    return mainloop_res;
}

/* ========================================================================= *
 * STAYALIVE
 * ========================================================================= */

static guint stayalive_timer_ms = 5 * 1000;

static guint stayalive_timer_id = 0;

static gboolean
stayalive_timer_cb(gpointer aptr)
{
    if( !stayalive_timer_id )
        goto EXIT;

    stayalive_timer_id = 0;

    log_emit(LOG_NOTICE, "stayalive timeout");

    mainloop_exit(EXIT_SUCCESS);

EXIT:
    return FALSE;
}

static void
stayalive_renew(void)
{
    if( stayalive_timer_id )
        g_source_remove(stayalive_timer_id);

    stayalive_timer_id = g_timeout_add(stayalive_timer_ms,
                                       stayalive_timer_cb, 0);
}

static void
stayalive_quit(void)
{
    if( stayalive_timer_id ) {
        g_source_remove(stayalive_timer_id),
            stayalive_timer_id = 0;
    }
}

/* ========================================================================= *
 * ENTRY_POINT
 * ========================================================================= */

int
main(int ac, char **av)
{
    int xc = EXIT_FAILURE;

    log_emit(LOG_NOTICE, "init");

    if( !service_init() )
        goto EXIT;

    stayalive_renew();

    xc = mainloop_run();

EXIT:
    stayalive_quit();

    service_quit();

    log_emit(LOG_NOTICE, "exit %d", xc);
    return xc;
}
