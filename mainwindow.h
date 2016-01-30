#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QOpenGLWindow>
#include <QMap>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>

#define HPIS_CONFIG_KEY_VIEWFINDER "viewfinder"

namespace Ui {
class MainWindow;
}

class MainWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event);
    int findWidgets(CameraWidget* widget);

    int toggleWidget(QString widgetName, int toggleValue);

private:
    QTimer*                 lookupCameraTimer;
    QTimer*                 liveviewTimer;

    GPContext*              context;
    CameraAbilitiesList*    abilitieslist;
    GPPortInfoList*         portinfolist;

    Camera*                 camera;
    CameraWidget*           cameraWindow;

    QMap<QString, CameraWidget*> widgets;
public slots:
    void lookupCamera();
    void capturePreview();
};

#endif // MAINWINDOW_H
