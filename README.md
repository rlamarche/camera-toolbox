# Camera Toolbox

## Introduction

Camera Toolbox is a multi-plaform, Qt-based, toolbox for controlling camera remotely through an API, a keyboard, a HTML5 web application, a mobile application, a REST API, and so on.

The main goal of the project is to be installed on an embedded device like a Raspberry PI, but any device supported by Qt5 would be appropriate.

The goal is to support any camera model from any brand through a unified API, so all developed tools would be compatible with any of these cameras.

## History

I started this project after a call with Pascal (from Azuru Diving), the creator of the underwater camera housing called "HPIS".
He told me about his project to construct a **universal** underwater camera housing based and was looking for developers to help him and Alexandre (the main software developer of the HPIS) to integrate cameras like Nikon or Canon DSLR.

We conclude that almost all software development around the HPIS will be open-source, and after looking over the internet I decided that this tool would not be dedicated to the HPIS but for any camera owner.

## Status & Roadmap

Currently, the project is tested only with a Nikon D800 through a connector named "libgphoto2". This is an awesome library which support a lot of cameras, so my tool would support a lot too. I think that a lot of people coming here already know it.
For those who don't, here the homepage : http://gphoto.sourceforge.net/

Of course, depending on the model or the brand, some specific development will be made to support other supported cameras.

Here some features in the pipe :

* Support maximum number of cameras supported by libgphoto, beginning with Nikon & Canon DSLR
* Support Sony Cameras through the wifi remote API (a SDK is provided)
* Define & implement a full REST API with Swagger
* Develop an embedded web application with ReactJS using the REST API to control the camera & display liveview
* Add more and more features

## Features

The currently implemented features are :

* Display fullscreen liveview on HDMI output of the Raspberry PI
* Control all basic camera parameters : Aperture, Shutter speed, ISO sensitivity, ISO Auto, Focus point, Exposure mode (PASM or more depending on the camera), over/under exposition
* Take a photo or record a movie (while recording a movie, liveview & some camera parameters are still updatable depending on the camera model)
* Nikon DSLR : zoom on the liveview (exactly same as zoom +/- to fine tune focus for example)
* Control all these parameters with a keyboard and a mouse


## Keyboard & mouse control

When clicking on the liveview, the camera autofocus where the user clicked.

Note : currently the sensor resolution of D800 is used for determining where to zoom, TODO : get the sensor resolution from upcoming liveview images. I need to patch libgphoto to do it because these informations are in the liveview response of the camera and libgphoto2 just returns the jpeg part for more compatibility.

Keys :

* Escape : Exit
* KP Enter : toggle liveview
* F : autofocus drive
* C : Switch to photo mode (liveview only)
* V : Switch to video mode (liveview only)
* A/D : decrease/increase aperture
* Shift + A/D : decrease/increase Program shift value (under/over expose in mode P)
* S/W : decrease/increase shutter speed
* Q/E : decrease/increase ISO
* Shift + Q/E : under/over expose (only in mode A or S)
* R/T : cycle through camera exposure modes (M, P, A, S)
* I : Enable auto ISO
* Shift + I : Disable auto ISO
* P : Enable exposure preview (liveview mode only, not available in all modes)
* Shift + P : Disable exposure previes 
* KP +/- : Zoom in / Zoom out focus point or auto-detected faces (liveview only)
* Space : Start / Stop movie recording (liveview only)
* Return : Capture a photo (in liveview or not in liveview)

## Setup

Use a provided image (TODO)

Or :

Follow install guides here : https://wiki.qt.io/RaspberryPi2EGLFS using Qt 5.7.0 branch.
Use a Raspbian Jessie Lite : https://www.raspberrypi.org/downloads/raspbian/

**Note** : apply the following patch to QtBase https://github.com/rlamarche/qtbase/commit/81eda315d1720215d99a04de8ad95893c231ef20

Make sure to have installed the following packages :

sudo apt-get install libudev-dev libinput-dev libts-dev libxcb-xinerama0-dev htop
sudo apt-get install libltdl-dev
sudo apt-get install libexif-dev
sudo apt-get install libturbojpeg1-dev
sudo apt-get install libusb-dev
sudo apt-get install libusb-1.0-0-dev
sudo apt-get install gdb-multiarch
sudo apt-get install xkb-data console-data
sudo apt-get install gphoto2


## Compile

In this repo, run : 

```
#!bash
git submodule init
git submodule update

cd vendor/libgphoto2
autoreconf --install --symlink
CFLAGS="--sysroot /path/to/sysroot" PATH="$PATH:/path/to/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/" ./configure --host=arm-linux-gnueabihf
cd ../../

cd vendor/qhttp
./update-dependencies.sh
cd ../../

<path to qt5 raspberry pi qmake>/qmake
make
```

## Run

Connect a screen to your Raspberry PI

```
#!bash
cd /home/pi
QT_QPA_EGLFS_FORCE888=1 ./hpis
```