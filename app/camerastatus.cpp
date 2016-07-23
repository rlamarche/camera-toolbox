#include "camerastatus.h"

using namespace hpis;

CameraStatus::CameraStatus()
{

}

Camera::CaptureMode CameraStatus::captureMode()
{
    return m_captureMode;
}

bool CameraStatus::isInLiveView()
{
    return m_isInLiveView;
}

bool CameraStatus::isRecording()
{
    return m_isRecording;
}

QString CameraStatus::aperture()
{
    return m_aperture;
}

QString CameraStatus::iso()
{
    return m_iso;
}

bool CameraStatus::isoAuto()
{
    return m_isoAuto;
}

QString CameraStatus::shutterSpeed()
{
    return m_shutterSpeed;
}

bool CameraStatus::exposurePreview()
{
    return m_exposurePreview;
}
