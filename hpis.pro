TEMPLATE = subdirs
SUBDIRS = app camlibs iolibs libgphoto2 libgphoto2_port
app.depends = libgphoto2 libgphoto2_port
camlibs.depends = libgphoto2
iolibs.depends = libgphoto2_port
libgphoto2.depends = libgphoto2_port
