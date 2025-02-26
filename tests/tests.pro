TEMPLATE = aux

autotest.files = auto/*.qml
autotest.path = /opt/tests/nemo-qml-plugin-dbus-qt5/auto

testdefinition.files = test-definition/tests.xml
testdefinition.path = /opt/tests/nemo-qml-plugin-dbus-qt5/test-definition

OTHER_FILES += \
    auto/*.qml \
    dbustestd/* \
    dbustestd/dbus/*.service \
    test-definition/*.xml \

INSTALLS += autotest testdefinition
