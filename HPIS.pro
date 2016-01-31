#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T17:40:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hpis
TEMPLATE = app

DEFINES += USE_OPENGL USE_EGL IS_RPI USE_OPENMAX

SOURCES += main.cpp\
        mainwindow.cpp \
        liveviewthread.cpp \
        liveviewdecoderthread.cpp


HEADERS  += mainwindow.h \
    liveviewthread.h \
    liveviewdecoderthread.h


FORMS    +=

LIBS     += -lgphoto2_port -lgphoto2

LIBS += -ljpeg \
        -lturbojpeg

target.path = /home/pi
INSTALLS += target
