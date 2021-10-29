TEMPLATE = aux

CONFIG += sailfish-qdoc-template

SAILFISH_QDOC.project = nemo-qml-plugin-dbus
SAILFISH_QDOC.config = nemo-qml-plugin-dbus.qdocconf
SAILFISH_QDOC.style = offline
SAILFISH_QDOC.path = /usr/share/doc/nemo-qml-plugin-dbus

OTHER_FILES += \
    src/index.qdoc
