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

private:
    QString m_aperture;
    QString m_shutterSpeed;
    QString m_iso;

};

}

Q_DECLARE_METATYPE(hpis::CameraStatus)

#endif // CAMERASTATUS_H
