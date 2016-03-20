#ifndef CAMERASTATUS_H
#define CAMERASTATUS_H

#include <QString>
#include <QCoreApplication>

namespace hpis {

class Camera;

class CameraStatus
{
    friend class Camera;
public:
    CameraStatus();

    QString aperture();
    QString shutterSpeed();
    QString iso();
    bool isoAuto();
    bool exposurePreview();

private:
    QString m_aperture;
    QString m_shutterSpeed;
    QString m_iso;
    bool m_isoAuto;
    bool m_exposurePreview;

};

}

Q_DECLARE_METATYPE(hpis::CameraStatus)

#endif // CAMERASTATUS_H
