TEMPLATE = subdirs

SUBDIRS = \
        nemo-dbus \
        plugin

plugin.depends = nemo-dbus
