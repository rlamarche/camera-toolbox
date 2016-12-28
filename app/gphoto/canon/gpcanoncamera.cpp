#include "gpcanoncamera.h"

#include <QSet>
#include <QDebug>

using namespace hpis;

GPCanonCamera::GPCanonCamera(QString cameraModel, QString cameraPort, QObject *parent) : GPCamera(cameraModel, cameraPort, parent)
{

}


QSet<GPCamera::CameraCapability> GPCanonCamera::capabilities()
{
    // TODO
    QSet<GPCamera::CameraCapability> capabilities;
    QString exposureMode = this->exposureMode();

    if (exposureMode == "M")
    {
        capabilities.insert(GPCamera::ChangeAperture);
        capabilities.insert(GPCamera::ChangeShutterSpeed);
        capabilities.insert(GPCamera::ChangeAperture);
        capabilities.insert(GPCamera::ChangeIso);
    }

    return capabilities;
}


// Init / Shutdown / Read settings

bool GPCanonCamera::init()
{
    bool success = GPCamera::init();

    if (success)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GPCanonCamera::shutdown()
{
    GPCamera::shutdown();
}

bool GPCanonCamera::readCameraSettings()
{
    gpReadExposureMode();
    gpReadAperture();
    gpReadShutterSpeed();
    gpReadIso();
    gpReadExposureCompensation();
    m_focusMode = gpReadRadioWidget(focusModeWidgetName(), m_focusModes);
    m_captureMode = CaptureModePhoto;

    return true;
}



// -------------------------- Capture preview




// Capture preview
bool GPCanonCamera::capturePreview(CameraPreview& cameraPreview)
{
    return GPCamera::capturePreview(cameraPreview);
}


/////////////////////////////////// Camera live view

bool GPCanonCamera::startLiveView()
{
    // TODO how to implement on Canon Rebel XT ?
    m_isInLiveView = true;
    return true;
}

bool GPCanonCamera::stopLiveView()
{
    // TODO how to implement on Canon Rebel XT ?
    m_isInLiveView = false;
    return true;
}


// -------------------------- Custom read methods


// -------------------------- Get / Set

// Capture mode


// Custom widget names

QString GPCanonCamera::apertureWidgetName()
{
    return "aperture";
}

QString GPCanonCamera::shutterSpeedWidgetName()
{
    return "shutterspeed";
}

QString GPCanonCamera::isoWidgetName()
{
    return "iso";
}

QString GPCanonCamera::exposureModeWidgetName()
{
    return "shootingmode";
}


QString GPCanonCamera::focusModeWidgetName()
{
    return "focusmode";
}

QString GPCanonCamera::exposureCompensationWidgetName()
{
    return "exposurecompensation";
}

