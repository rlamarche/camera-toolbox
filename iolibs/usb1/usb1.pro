TEMPLATE = lib
#CONFIG = staticlib sharedlib
#CONFIG += compile_libtool


SOURCES = \
        ../../vendor/libgphoto2/libgphoto2_port/libusb1/libusb1.c \


HEADERS = \

INCLUDEPATH = \
    ../../vendor/libgphoto2/libgphoto2_port \
    $$[QT_SYSROOT]//usr/include/libusb-1.0

QMAKE_CFLAGS = -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -fPIC \
#        -module -no-undefined -avoid-version \
#        -export-dynamic \
#        -export-symbols ../../vendor/libgphoto2/libgphoto2_port/iolib.sym

QMAKE_LIBTOOL = libtool --tag=CC
#QMAKE_LFLAGS = -g -O2 -Wall -Wmissing-declarations -Wmissing-prototypes -module -no-undefined -avoid-version -export-dynamic \
#                -export-symbols $$PWD/../../vendor/libgphoto2/libgphoto2_port/iolib.sym

LIBS += -lusb-1.0 \
        -L../../libgphoto2_port -llibgphoto2_port \

DEFINES += \
#        LOCALEDIR=\\\"locale\\\" \
#        CAMLIBS=\\\"camlibs\\\" \
#        IOLIBS=\\\"iolibs\\\" \
        _GPHOTO2_INTERNAL_CODE \

target.path = /home/pi/iolibs
INSTALLS += target
