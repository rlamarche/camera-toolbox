#include "camerasettings.h"

#include <QJsonObject>
#include <QJsonArray>

using namespace hpis;


CameraSettings::CameraSettings()
{

}


Camera::CaptureMode CameraSettings::captureMode()
{
    return m_captureMode;
}


QString CameraSettings::exposureMode()
{
    return m_exposureMode;
}


QString CameraSettings::aperture()
{
    return m_aperture;
}

QString CameraSettings::iso()
{
    return m_iso;
}

QString CameraSettings::focusMode()
{
    return m_focusMode;
}

QString CameraSettings::focusMetering()
{
    return m_focusMetering;
}

bool CameraSettings::isoAuto()
{
    return m_isoAuto;
}

QString CameraSettings::shutterSpeed()
{
    return m_shutterSpeed;
}

bool CameraSettings::exposurePreview()
{
    return m_exposurePreview;
}

QJsonObject CameraSettings::toJsonObject()
{
    QJsonObject status;

    status["exposureMode"] = m_exposureMode;
    status["captureMode"] = m_captureMode;
    status["aperture"] = m_aperture;
    status["shutterSpeed"] = m_shutterSpeed;
    status["iso"] = m_iso;
    status["focusMode"] = m_focusMode;
    status["focusMetering"] = m_focusMetering;

    return status;
}

CameraSettings CameraSettings::fromJsonObject(QJsonObject jsonObject)
{
    CameraSettings cameraSettings;

    QJsonValueRef exposureModeRef = jsonObject["exposureMode"];
    if (exposureModeRef.isString())
    {
        cameraSettings.m_exposureMode = exposureModeRef.toString();
    }

    QJsonValueRef apertureRef = jsonObject["aperture"];
    if (apertureRef.isString())
    {
        cameraSettings.m_aperture = apertureRef.toString();
    }

    QJsonValueRef shutterSpeedRef = jsonObject["shutterSpeed"];
    if (shutterSpeedRef.isString())
    {
        cameraSettings.m_shutterSpeed = shutterSpeedRef.toString();
    }

    QJsonValueRef isoRef = jsonObject["iso"];
    if (isoRef.isString())
    {
        cameraSettings.m_iso = isoRef.toString();
    }

    QJsonValueRef focusModeRef = jsonObject["focusMode"];
    if (focusModeRef.isString())
    {
        cameraSettings.m_focusMode = focusModeRef.toString();
    }

    QJsonValueRef focusMeteringRef = jsonObject["focusMetering"];
    if (focusMeteringRef.isString())
    {
        cameraSettings.m_focusMetering = focusMeteringRef.toString();
    }

    return cameraSettings;
}
