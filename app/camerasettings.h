#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

#include <QString>

#include "camera.h"

namespace hpis {

class CameraSettings
{
public:
    CameraSettings();

    Camera::CaptureMode captureMode();

    QString exposureMode();
    QString aperture();
    QString shutterSpeed();
    QString iso();
    QString focusMode();
    QString focusMetering();

    bool isoAuto();
    bool exposurePreview();

    QJsonObject toJsonObject();

    static CameraSettings fromJsonObject(QJsonObject jsonObject);
private:
    Camera::CaptureMode m_captureMode;
    QString m_exposureMode;
    QString m_aperture;
    QString m_shutterSpeed;
    QString m_iso;
    QString m_focusMode;
    QString m_focusMetering;
    bool m_isoAuto;
    bool m_exposurePreview;
};

}
#endif // CAMERASETTINGS_H
