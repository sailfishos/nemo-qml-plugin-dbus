TEMPLATE = subdirs
SUBDIRS += src tests

CONFIG += mer-qdoc-template
MER_QDOC.project = nemo-qml-plugin-dbus
MER_QDOC.config = doc/nemo-qml-plugin-dbus.qdocconf
MER_QDOC.style = offline
MER_QDOC.path = /usr/share/doc/nemo-qml-plugin-dbus

OTHER_FILES += \
    rpm/nemo-qml-plugin-dbus-qt5.spec \
    doc/src/index.qdoc

tests.depends = src
