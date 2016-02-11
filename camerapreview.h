#ifndef CAMERAPREVIEW_H
#define CAMERAPREVIEW_H


class CameraPreview
{
public:
    enum Format {
        FormatJPG
    };

    CameraPreview();
    virtual ~CameraPreview() {}

    virtual const char* data() = 0;
    virtual unsigned long size() = 0;
    virtual Format format() = 0;
};

#endif // CAMERAPREVIEW_H
