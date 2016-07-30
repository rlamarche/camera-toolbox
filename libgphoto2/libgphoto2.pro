TEMPLATE = lib
CONFIG = staticlib


SOURCES = \
        ../vendor/libgphoto2/libgphoto2/gphoto2-abilities-list.c \
        ../vendor/libgphoto2/libgphoto2/ahd_bayer.c 		 \
        ../vendor/libgphoto2/libgphoto2/bayer.c                  \
        ../vendor/libgphoto2/libgphoto2/gphoto2-camera.c	 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-context.c	 \
        ../vendor/libgphoto2/libgphoto2/exif.c     		 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-file.c		 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-filesys.c	 \
        ../vendor/libgphoto2/libgphoto2/gamma.c 		 \
        ../vendor/libgphoto2/libgphoto2/jpeg.c 		 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-list.c		 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-result.c	 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-version.c	 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-setting.c	 \
        ../vendor/libgphoto2/libgphoto2/gphoto2-widget.c \

HEADERS = \
        ../vendor/libgphoto2/config.h	\
        ../vendor/libgphoto2/libgphoto2/bayer.h	\
        ../vendor/libgphoto2/libgphoto2/exif.h \
        ../vendor/libgphoto2/libgphoto2/gamma.h \
        ../vendor/libgphoto2/libgphoto2/jpeg.h \

INCLUDEPATH = \
    ../vendor/libgphoto2 \
    ../vendor/libgphoto2/libgphoto2 \
    ../vendor/libgphoto2/libgphoto2_port \
    $$[QT_SYSROOT]/usr/include/libxml2

LIBS += \
        -L../libgphoto2_port -llibgphoto2_port \
        -lltdl -lexif

QMAKE_CFLAGS += -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -fPIC

DEFINES += \
        LOCALEDIR=\\\"locale\\\" \
        CAMLIBS=\\\"camlibs\\\" \
        IOLIBS=\\\"iolibs\\\" \
        _GPHOTO2_INTERNAL_CODE \

