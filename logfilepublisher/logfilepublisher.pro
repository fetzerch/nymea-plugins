include(../plugins.pri)

QT += network

TARGET = $$qtLibraryTarget(nymea_devicepluginlogfilepublisher)

SOURCES += \
    devicepluginlogfilepublisher.cpp \
    filesystem.cpp \
    ftpupload.cpp

HEADERS += \
    devicepluginlogfilepublisher.h \
    filesystem.h \
    ftpupload.h