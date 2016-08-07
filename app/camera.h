#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include "camerapreview.h"
//#include "camerastatus.h"

namespace hpis {

class CameraStatus;
class CameraInfo;

class Camera : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString exposureMode READ exposureMode WRITE setExposureMode)
    Q_PROPERTY(QString aperture READ aperture WRITE setAperture)
    Q_PROPERTY(QString shutterSpeed READ shutterSpeed WRITE setShutterSpeed)
    Q_PROPERTY(QString iso READ iso WRITE setIso)
    Q_PROPERTY(bool isoAuto READ isoAuto WRITE setIsoAuto)
public:
    enum CameraCapability {
        LiveView,
        CapturePhoto,
        RecordMovie,
        AfDrive,
        ChangeExposureMode,
        ChangeAfArea,
        ChangeAperture,
        ChangeShutterSpeed,
        ChangeIso,
        IsoAuto,
        ChangeExposurePreview,
        Zoom,
        ChangeExposureCompensation,
        ChangeProgramShiftValue
    };

    enum CaptureMode {
        CaptureModePhoto,
        CaptureModeVideo
    };

    explicit Camera(QObject *parent = 0);
    virtual ~Camera() {};

    // Info
    virtual QString displayName() = 0;
    virtual QSet<CameraCapability> capabilities() = 0;
    virtual QString manufacturer() = 0;
    virtual QString cameraModel() = 0;

    // Init / Shutdown / Read
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool readCameraSettings() = 0;

    // Capture photo
    virtual bool capturePhoto() = 0;

    // Capture video
    virtual bool startRecordMovie() = 0;
    virtual bool stopRecordMovie() = 0;
    virtual bool isRecording() = 0;

    // Focus
    virtual bool afDrive() = 0;
    virtual bool changeAfArea(int x, int y) = 0;

    // Live view
    virtual bool capturePreview(CameraPreview& cameraPreview) = 0;
    virtual bool startLiveView() = 0;
    virtual bool stopLiveView() = 0;
    virtual bool isInLiveView() = 0;

    // Capture mode
    virtual bool setCaptureMode(CaptureMode captureMode) = 0;
    virtual CaptureMode captureMode() = 0;


    // Aperture
    virtual QStringList apertures() = 0;
    virtual bool setAperture(QString aperture) = 0;
    virtual QString aperture() = 0;
    virtual bool increaseAperture() = 0;
    virtual bool decreaseAperture() = 0;

    // Shutter speed
    virtual QStringList shutterSpeeds() = 0;
    virtual QString shutterSpeed() = 0;
    virtual bool setShutterSpeed(QString shutterSpeed) = 0;
    virtual bool increaseShutterSpeed() = 0;
    virtual bool decreaseShutterSpeed() = 0;

    // ISO Auto
    virtual bool isoAuto() = 0;
    virtual bool setIsoAuto(bool isoAuto) = 0;

    // ISO
    virtual QStringList isos() = 0;
    virtual QString iso() = 0;
    virtual bool setIso(QString iso) = 0;
    virtual bool increaseIso() = 0;
    virtual bool decreaseIso() = 0;

    // Exposure mode
    virtual QStringList exposureModes() = 0;
    virtual QString exposureMode() = 0;
    virtual bool setExposureMode(QString exposureMode) = 0;
    virtual bool exposureModePlus() = 0;
    virtual bool exposureModeMinus() = 0;

    // Live view zoom ratio
    virtual QString lvZoomRatio() = 0;
    virtual bool increaseLvZoomRatio() = 0;
    virtual bool decreaseLvZoomRatio() = 0;

    // Exposure preview
    virtual bool setExposurePreview(bool exposurePreview) = 0;
    virtual bool exposurePreview() = 0;

    // Program shift value
    virtual bool setProgramShiftValue(int value) = 0;
    virtual int programShiftValue() = 0;
    virtual int programShiftValueMax() = 0;
    virtual int programShiftValueMin() = 0;
    virtual int programShiftValueStep() = 0;

    // Exposure compensation
    virtual bool setExposureCompensation(QString value) = 0;
    virtual QString exposureCompensation() = 0;
    virtual bool increaseExposureCompensation() = 0;
    virtual bool decreaseExposureCompensation() = 0;

    CameraStatus status();
    CameraInfo info();
signals:

public slots:
};
}

#endif // CAMERA_H
