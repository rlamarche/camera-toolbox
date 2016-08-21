#ifndef CAMERASTATUS_H
#define CAMERASTATUS_H

#include <QString>

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

    QList<QString> exposureModes();
    QString exposureMode();

    QList<QString> apertures();
    QString aperture();

    QList<QString> shutterSpeeds();
    QString shutterSpeed();

    QList<QString> isos();
    QString iso();

    QList<QString> focusModes();
    QString focusMode();

    QList<QString> focusMeterings();
    QString focusMetering();

    bool isoAuto();
    bool exposurePreview();


    QJsonObject toJsonObject();

private:
    Camera::CaptureMode m_captureMode;
    bool m_isInLiveView;
    bool m_isRecording;

    QList<QString> m_exposureModes;
    QString m_exposureMode;

    QList<QString> m_apertures;
    QString m_aperture;

    QList<QString> m_shutterSpeeds;
    QString m_shutterSpeed;

    QList<QString> m_isos;
    QString m_iso;

    QList<QString> m_focusModes;
    QString m_focusMode;

    QList<QString> m_focusMeterings;
    QString m_focusMetering;

    bool m_isoAuto;
    bool m_exposurePreview;


};

}

Q_DECLARE_METATYPE(hpis::CameraStatus)

#endif // CAMERASTATUS_H
