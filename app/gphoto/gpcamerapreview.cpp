#include "gpcamerapreview.h"

GPCameraPreview::GPCameraPreview(CameraFile* cameraFile) : CameraPreview(), m_cameraFile(cameraFile)
{
    gp_file_ref(m_cameraFile);
    gp_file_get_data_and_size(m_cameraFile, &m_data, &m_size);
}

GPCameraPreview::GPCameraPreview(const GPCameraPreview &copy)
{
    m_cameraFile = copy.m_cameraFile;
    m_data = copy.m_data;
    m_size = copy.m_size;

    gp_file_ref(m_cameraFile);
}

GPCameraPreview::~GPCameraPreview()
{
    gp_file_unref(m_cameraFile);
}


const char* GPCameraPreview::data()
{
    return m_data;
}

unsigned long GPCameraPreview::size()
{
    return m_size;
}

CameraPreview::Format GPCameraPreview::format()
{
    return CameraPreview::FormatJPG;
}
