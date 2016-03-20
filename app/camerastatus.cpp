#include "camerastatus.h"

hpis::CameraStatus::CameraStatus()
{

}


QString hpis::CameraStatus::aperture()
{
    return m_aperture;
}

QString hpis::CameraStatus::iso()
{
    return m_iso;
}

bool hpis::CameraStatus::isoAuto()
{
    return m_isoAuto;
}

QString hpis::CameraStatus::shutterSpeed()
{
    return m_shutterSpeed;
}

bool hpis::CameraStatus::exposurePreview()
{
    return m_exposurePreview;
}
