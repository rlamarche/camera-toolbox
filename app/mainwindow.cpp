#include "mainwindow.h"

#include <QTimer>

#include <QKeyEvent>
#include <QShowEvent>

#include <QPainter>
#include <QTime>

#include <QDebug>

#include <iostream>

MainWindow::MainWindow(hpis::Camera* camera) :
    QOpenGLWindow(), m_liveviewTimer(0), m_fps(0)
{
    camera->setParent(this);

#ifdef USE_RPI
    m_overscanLeft = -32;
    m_overscanRight = -32;
    m_overscanTop = -32;
    m_overscanBottom = -32;
#else
    m_overscanLeft = 0;
    m_overscanRight = 0;
    m_overscanTop = 0;
    m_overscanBottom = 0;
#endif

    m_cameraThread = new CameraThread(camera, this);

    connect(m_cameraThread, SIGNAL(imageAvailable(QImage)), this, SLOT(showImage(QImage)));
    connect(m_cameraThread, SIGNAL(cameraStatus(hpis::CameraStatus)), this, SLOT(cameraStatus(hpis::CameraStatus)));
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

    if (frameCount > 0 && frameCount % 60 == 0) {
        frameCount = 0;
        m_fps = 60.0f / time.elapsed() * 1000.0f;
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
        m_fps = 10.0f / time.elapsed() * 1000.0f;
        time.restart();
    }

    m_image = image;
    update();
}


void MainWindow::cameraStatus(hpis::CameraStatus cameraStatus)
{

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

    case Qt::Key_C:
        m_cameraThread->executeCommand(CameraThread::CommandPhotoMode);
        break;
    case Qt::Key_V:
        m_cameraThread->executeCommand(CameraThread::CommandVideoMode);
        break;


    case Qt::Key_D:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseAperture);
        break;
    case Qt::Key_Q:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseAperture);
        break;

    case Qt::Key_Z:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseShutterSpeed);
        break;
    case Qt::Key_S:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseShutterSpeed);
        break;

    case Qt::Key_E:
        m_cameraThread->executeCommand(CameraThread::CommandExposureModePlus);
        break;
    case Qt::Key_A:
        m_cameraThread->executeCommand(CameraThread::CommandExposureModeMinus);
        break;

    case Qt::Key_Plus:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseLvZoomRatio);
        break;
    case Qt::Key_Minus:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseLvZoomRatio);
        break;

    case Qt::Key_Space:
        m_cameraThread->executeCommand(CameraThread::CommandStartMovie);
        break;
    case Qt::Key_Return:
        m_cameraThread->executeCommand(CameraThread::CommandCapturePhoto);
        break;
    default:
        QOpenGLWindow::keyPressEvent(event);
    }
}

void MainWindow::resizeGL(int w, int h)
{

}

void MainWindow::mousePressEvent(QMouseEvent *ev)
{
    int x = ev->x();
    int y = ev->y();


    if (!m_imageRect.isNull() && !m_image.isNull())
    {
        int xImage = m_image.width() * (x - m_imageRect.x()) / m_imageRect.width();
        int yImage = m_image.height() * (y - m_imageRect.y()) / m_imageRect.height();

        qDebug() << xImage << yImage;


        m_cameraThread->executeCommand(CameraThread::Command::changeAfArea(xImage, yImage));
    }
}

void MainWindow::calcImageRect()
{
    float ratio = (float) m_image.height() / (float) m_image.width();

    float width = (float) this->width();
    float height = (this->width() + m_overscanLeft + m_overscanRight) * ratio;

    if (height > (this->height() + m_overscanBottom + m_overscanTop))
    {
        height = (float) (this->height() + m_overscanBottom + m_overscanTop);
        width = height / ratio;
    }

    int x = ((this->width() + m_overscanLeft + m_overscanRight) - (int) width) / 2 - m_overscanLeft;
    int y = ((this->height() + m_overscanBottom + m_overscanTop) - (int) height) / 2 - m_overscanTop;

    m_imageRect = QRect(x, y, (int) width, (int) height);
}

void MainWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    QPainter p(this);

    calcImageRect();


    //p.drawPixmap(x, y, (int) width, (int) height, m_preview);
    p.drawImage(m_imageRect, m_image);
    //p.drawImage(QRect(0, 0, m_image.width(), m_image.height()), m_image);

    p.setPen(QColor(255, 255, 255));
    //p.drawText(QPointF(0, 0), QString("FPS : %1").arg(fps));
    p.drawText(0 - m_overscanLeft, this->height() + m_overscanBottom, QString("FPS : %1").arg(m_fps));
   // p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter, QString("Hello, World"));

}
