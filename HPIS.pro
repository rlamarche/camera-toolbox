#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T17:40:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hpis
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    +=

LIBS     += -lgphoto2_port -lgphoto2

target.path = /home/pi
INSTALLS += target
