#ifndef GPCAMERAPREVIEW_H
#define GPCAMERAPREVIEW_H

#include "../camerapreview.h"

#include <gphoto2/gphoto2-camera.h>

class GPCameraPreview : public CameraPreview
{
public:
    GPCameraPreview(CameraFile* cameraFile);
    GPCameraPreview(const GPCameraPreview& copy);

    ~GPCameraPreview();

    const char* data();
    unsigned long size();
    Format format();

private:
    CameraFile* m_cameraFile;

    const char* m_data;
    unsigned long m_size;
};

#endif // GPCAMERAPREVIEW_H
