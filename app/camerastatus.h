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

    QList<QString> exposureModes();
    QString exposureMode();

    QList<QString> apertures();
    QString aperture();

    QList<QString> shutterSpeeds();
    QString shutterSpeed();

    QList<QString> isos();
    QString iso();

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

    bool m_isoAuto;
    bool m_exposurePreview;


};

}

Q_DECLARE_METATYPE(hpis::CameraStatus)

#endif // CAMERASTATUS_H
