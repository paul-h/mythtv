include ( ../../mythconfig.mak )
include ( ../../settings.pro )
include ( ../../../mythtv/version.pro )
include ( ../../programs-libs.pro )

DEPENDPATH *= $${INCLUDEPATH}

TEMPLATE = app
CONFIG += thread
TARGET = mythbmcapture

target.path = $${PREFIX}/bin

INSTALLS = target

QMAKE_CLEAN += $(TARGET)

SDK_PATH =/usr/include/DeckLink

INCLUDEPATH += $${SDK_PATH}

CFLAGS +=-Wno-multichar -I $${SDK_PATH} -fno-rtti

# Input
HEADERS += capture.h Config.h
SOURCES += capture.cpp Config.cpp $${SDK_PATH}/DeckLinkAPIDispatch.cpp

LIBS += -lm -ldl -lpthread
