#-------------------------------------------------
#
# Project created by QtCreator 2016-01-30T17:40:04
#
#-------------------------------------------------

QT       += core gui network #declarative

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hpis
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    camerathread.cpp \
    decoderthread.cpp \
    camera.cpp \
    camerapreview.cpp \
    gphoto/gpcamerapreview.cpp \
    gphoto/gpcamera.cpp \
    camerastatus.cpp \
#    histogram/imageanalyzer.cpp \
#    histogram/histogram.cpp
    cameraserver.cpp




HEADERS  += mainwindow.h \
    camerathread.h \
    decoderthread.h \
    camera.h \
    camerapreview.h \
    gphoto/gpcamerapreview.h \
    gphoto/gpcamera.h \
    camerastatus.h \
#    histogram/imageanalyzer.h \
#    histogram/histogram.h
    cameraserver.h


INCLUDEPATH += ../vendor/libgphoto2 ../vendor/libgphoto2/libgphoto2_port ../vendor/qhttp/src
#LIBS     += -lgphoto2_port -lgphoto2
LIBS += \
        -L../libgphoto2 -llibgphoto2 \
        -L../libgphoto2_port -llibgphoto2_port \
        -L../vendor/qhttp/xbin -lqhttp \
        -lltdl -lexif


DEFINES += _GPHOTO2_INTERNAL_CODE
DEFINES += USE_LIBTURBOJPEG

defined(USE_LIBJPEG) {
LIBS += -ljpeg
}

#defined(USE_LIBTURBOJPEG) {
LIBS += -lturbojpeg
#}


#USE_RPI=true


defined(USE_RPI) {

    #DEFINES += USE_OPENGL USE_EGL IS_RPI USE_OPENMAX USE_VCHIQ_ARM VERBOSE
    #DEFINES += VERBOSE

    DEFINES += USE_RPI
    DEFINES += HAVE_LIBOPENMAX=2 OMX OMX_SKIP64BIT USE_EXTERNAL_OMX HAVE_LIBBCM_HOST USE_EXTERNAL_LIBBCM_HOST USE_VCHIQ_ARM

    SOURCES += \
        omx/ilclient.c \
        omx/ilcore.c \
        hello_jpeg_v2/Event.cpp \
        hello_jpeg_v2/JPEG.cpp \
        hello_jpeg_v2/Locker.cpp \
        hello_jpeg_v2/Logger.cpp \
        hello_jpeg_v2/OMXComponent.cpp \
        hello_jpeg_v2/OMXCore.cpp \
        gpu/omximagedecoder.cpp


    HEADERS  += \
        omx/jpeg.h \
        omx/ilclient.h \
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


    LIBS += -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt

}

target.path = /home/pi
INSTALLS += target

