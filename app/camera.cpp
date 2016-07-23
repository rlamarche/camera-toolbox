#include "camera.h"
#include "camerastatus.h"

using namespace hpis;

Camera::Camera(QObject *parent) : QObject(parent)
{

}

CameraStatus hpis::Camera::status()
{
    CameraStatus cs;
    cs.m_captureMode = captureMode();
    cs.m_isInLiveView = isInLiveView();
    cs.m_isRecording = isRecording();
    cs.m_aperture = aperture();
    cs.m_shutterSpeed = shutterSpeed();
    cs.m_iso = iso();
    cs.m_isoAuto = isoAuto();
    cs.m_exposurePreview = exposurePreview();


    return cs;
}
