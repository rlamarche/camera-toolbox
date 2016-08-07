#include "camerainfo.h"

using namespace hpis;

CameraInfo::CameraInfo()
{

}


QJsonObject CameraInfo::toJsonObject()
{
    QJsonObject info;

    info["displayName"] = m_displayName;
    info["manufacturer"] = m_manufacturer;
    info["cameraModel"] = m_cameraModel;

    return info;
}
