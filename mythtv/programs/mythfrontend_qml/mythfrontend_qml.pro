include ( ../../settings.pro )
include ( ../../version.pro )
include ( ../programs-libs.pro )

include(QmlVlc/QmlVlc.pri)

INCLUDEPATH += QmlVlc

QT += qml quick sql xml
CONFIG += c++11

TEMPLATE = app

TARGET = mythfrontend_qml
target.path = $${PREFIX}/bin
INSTALLS = target

qml.path = $${PREFIX}/share/mythtv/qml
qml.files += Themes Models MenuThemes main.qml Util.js

INSTALLS += qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = Themes Models MenuThemes

QMAKE_CLEAN += $(TARGET)

# Input
HEADERS += sqlquerymodel.h databaseutils.h

SOURCES += main.cpp
SOURCES += sqlquerymodel.cpp databaseutils.cpp

