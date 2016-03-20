#include "camera.h"

hpis::Camera::Camera(QObject *parent) : QObject(parent)
{

}

hpis::CameraStatus hpis::Camera::status()
{
    CameraStatus cs;
    cs.m_aperture = aperture();
    cs.m_shutterSpeed = shutterSpeed();
    cs.m_iso = iso();
    cs.m_isoAuto = isoAuto();
    cs.m_exposurePreview = exposurePreview();


    return cs;
}
