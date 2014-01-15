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
    flashp32.cpp \
    simple_crypt.c \
    bootloader.cpp \
    comport.cpp \
    hex.cpp \
    hexfile.cpp

HEADERS += \
    simple_crypt.h \
    comport.h \
    hex.h \
    bootLoader.h \
    hexfile.h
