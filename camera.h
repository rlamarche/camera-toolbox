#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include "camerapreview.h"

namespace hpis {

class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = 0);
    virtual ~Camera() {};

    virtual bool init() = 0;
    virtual void shutdown() = 0;

    virtual bool capturePreview(CameraPreview** cameraPreview) = 0;
signals:

public slots:
};
}

#endif // CAMERA_H
