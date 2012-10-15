include ( ../../mythconfig.mak )
include ( ../../settings.pro )
include ( ../../programs-libs.pro )

QT += network sql xml

INCLUDEPATH += $${PREFIX}/include/mythtv
INCLUDEPATH += $${PREFIX}/include/mythtv/libmythui
INCLUDEPATH += $${PREFIX}/include/mythtv/libmythdb

TEMPLATE = lib
CONFIG += plugin thread warn_on debug
TARGET = mythwstation
target.path = $${LIBDIR}/mythtv/plugins
INSTALLS += target

templatefiles.path = $${PREFIX}/share/mythtv/wmr928
templatefiles.files = ../plot_templates/*.dat

INSTALLS += templatefiles

# Input
HEADERS += currentcond.h chartviewer.h wmrsettings.h wmrclient.h

SOURCES += main.cpp currentcond.cpp chartviewer.cpp wmrsettings.cpp wmrclient.cpp

macx {
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
}
