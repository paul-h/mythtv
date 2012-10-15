include ( ../../mythconfig.mak )
include (../../settings.pro )

TEMPLATE = app

CONFIG -= qt

TARGET = mythwmrserver
target.path = $${PREFIX}/bin

INSTALLS = target

LIBS = $$system(mysql_config --libs)

# Input
HEADERS += wmrserver.h

SOURCES += main.cpp wmrserver.cpp
