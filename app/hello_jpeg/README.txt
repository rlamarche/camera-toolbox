This is a "sample" (I wrote enough code that it could be called a small framework now LOL) showing
how to decode JPEG on the raspberry pi using the OpenMAX API.

I've tested it on both small (as in file size JPEGs and large (as in file size and resolution) JPEGs
and it seems to work.  Decoding large JPEGs is harder from a dev point of view
and I spent quite a bit of time getting it to
work correctly.  I had to mainly go by trial and error and reading the official OpenMAX spec to get
this working.

This program will decode to a RGBA buffer and write out a file called output.raw when it is done.
Making it decode in other color formats should be fairly trivial
(just search for OMX_COLOR_Format32bitABGR8888 and change it to whatever you want).

By default, the program will run in benchmark mode and decode the same JPEG over and over again
300 times.  It will then spit out a frames/second result at the end.  For small JPEGs I am getting
as high as 150 FPS, for big ones my numbers dropped down to 15 FPS.

I also have some event logging built-in which may be very handy for troubleshooting your own code
and seeing how and when events fire.  To enable event logging, just uncomment the "#define VERBOSE"
inside OMXComponent.h and it _should_ just decode 1 frame and spit out a bunch of logging.

To resize the output, modify lines 460 and 461 of JPEG.cpp

This sample should be "final" unless I find any glaring defects later.  Feel free to share this
with everyone who may benefit as I feel it is important to have this kind of information well
documented and preserved.  Just make sure you don't make any money off it (unless you're one of the
raspberry pi dudes in which case feel free to bundle this on any official images you want).

I am very happy with most of the code except for the massive JPEG::DoIt method.  I could've split
this up into Init(), Run(), and Shutdown() functions but I am just too tired to keep messing with this
now that it works.

--Matt Ownby
(http://www.daphne-emu.com and http://www.laserdisc-replacement.com)
