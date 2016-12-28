#ifndef GPCANONCAMERA_H
#define GPCANONCAMERA_H

#include "../gpcamera.h"

#include <QObject>


namespace hpis {

class GPCanonCamera : public GPCamera
{
    Q_OBJECT
public:
    explicit GPCanonCamera(QString cameraModel, QString cameraPort, QObject *parent = 0);
    virtual QSet<CameraCapability> capabilities();

    // Init / Shutdown / Read
    bool init();
    void shutdown();
    bool readCameraSettings();

    bool capturePreview(CameraPreview& cameraPreview);

protected:
    // ---------- Read

    // ---------- Get / Set

    // ---------- Widget names
    virtual QString apertureWidgetName();
    virtual QString shutterSpeedWidgetName();
    virtual QString isoWidgetName();

    virtual QString exposureModeWidgetName();
    virtual QString focusModeWidgetName();

    virtual QString exposureCompensationWidgetName();
signals:

public slots:
};



}
#endif // GPCANONCAMERA_H
