TEMPLATE = lib
CONFIG = staticlib


SOURCES = \
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-info-list.c	\
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-log.c		\
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-version.c		\
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port.c 			\
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-portability.c	\
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-result.c



HEADERS = \
        ../vendor/libgphoto2/libgphoto2_port/libgphoto2_port/gphoto2-port-info.h \

INCLUDEPATH = \
    ../vendor/libgphoto2/libgphoto2_port

QMAKE_CFLAGS = -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -fPIC

DEFINES += \
        LOCALEDIR=\\\"locale\\\" \
        CAMLIBS=\\\"camlibs\\\" \
        IOLIBS=\\\"iolibs\\\" \
        _GPHOTO2_INTERNAL_CODE \

