#-------------------------------------------------
#
# Project created by QtCreator 2017-12-29T10:37:55
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Dispatch
CONFIG   += console
CONFIG   += C++11
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += /usr/local/lib/libopencv_core.so.3.3    \
        /usr/local/lib/libopencv_highgui.so.3.3  \
        /usr/local/lib/libopencv_imgcodecs.so.3.3 \
        /usr/local/lib/libopencv_imgproc.so.3.3

SOURCES += main.cpp \
    map.cpp \
    task.cpp \
    agent.cpp \
    AStar.cpp \
    manager.cpp \
    process.cpp \
    taskgroup.cpp \
    path.cpp \
    servicepoint.cpp \
    acrosspoint.cpp \
    waypoint.cpp \
    combpath.cpp \
    dodgepos.cpp \
    dodgetaskgroup.cpp \
    realtaskgroup.cpp

HEADERS += \
    map.h \
    task.h \
    agent.h \
    AStar.hpp \
    manager.h \
    process.h \
    taskgroup.h \
    path.h \
    servicepoint.h \
    acrosspoint.h \
    waypoint.h \
    combpath.h \
    dodgepos.h \
    dodgetaskgroup.h \
    realtaskgroup.h
