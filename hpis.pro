TEMPLATE = subdirs
SUBDIRS = app camlibs iolibs libgphoto2 libgphoto2_port vendor/qhttp
app.depends = libgphoto2 libgphoto2_port vendor/qhttp
camlibs.depends = libgphoto2
iolibs.depends = libgphoto2_port
libgphoto2.depends = libgphoto2_port
