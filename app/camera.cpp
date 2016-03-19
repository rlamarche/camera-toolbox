#include "camera.h"

hpis::Camera::Camera(QObject *parent) : QObject(parent)
{

}

hpis::CameraStatus hpis::Camera::status()
{
    CameraStatus cs;
    cs.m_aperture = aperture();

    return cs;
}
