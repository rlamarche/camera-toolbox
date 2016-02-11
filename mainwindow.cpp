#include "mainwindow.h"

#include <QTimer>

#include <QKeyEvent>
#include <QShowEvent>

#include <QPainter>
#include <QTime>

#include <QDebug>

#include <iostream>

MainWindow::MainWindow(hpis::Camera* camera) :
    QOpenGLWindow(), m_liveviewTimer(0), fps(0)
{
    camera->setParent(this);
    m_overscanLeft = -32;
    m_overscanRight = -32;
    m_overscanTop = -32;
    m_overscanBottom = -32;

    m_cameraThread = new CameraThread(camera, this);

    connect(m_cameraThread, SIGNAL(imageAvailable(QImage)), this, SLOT(showImage(QImage)));
    m_cameraThread->start();
}

MainWindow::~MainWindow()
{
    if (m_liveviewTimer) {
        m_liveviewTimer->stop();
    }
    if (m_cameraThread->isRunning()) {
        m_cameraThread->stop();
        m_cameraThread->wait();
    }
}

void MainWindow::showPreview(QPixmap preview)
{
    static QTime time;
    static int frameCount = 0;

    frameCount ++;

    if (frameCount > 0 && frameCount % 10 == 0) {
        frameCount = 0;
        fps = 10.0f / time.elapsed() * 1000.0f;
        time.restart();
    }

    m_preview = preview;
    update();
}

void MainWindow::showImage(QImage image)
{
    static QTime time;
    static int frameCount = 0;

    frameCount ++;

    if (frameCount > 0 && frameCount % 10 == 0) {
        frameCount = 0;
        fps = 10.0f / time.elapsed() * 1000.0f;
        time.restart();
    }

    m_image = image;
    update();
}
/*
int MainWindow::setToggleWidget(QString widgetName, int toggleValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &toggleValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to toggle widget :" << widgetName;
        }
        return ret;
    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

int MainWindow::setRangeWidget(QString widgetName, float rangeValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &rangeValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to set range value to widget :" << widgetName;
        }

        return ret;
    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

int MainWindow::updateConfig()
{
    int ret = gp_camera_set_config(m_camera, m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to update camera config";
    }
    return ret;
}
*/

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;

    case Qt::Key_Enter:
        m_cameraThread->executeCommand(CameraThread::CommandToggleLiveview);
        break;

    case Qt::Key_Up:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseAperture);
        break;

    case Qt::Key_Down:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseAperture);
        break;

    case Qt::Key_Left:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseShutterSpeed);
        break;

    case Qt::Key_Right:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseShutterSpeed);
        break;
    case Qt::Key_Space:
        m_cameraThread->executeCommand(CameraThread::CommandStartMovie);
        break;

    default:
        QOpenGLWindow::keyPressEvent(event);
    }
}

void MainWindow::resizeGL(int w, int h)
{

}

void MainWindow::paintGL()
{
    QPainter p(this);

    float ratio = (float) m_image.height() / (float) m_image.width();

    float width = (float) m_image.width();
    float height = (this->width() + m_overscanLeft + m_overscanRight) * ratio;

    if (height > (this->height() + m_overscanBottom + m_overscanTop))
    {
        height = (float) (this->height() + m_overscanBottom + m_overscanTop);
        width = height / ratio;
    }

    int x = ((this->width() + m_overscanLeft + m_overscanRight) - (int) width) / 2 - m_overscanLeft;
    int y = ((this->height() + m_overscanBottom + m_overscanTop) - (int) height) / 2 - m_overscanTop;


    //p.drawPixmap(x, y, (int) width, (int) height, m_preview);
    p.drawImage(QRect(x, y, (int) width, (int) height), m_image);
    //p.drawImage(QRect(0, 0, m_image.width(), m_image.height()), m_image);

    p.setPen(QColor(255, 255, 255));
    //p.drawText(QPointF(0, 0), QString("FPS : %1").arg(fps));
    p.drawText(0 - m_overscanLeft, this->height() + m_overscanBottom, QString("FPS : %1").arg(fps));
   // p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter, QString("Hello, World"));

}
