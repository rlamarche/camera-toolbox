#include "camerastatus.h"

#include <QJsonObject>

using namespace hpis;

CameraStatus::CameraStatus()
{

}

Camera::CaptureMode CameraStatus::captureMode()
{
    return m_captureMode;
}

bool CameraStatus::isInLiveView()
{
    return m_isInLiveView;
}

bool CameraStatus::isRecording()
{
    return m_isRecording;
}

QString CameraStatus::aperture()
{
    return m_aperture;
}

QString CameraStatus::iso()
{
    return m_iso;
}

bool CameraStatus::isoAuto()
{
    return m_isoAuto;
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

    status["captureMode"] = m_captureMode;
    status["isInLiveView"] = m_isInLiveView;
    status["isRecording"] = m_isRecording;
    status["aperture"] = m_aperture;
    status["shutterSpeed"] = m_shutterSpeed;
    status["iso"] = m_iso;

    return status;
}
