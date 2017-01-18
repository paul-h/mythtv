TEMPLATE = lib

CONFIG += plugin thread

TARGET = access_myth_plugin

INCLUDEPATH += /usr/include/vlc/plugins

#QMAKE_CFLAGS += `pkg-config --cflags vlc-plugin`
#LIBS += `pkg-config --libs vlc-plugin`

QMAKE_CFLAGS += -D__PLUGIN__ -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE -I/usr/include/vlc/plugins
LIBS += -lvlccore

#DEFINES += MODULE_STRING="myth"

# Input
SOURCES += myth.c

target.path = /usr/lib/vlc/plugins/access
INSTALLS += target