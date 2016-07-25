#include "camerastatus.h"

#include <QJsonObject>
#include <QJsonArray>

using namespace hpis;

CameraStatus::CameraStatus()
{

}

Camera::CaptureMode CameraStatus::captureMode()
{
    return m_captureMode;
}

QList<QString> CameraStatus::exposureModes()
{
    return m_exposureModes;
}

QString CameraStatus::exposureMode()
{
    return m_exposureMode;
}

bool CameraStatus::isInLiveView()
{
    return m_isInLiveView;
}

bool CameraStatus::isRecording()
{
    return m_isRecording;
}

QList<QString> CameraStatus::apertures()
{
    return m_apertures;
}

QString CameraStatus::aperture()
{
    return m_aperture;
}

QList<QString> CameraStatus::isos()
{
    return m_isos;
}

QString CameraStatus::iso()
{
    return m_iso;
}

bool CameraStatus::isoAuto()
{
    return m_isoAuto;
}

QList<QString> CameraStatus::shutterSpeeds()
{
    return m_shutterSpeeds;
}

QString CameraStatus::shutterSpeed()
{
    return m_shutterSpeed;
}

bool CameraStatus::exposurePreview()
{
    return m_exposurePreview;
}

QJsonObject CameraStatus::toJsonObject()
{
    QJsonObject status;

    //status["exposureModes"] = QJsonArray(m_exposureModes);

    status["exposureMode"] = m_exposureMode;
    status["captureMode"] = m_captureMode;
    status["isInLiveView"] = m_isInLiveView;
    status["isRecording"] = m_isRecording;
    status["aperture"] = m_aperture;
    status["shutterSpeed"] = m_shutterSpeed;
    status["iso"] = m_iso;

    return status;
}
