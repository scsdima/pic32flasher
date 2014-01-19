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
SOURCE_DIR = $$PWD

TEMPLATE = app

DESTDIR = $$SOURCE_DIR/bin

SOURCES += \
    flashp32.cpp \
    bootloader.cpp \
    comport.cpp \
    hex.cpp \
    hexfile.cpp \
    simple_crypt.cpp

HEADERS += \
    simple_crypt.h \
    comport.h \
    hex.h \
    bootLoader.h \
    hexfile.h
