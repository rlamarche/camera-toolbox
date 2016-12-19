TEMPLATE = lib
#CONFIG = staticlib
#CONFIG += compile_libtool


SOURCES = \
        ../../vendor/libgphoto2/camlibs/ptp2/ptp.c \
        ../../vendor/libgphoto2/camlibs/ptp2/library.c ../../vendor/libgphoto2/camlibs/ptp2/usb.c \
        ../../vendor/libgphoto2/camlibs/ptp2/ptpip.c ../../vendor/libgphoto2/camlibs/ptp2/config.c \
        ../../vendor/libgphoto2/camlibs/ptp2/olympus-wrap.c \
        ../../vendor/libgphoto2/camlibs/ptp2/chdk.c \

HEADERS = \
        ../../vendor/libgphoto2/camlibs/ptp2/ptp.h ../../vendor/libgphoto2/camlibs/ptp2/chdk_ptp.h ../../vendor/libgphoto2/camlibs/ptp2/chdk_live_view.h \
        ../../vendor/libgphoto2/camlibs/ptp2/ptp-bugs.h \
        ../../vendor/libgphoto2/camlibs/ptp2/ptp-private.h \
        ../../vendor/libgphoto2/camlibs/ptp2/music-players.h ../../vendor/libgphoto2/camlibs/ptp2/device-flags.h \
        ../../vendor/libgphoto2/camlibs/ptp2/olympus-wrap.h \


INCLUDEPATH = \
    ../../vendor/libgphoto2 \
    ../../vendor/libgphoto2/libgphoto2 \
    ../../vendor/libgphoto2/libgphoto2_port \
    $$[QT_SYSROOT]/usr/include//libxml2

QMAKE_CFLAGS = -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -fPIC

QMAKE_LIBTOOL = libtool --tag=CC
#QMAKE_LFLAGS = -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -module -no-undefined -avoid-version -export-dynamic \
#                -export-symbols $$PWD/../../vendor/libgphoto2/camlibs/camlib.sym


LIBS += \
        -L../../libgphoto2 -llibgphoto2 \
        -L../../libgphoto2_port -llibgphoto2_port \
#        -lltdl -lexif

DEFINES += \
        _GPHOTO2_INTERNAL_CODE \
#        LOCALEDIR=\\\"locale\\\" \
#        CAMLIBS=\\\"camlibs\\\" \
#        IOLIBS=\\\"iolibs\\\" \


target.path = /home/pi/camlibs
INSTALLS += target

