#ifndef GPCAMERA_H
#define GPCAMERA_H

#include "../camera.h"


#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>

namespace hpis {

class GPCamera : public Camera
{
    Q_OBJECT
public:
    explicit GPCamera(QString cameraModel, QString cameraPort, QObject *parent = 0);
    ~GPCamera();

    bool init();
    void shutdown();
    bool capturePreview(CameraPreview** cameraPreview);

protected:
    void reportError(QString error);
    QString errorCodeToString(int errorCode);

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

    QList<QString> m_cameraApertures;
    int m_cameraAperture;

    QList<QString> m_cameraShutterSpeeds;
    int m_cameraShutterSpeed;

signals:

public slots:
};

}

#endif // GPCAMERA_H
