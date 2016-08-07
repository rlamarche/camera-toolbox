#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include "camera.h"

#include <QString>
#include <QJsonObject>

namespace hpis {

class CameraInfo
{
    friend class Camera;
public:
    CameraInfo();

    QString displayName() { return m_displayName; }
    QString manufacturer() { return m_manufacturer; }
    QString cameraModel() { return m_cameraModel; }

    QJsonObject toJsonObject();

private:
    QString m_displayName;
    QString m_manufacturer;
    QString m_cameraModel;
};

}

#endif // CAMERAINFO_H
