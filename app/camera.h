#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include "camerapreview.h"
#include "camerastatus.h"

namespace hpis {

class Camera : public QObject
{
    Q_OBJECT
public:
    enum CaptureMode {
        CaptureModePhoto,
        CaptureModeVideo
    };


    explicit Camera(QObject *parent = 0);
    virtual ~Camera() {};

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual bool readCameraSettings() = 0;

    virtual bool capturePreview(CameraPreview** cameraPreview) = 0;
    virtual bool capturePhoto() = 0;
    virtual bool startRecordMovie() = 0;
    virtual bool stopRecordMovie() = 0;

    virtual bool startLiveView() = 0;
    virtual bool stopLiveview() = 0;

    virtual bool setCaptureMode(CaptureMode captureMode) = 0;
    virtual CaptureMode captureMode() = 0;

    virtual QString aperture() = 0;
    virtual QString shutterSpeed() = 0;
    virtual QString iso() = 0;
    virtual bool isoAuto() = 0;
    virtual bool setIsoAuto(bool isoAuto) = 0;

    virtual QString exposureMode() = 0;
    virtual QString lvZoomRatio() = 0;

    virtual bool changeAfArea(int x, int y) = 0;

    virtual bool increaseAperture() = 0;
    virtual bool decreaseAperture() = 0;

    virtual bool increaseShutterSpeed() = 0;
    virtual bool decreaseShutterSpeed() = 0;

    virtual bool setIso(QString iso) = 0;

    virtual bool increaseIso() = 0;
    virtual bool decreaseIso() = 0;

    virtual bool exposureModePlus() = 0;
    virtual bool exposureModeMinus() = 0;


    virtual bool increaseLvZoomRatio() = 0;
    virtual bool decreaseLvZoomRatio() = 0;

    virtual bool setExposurePreview(bool exposurePreview) = 0;
    virtual bool exposurePreview() = 0;


    CameraStatus status();
signals:

public slots:
};
}

#endif // CAMERA_H
