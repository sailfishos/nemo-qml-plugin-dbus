TEMPLATE = subdirs
SUBDIRS += src tests doc

OTHER_FILES += \
    rpm/nemo-qml-plugin-dbus-qt5.spec

tests.depends = src
