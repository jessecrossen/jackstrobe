#-------------------------------------------------
#
# Project created by QtCreator 2015-12-29T22:36:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jackstrobe
TEMPLATE = app

CONFIG += c++11

LIBS += -L/usr/include/jack/ -ljack

SOURCES += main.cpp\
        widget.cpp \
    jackinput.cpp \
    frequencymap.cpp

HEADERS  += widget.h \
    jackinput.h \
    frequencymap.h

FORMS    += widget.ui
