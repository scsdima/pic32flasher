#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T21:46:33
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = fpic32_cmd
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    Hex.cpp \
    flashp32.cpp \
    ComPort.cpp \
    BootLoader.cpp \
    simple_crypt.c \
    commthread.cpp

HEADERS += \
    Hex.h \
    ComPort.h \
    BootLoader.h \
    config.h \
    simple_crypt.h \
    commthread.h
