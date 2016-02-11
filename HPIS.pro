#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T17:40:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hpis
TEMPLATE = app

#DEFINES += USE_OPENGL USE_EGL IS_RPI USE_OPENMAX USE_VCHIQ_ARM VERBOSE
#DEFINES += VERBOSE

DEFINES += HAVE_LIBOPENMAX=2 OMX OMX_SKIP64BIT USE_EXTERNAL_OMX HAVE_LIBBCM_HOST USE_EXTERNAL_LIBBCM_HOST USE_VCHIQ_ARM

SOURCES += main.cpp\
        mainwindow.cpp \
    camerathread.cpp \
    decoderthread.cpp \
    camera.cpp \
    camerapreview.cpp \
    gphoto/gpcamerapreview.cpp \
    gphoto/gpcamera.cpp \
    omx/ilclient.c \
    omx/ilcore.c \
#    omx/jpeg.c \
#    hello_jpeg/Event.cpp \
#    hello_jpeg/JPEG.cpp \
#    hello_jpeg/Locker.cpp \
#    hello_jpeg/Logger.cpp \
#    hello_jpeg/OMXComponent.cpp \
#    hello_jpeg/OMXCore.cpp \
    hello_jpeg_v2/Event.cpp \
    hello_jpeg_v2/JPEG.cpp \
    hello_jpeg_v2/Locker.cpp \
    hello_jpeg_v2/Logger.cpp \
    hello_jpeg_v2/OMXComponent.cpp \
    hello_jpeg_v2/OMXCore.cpp \
    gpu/omximagedecoder.cpp


HEADERS  += mainwindow.h \
    camerathread.h \
    decoderthread.h \
    camera.h \
    camerapreview.h \
    gphoto/gpcamerapreview.h \
    gphoto/gpcamera.h \
    omx/jpeg.h \
    omx/ilclient.h \
#    hello_jpeg/Event.h \
#    hello_jpeg/IEvent.h \
#    hello_jpeg/ILocker.h \
#    hello_jpeg/ILogger.h \
#    hello_jpeg/JPEG.h \
#    hello_jpeg/Locker.h \
#    hello_jpeg/Logger.h \
#    hello_jpeg/OMXComponent.h \
#    hello_jpeg/OMXCore.h \
    hello_jpeg_v2/Event.h \
    hello_jpeg_v2/IEvent.h \
    hello_jpeg_v2/ILocker.h \
    hello_jpeg_v2/ILogger.h \
    hello_jpeg_v2/JPEG.h \
    hello_jpeg_v2/Locker.h \
    hello_jpeg_v2/Logger.h \
    hello_jpeg_v2/MyDeleter.h \
    hello_jpeg_v2/OMXComponent.h \
    hello_jpeg_v2/OMXCore.h \
    gpu/omximagedecoder.h


FORMS    +=

LIBS     += -lgphoto2_port -lgphoto2

LIBS += -ljpeg \
        -lturbojpeg

LIBS += -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt

target.path = /home/pi
INSTALLS += target

CONFIG += staticlib
