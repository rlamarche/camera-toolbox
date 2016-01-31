#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "camerathread.h"


#include <QOpenGLWindow>
#include <QMap>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>


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
    int findWidgets(CameraWidget* widget);

    /*
    int setToggleWidget(QString widgetName, int toggleValue);
    int setRangeWidget(QString widgetName, float rangeValue);
    int updateConfig();*/

    // Events
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

private:
    // Timers
    QTimer*                 m_lookupCameraTimer;
    QTimer*                 m_liveviewTimer;

    // Threads
    CameraThread m_cameraThread;

    // Data
    int m_overscanLeft;
    int m_overscanRight;
    int m_overscanTop;
    int m_overscanBottom;

    QPixmap m_preview;
    QImage m_image;
    float fps;

public slots:
    void showPreview(QPixmap preview);
    void showImage(QImage image);
    //void lookupCamera();
};

#endif // MAINWINDOW_H
