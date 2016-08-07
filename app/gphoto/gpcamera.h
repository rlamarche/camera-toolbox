#ifndef GPCAMERA_H
#define GPCAMERA_H

#include "../camera.h"


#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>

#include <QMap>
#include <QList>

namespace hpis {

class GPCamera : public Camera
{
    Q_OBJECT

public:
    enum RecordingMedia {
        RecordingMediaCard = 0,
        RecordingMediaSDRAM,
        RecordingMediaBoth
    };

    enum CaptureTarget {
        CaptureTargetSDRAM = 0,
        CaptureTargetCard
    };

    enum StillCaptureMode {
        StillCaptureModeSingleShot = 0,
        StillCaptureModeBurst,
        StillCaptureModeLowSpeed,
        StillCaptureModeTimer,
        StillCaptureModeMirrorUp,
        StillCaptureModeQuiet
    };


    explicit GPCamera(QString cameraModel, QString cameraPort, QObject *parent = 0);
    ~GPCamera();

    // Infos
    QString displayName();
    QString manufacturer();
    QString cameraModel();

    // Init / Shutdown / Read
    bool init();
    void shutdown();
    bool readCameraSettings();

    // Capture photo
    bool capturePhoto();

    // Capture video
    bool startRecordMovie();
    bool stopRecordMovie();
    bool isRecording();

    // Focus
    bool afDrive();
    bool changeAfArea(int x, int y);

    // Live view
    bool capturePreview(CameraPreview& cameraPreview);
    bool startLiveView();
    bool stopLiveView();
    bool isInLiveView();

    // Capture mode
    virtual bool setCaptureMode(CaptureMode captureMode) = 0;
    CaptureMode captureMode();

    // Recording media
    bool setRecordingMedia(RecordingMedia recordingMedia);
    QString recordingMedia();

    // Capture target
    bool setCaptureTarget(CaptureTarget captureTarget);
    QString captureTarget();

    // Still capture mode
    bool setStillCaptureMode(StillCaptureMode stillCaptureMode);
    QString stillCaptureMode();

    // Exposure preview
    virtual bool setExposurePreview(bool exposurePreview) = 0;
    virtual bool exposurePreview();

    // Aperture
    QStringList apertures();
    QString aperture();
    bool setAperture(QString aperture);
    bool increaseAperture();
    bool decreaseAperture();

    // Shutter speed
    QStringList shutterSpeeds();
    bool setShutterSpeed(QString shutterSpeed);
    QString shutterSpeed();
    bool increaseShutterSpeed();
    bool decreaseShutterSpeed();

    // ISO Auto
    bool isoAuto();
    virtual bool setIsoAuto(bool isoAuto) = 0;

    // ISO
    QStringList isos();
    QString iso();
    bool setIso(QString iso);
    bool increaseIso();
    bool decreaseIso();

    // Exposure mode
    QStringList exposureModes();
    QString exposureMode();
    bool setExposureMode(QString exposureMode);
    bool exposureModePlus();
    bool exposureModeMinus();

    // Live view zoom ratio
    QString lvZoomRatio();
    virtual bool increaseLvZoomRatio() = 0;
    virtual bool decreaseLvZoomRatio() = 0;


    // Program shift value
    bool setProgramShiftValue(int value);
    int programShiftValue();
    int programShiftValueMin();
    int programShiftValueMax();
    int programShiftValueStep();

    // Exposure compensation
    bool setExposureCompensation(QString value);
    QString exposureCompensation();
    bool increaseExposureCompensation();
    bool decreaseExposureCompensation();

protected:
    void reportError(QString error);
    QString errorCodeToString(int errorCode);

    bool lookupWidgets(CameraWidget* widget, QString path);

    int gpExtractWidgetChoices(QString widgetName, QStringList& choice);

    // get widget value
    int gpGetToggleWidgetValue(QString widgetName, int* value);
    int gpGetRadioWidgetValue(QString widgetName, QString& value);
    int gpGetRangeWidgetValue(QString widgetName, float* value);
    int gpGetTextWidgetValue(QString widgetName, QString& value);

    // set widget
    int gpSetToggleWidget(QString widgetName, int toggleValue);
    int gpSetRangeWidget(QString widgetName, float rangeValue);
    int gpSetRadioWidget(QString widgetName, QString radioValue);
    int gpSetTextWidget(QString widgetName, QString textValue);

    int gpGetRangeWidgetInfo(QString widgetName, float* min, float* max, float* step);

    // widget names
    virtual QString manufacturerWidgetName();
    virtual QString cameraModelWidgetName();


    virtual QString viewfinderWidgetName() = 0;

    virtual QString apertureWidgetName();
    virtual QString shutterSpeedWidgetName();
    virtual QString isoWidgetName();

    virtual QString afModeWidgetName();

    virtual QString exposureModeWidgetName() = 0;

    virtual QString afAreaWidgetName();
    virtual QString afAtWidgetName();
    virtual QString afDriveWidgetName();

    virtual QString recordingMediaWidgetName();
    virtual QString captureTargetWidgetName();

    virtual QString stillCaptureModeWidgetName();

    virtual QString exposureCompensationWidgetName();

    virtual QString programShiftValueWidgetName();

    virtual QString exposureIndicatorWidgetName();

    virtual bool gpReadCaptureMode() = 0;

    virtual bool gpReadExposureMode();

    virtual bool gpReadAperture();

    virtual bool gpReadShutterSpeed();

    virtual bool gpReadIso();

    virtual bool gpReadIsoAuto() = 0;

    virtual bool gpReadLvZoomRatio() = 0;

    virtual bool gpReadRecordingMedia();

    virtual bool gpReadCaptureTarget();

    virtual bool gpReadStillCaptureMode();

    virtual bool gpReadExposurePreview() = 0;

    virtual bool gpReadViewFinder();

    virtual bool gpReadProgramShiftValue();

    virtual bool gpReadExposureCompensation();


    int m_cameraNumber;


    // GPhoto context
    ::GPContext*              m_context;
    ::CameraAbilitiesList*    m_abilitiesList;
    ::GPPortInfoList*         m_portInfoList;
    ::Camera*                 m_camera;
    ::CameraWidget*           m_cameraWindow;

    // Camera infos
    CameraAbilities m_cameraAbilities;
    QString m_model;
    QString m_cameraPort;
    QString m_manufacturer;
    QString m_cameraModel;

    // Camera settings
    CaptureMode m_captureMode;

    bool m_isInLiveView;
    bool m_isRecording;

    QStringList m_cameraApertures;
    int m_cameraAperture;

    QStringList m_cameraShutterSpeeds;
    int m_cameraShutterSpeed;

    QStringList m_cameraIsos;
    int m_cameraIso;

    bool m_cameraIsoAuto;

    QStringList m_exposureModes;
    int m_exposureMode;

    QStringList m_lvZoomRatios;
    int m_lvZoomRatio;

    QStringList m_recordingMedias;
    int m_recordingMedia;

    QStringList m_captureTargets;
    int m_captureTarget;

    QStringList m_stillCaptureModes;
    int m_stillCaptureMode;

    bool m_viewfinder;

    bool m_exposurePreview;

    int m_programShiftValue;
    int m_programShiftValueMin;
    int m_programShiftValueMax;
    int m_programShiftValueStep;

    QStringList m_exposureCompensations;
    int m_exposureCompensation;

private:



signals:

public slots:
};

}

#endif // GPCAMERA_H
