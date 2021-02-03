QT += core qml quick quickcontrols2

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++14
CONFIG += console

RESOURCES += qml.qrc

SOURCES += glWindow.cpp main.cpp glhelper.cpp \
    meshobject.cpp
HEADERS += glWindow.h glhelper.h \
    meshobject.h

# GEL
SOURCES += $$files(GEL/CGLA/*.cpp, true)
SOURCES += $$files(GEL/HMesh/*.cpp, true)
HEADERS += $$files(GEL/CGLA/*.h, true)
HEADERS += $$files(GEL/CGLAUtil/*.h, true)
HEADERS += $$files(GEL/HMesh/*.h, true)
SOURCES += $$files(GEL/Geometry/*.cpp, true)
HEADERS += $$files(GEL/Geometry/*.h, true)\
SOURCES += $$files(GEL/Util/*.cpp, true)
HEADERS += $$files(GEL/Util/*.h, true)
SOURCES += GEL/Geometry/jrs_triangle.c
