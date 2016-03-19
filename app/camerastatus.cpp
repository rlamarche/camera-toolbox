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

QString hpis::CameraStatus::shutterSpeed()
{
    return m_shutterSpeed;
}
