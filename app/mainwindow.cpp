/*
 * This file is part of Camera Toolbox.
 *   (https://github.com/rlamarche/camera-toolbox)
 * Copyright (c) 2016 Romain Lamarche.
 *
 * Camera Toolbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * Camera Toolbox is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include <QTimer>

#include <QKeyEvent>
#include <QShowEvent>

#include <QPainter>
#include <QTime>

#include <QDebug>

#include <iostream>

using namespace hpis;

MainWindow::MainWindow(CameraThread* cameraThread) :
    QOpenGLWindow(), m_liveviewTimer(0), m_fps(0)
{
#ifdef USE_RPI
    m_overscanLeft = -32;
    m_overscanRight = -32;
    m_overscanTop = -32;
    m_overscanBottom = -32;
#else
    m_overscanLeft = -32;
    m_overscanRight = -32;
    m_overscanTop = -32;
    m_overscanBottom = -32;
#endif

    m_cameraThread = cameraThread;

    //m_imageAnalyzer = new ImageAnalyzer();
    //m_histogramDisplay = new HistogramDisplay();

    //m_histogramDisplay->setImageAnalyzer(m_imageAnalyzer);

    connect(m_cameraThread, SIGNAL(imageAvailable(QImage)), this, SLOT(showImage(QImage)));
    connect(m_cameraThread, SIGNAL(cameraStatusAvailable(hpis::CameraStatus)), this, SLOT(cameraStatus(hpis::CameraStatus)));
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

    //delete m_histogramDisplay;
    //delete m_imageAnalyzer;
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

    //QImage toAnalyze = m_image.convertToFormat(QImage::Format_ARGB32);

    //m_imageAnalyzer->analyze((unsigned int*) toAnalyze.bits(), toAnalyze.width(), toAnalyze.height(), toAnalyze.bytesPerLine() / 4, true);

    update();
}


void MainWindow::cameraStatus(CameraStatus cameraStatus)
{
    m_cameraStatus = cameraStatus;
    update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;

    case Qt::Key_Enter:
    case Qt::Key_L:
        m_cameraThread->executeCommand(CameraThread::CommandToggleLiveview);
        break;

    case Qt::Key_F:
        m_cameraThread->executeCommand(CameraThread::CommandAfDrive);
        break;

    case Qt::Key_C:
        m_cameraThread->executeCommand(CameraThread::CommandPhotoMode);
        break;
    case Qt::Key_V:
        m_cameraThread->executeCommand(CameraThread::CommandVideoMode);
        break;

    case Qt::Key_D:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandIncreaseProgramShiftValue);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandIncreaseAperture);
        }
        break;
    case Qt::Key_A:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandDecreaseProgramShiftValue);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandDecreaseAperture);
        }
        break;

    case Qt::Key_W:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseShutterSpeed);
        break;
    case Qt::Key_S:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseShutterSpeed);
        break;

    case Qt::Key_E:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandIncreaseExposureCompensation);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandIncreaseIso);
        }
        break;
    case Qt::Key_Q:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandDecreaseExposureCompensation);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandDecreaseIso);
        }
        break;

    case Qt::Key_T:
        m_cameraThread->executeCommand(CameraThread::CommandExposureModePlus);
        break;
    case Qt::Key_R:
        m_cameraThread->executeCommand(CameraThread::CommandExposureModeMinus);
        break;
    case Qt::Key_I:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandDisableIsoAuto);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandEnableIsoAuto);
        }
        break;

    case Qt::Key_P:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            m_cameraThread->executeCommand(CameraThread::CommandDisableExposurePreview);
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandEnableExposurePreview);
        }
        break;

    case Qt::Key_Plus:
        m_cameraThread->executeCommand(CameraThread::CommandIncreaseLvZoomRatio);
        break;
    case Qt::Key_Minus:
        m_cameraThread->executeCommand(CameraThread::CommandDecreaseLvZoomRatio);
        break;

    case Qt::Key_Space:
        m_cameraThread->executeCommand(CameraThread::CommandStartStopMovie);
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

    //QImage histogram(640, 480, QImage::Format_ARGB32);
    //m_histogramDisplay->renderHistogram((unsigned int *) histogram.bits(), histogram.bytesPerLine() / 4, histogram.width(), histogram.height(), false);
    //p.drawImage(m_imageRect, histogram);


    p.setPen(QColor(255, 255, 255));
    //p.drawText(QPointF(0, 0), QString("FPS : %1").arg(fps));
    p.drawText(0 - m_overscanLeft, this->height() + m_overscanBottom, QString("FPS : %1").arg(m_fps));
    p.drawText(0 - m_overscanLeft, 0 - m_overscanTop, QString("Aperture : %1").arg(m_cameraStatus.aperture()));
    p.drawText(width() - m_overscanRight - 500, 0 - m_overscanTop, QString("Speed : %1").arg(m_cameraStatus.shutterSpeed()));

    p.drawText(width() - m_overscanRight - 500, this->height() + m_overscanBottom, QString("ISO : %1").arg(m_cameraStatus.iso()));



   // p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter, QString("Hello, World"));

}
