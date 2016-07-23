#ifndef CAMERASTATUS_H
#define CAMERASTATUS_H

#include <QString>
#include <QCoreApplication>

#include "camera.h"

namespace hpis {


class CameraStatus
{
    friend class Camera;
public:
    CameraStatus();

    Camera::CaptureMode captureMode();
    bool isInLiveView();
    bool isRecording();

    QString aperture();
    QString shutterSpeed();
    QString iso();
    bool isoAuto();
    bool exposurePreview();
    QJsonObject toJsonObject();

private:
    Camera::CaptureMode m_captureMode;
    bool m_isInLiveView;
    bool m_isRecording;

    QString m_aperture;
    QString m_shutterSpeed;
    QString m_iso;
    bool m_isoAuto;
    bool m_exposurePreview;

};

}

Q_DECLARE_METATYPE(hpis::CameraStatus)

#endif // CAMERASTATUS_H
