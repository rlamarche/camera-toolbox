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
    bool capturePreview(CameraPreview** cameraPreview);
    bool startLiveView();
    bool stopLiveView();
    bool isInLiveView();

    // Capture mode
    bool setCaptureMode(CaptureMode captureMode);
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
    bool setExposurePreview(bool exposurePreview);
    bool exposurePreview();

    // Aperture
    QList<QString> apertures();
    QString aperture();
    bool setAperture(QString aperture);
    bool increaseAperture();
    bool decreaseAperture();

    // Shutter speed
    QList<QString> shutterSpeeds();
    bool setShutterSpeed(QString shutterSpeed);
    QString shutterSpeed();
    bool increaseShutterSpeed();
    bool decreaseShutterSpeed();

    // ISO Auto
    bool isoAuto();
    bool setIsoAuto(bool isoAuto);

    // ISO
    QList<QString> isos();
    QString iso();
    bool setIso(QString iso);
    bool increaseIso();
    bool decreaseIso();

    // Exposure mode
    QList<QString> exposureModes();
    QString exposureMode();
    bool setExposureMode(QString exposureMode);
    bool exposureModePlus();
    bool exposureModeMinus();

    // Live view zoom ratio
    QString lvZoomRatio();
    bool increaseLvZoomRatio();
    bool decreaseLvZoomRatio();


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

    bool extractWidgetChoices(QString widgetName, QList<QString>& choice);

    // get widget value
    int gpGetToggleWidgetValue(QString widgetName, int* value);
    int gpGetRadioWidgetValue(QString widgetName, QString& value);
    int gpGetRangeWidgetValue(QString widgetName, float* value);

    // set widget
    int gpSetToggleWidget(QString widgetName, int toggleValue);
    int gpSetRangeWidget(QString widgetName, float rangeValue);
    int gpSetRadioWidget(QString widgetName, QString radioValue);
    int gpSetTextWidget(QString widgetName, QString textValue);

    int gpGetRangeWidgetInfo(QString widgetName, float* min, float* max, float* step);

    // widget names
    virtual QString viewfinderWidgetName();

    virtual QString captureModeWidgetName();

    virtual QString apertureWidgetName();
    virtual QString shutterSpeedWidgetName();
    virtual QString isoWidgetName();
    virtual QString isoAutoWidgetName();

    virtual QString liveviewSelectorWidgetName();
    virtual QString afModeWidgetName();
    virtual QString lvZoomRatioWidgetName();

    virtual QString exposureModeWidgetName();

    virtual QString afAreaWidgetName();
    virtual QString afAtWidgetName();
    virtual QString afDriveWidgetName();

    virtual QString recordingMediaWidgetName();
    virtual QString captureTargetWidgetName();

    virtual QString stillCaptureModeWidgetName();

    virtual QString exposurePreviewWidgetName();

    virtual QString exposureCompensationWidgetName();

    virtual QString programShiftValueWidgetName();

    virtual QString exposureIndicatorWidgetName();
private:

    int m_cameraNumber;


    // GPhoto context
    ::GPContext*              m_context;
    ::CameraAbilitiesList*    m_abilitiesList;
    ::GPPortInfoList*         m_portInfoList;
    ::Camera*                 m_camera;
    ::CameraWidget*           m_cameraWindow;

    // Camera infos
    CameraAbilities m_cameraAbilities;
    QString m_cameraModel;
    QString m_cameraPort;

    // Camera settings
    CaptureMode m_captureMode;

    bool m_isInLiveView;
    bool m_isRecording;

    QList<QString> m_cameraApertures;
    int m_cameraAperture;

    QList<QString> m_cameraShutterSpeeds;
    int m_cameraShutterSpeed;

    QList<QString> m_cameraIsos;
    int m_cameraIso;

    bool m_cameraIsoAuto;

    QList<QString> m_exposureModes;
    int m_exposureMode;

    QList<QString> m_lvZoomRatios;
    int m_lvZoomRatio;

    QList<QString> m_recordingMedias;
    int m_recordingMedia;

    QList<QString> m_captureTargets;
    int m_captureTarget;

    QList<QString> m_stillCaptureModes;
    int m_stillCaptureMode;

    bool m_viewfinder;

    bool m_exposurePreview;

    int m_programShiftValue;
    int m_programShiftValueMin;
    int m_programShiftValueMax;
    int m_programShiftValueStep;

    QList<QString> m_exposureCompensations;
    int m_exposureCompensation;

signals:

public slots:
};

}

#endif // GPCAMERA_H
