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
#include "camerathread.h"

#include "decoderthread.h"

#ifdef USE_LIBJPEG
#include <jpeglib.h>
#include <setjmp.h>
#endif

#ifdef USE_LIBTURBOJPEG
#include <turbojpeg.h>
#endif

#include <QDebug>

#include <QTime>

using namespace hpis;

QString
hpis_gp_port_result_as_string (int result);

CameraThread::Command::Command()
{

}

CameraThread::Command::Command(CommandType commandType) : m_commandType(commandType)
{

}

int CameraThread::Command::x()
{
    return m_x;
}

int CameraThread::Command::y()
{
    return m_y;
}

QString CameraThread::Command::propertyName()
{
    return m_propertyName;
}

QVariant CameraThread::Command::value()
{
    return m_value;
}

CameraThread::CommandType CameraThread::Command::type()
{
    return m_commandType;
}

CameraThread::Command CameraThread::Command::changeAfArea(int x, int y)
{
    Command command(CameraThread::CommandChangeAfArea);
    command.m_x = x;
    command.m_y = y;

    return command;
}

CameraThread::Command CameraThread::Command::setIso(QString value)
{
    Command command(CameraThread::CommandSetIso);
    command.m_value = value;

    return command;
}

CameraThread::Command CameraThread::Command::setProperty(QString propertyName, QVariant value)
{
    Command command(CameraThread::CommandSetProperty);
    command.m_propertyName = propertyName;
    command.m_value = value;

    return command;
}



CameraThread::CameraThread(Camera* camera, QObject *parent) : QThread(parent),
    m_camera(camera), m_stop(false), m_decoderThread(0)
{
    refreshTimeoutMs = 1000;
}


bool CameraThread::init()
{
    m_decoderThread = new DecoderThread(this);
    m_decoderThread->start();

    m_cameraInfo = m_camera->info();

    return true;
}

void CameraThread::shutdown()
{
    m_camera->shutdown();
    m_decoderThread->stop();
    m_decoderThread->wait();
}


void CameraThread::run()
{
    qInfo() << "Start camera thread";
    if (!init()) {
        shutdown();
        return;
    }

    Command command;
    QTime time;
    time.start();
    int timeout;

    CameraStatus currentCameraStatus = m_camera->status();

    m_mutex.lock();
    m_cameraStatus = currentCameraStatus;
    m_mutex.unlock();

    emit cameraStatusAvailable(currentCameraStatus);

    while (!m_stop) {
        if (time.elapsed() > refreshTimeoutMs)
        {
            time.restart();
            m_camera->readCameraSettings();
            currentCameraStatus = m_camera->status();

            m_mutex.lock();
            m_cameraStatus = currentCameraStatus;
            m_mutex.unlock();

            emit cameraStatusAvailable(currentCameraStatus);

        }
        /*
        if (m_commandQueue.isEmpty())
        {
            m_mutex.lock();
            m_condition.wait(&m_mutex);
            m_mutex.unlock();
        }*/

        if (!m_stop && currentCameraStatus.isInLiveView()) {
            doCapturePreview();
        } else if (m_commandQueue.isEmpty()) {
            timeout = refreshTimeoutMs - time.elapsed();
            if (timeout > 0) {
                m_mutex.lock();
                m_condition.wait(&m_mutex, timeout);
                m_mutex.unlock();
            }
        }

        m_mutex.lock();
        if (!m_stop && !m_commandQueue.isEmpty()) {
            while (!m_commandQueue.isEmpty())
            {
                command = m_commandQueue.dequeue();
                m_mutex.unlock();

                currentCameraStatus = doCommand(command);

                m_mutex.lock();
                m_cameraStatus = currentCameraStatus;
                m_mutex.unlock();

                emit cameraStatusAvailable(m_cameraStatus);

                m_mutex.lock();
            }
            m_mutex.unlock();
        } else {
            m_mutex.unlock();
        }
    }

    shutdown();
    qInfo() << "Stop camera thread";
}

CameraStatus CameraThread::cameraStatus()
{
    QMutexLocker locker(&m_mutex);
    return m_cameraStatus;
}

CameraInfo CameraThread::cameraInfo()
{
    return m_cameraInfo;
}

void CameraThread::setCameraSettings(CameraSettings cameraSettings)
{
    if (!cameraSettings.aperture().isNull())
    {
        executeCommand(Command::setProperty("aperture", cameraSettings.aperture()));
    }
    if (!cameraSettings.shutterSpeed().isNull())
    {
        executeCommand(Command::setProperty("shutterSpeed", cameraSettings.shutterSpeed()));
    }
    if (!cameraSettings.iso().isNull())
    {
        executeCommand(Command::setProperty("iso", cameraSettings.iso()));
    }
}

void CameraThread::doCapturePreview()
{
    CameraPreview cameraPreview;
    if (m_camera->capturePreview(cameraPreview))
    {
        emit previewAvailable(cameraPreview);
        m_decoderThread->decodePreview(cameraPreview);
    } else {
        //m_liveview = false;
        qInfo() << "The camera is not ready, try again later.";
    }
}

void CameraThread::stop()
{
    m_mutex.lock();
    m_stop = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void CameraThread::executeCommand(Command command)
{
    m_mutex.lock();
    m_commandQueue.append(command);
    m_condition.wakeOne();
    m_mutex.unlock();
}

void CameraThread::capturePreview()
{
    m_condition.wakeOne();
}

void CameraThread::previewDecoded(QImage image)
{
    emit imageAvailable(image);
}



CameraStatus CameraThread::doCommand(Command command) const
{
    CameraStatus cameraStatus = m_camera->status();
    bool success = false;
    int programShiftValue;

    switch (command.type()) {
    case CommandStartLiveview:
        success = m_camera->startLiveView();
        break;

    case CommandStopLiveview:
        success = m_camera->stopLiveView();
        break;
    case CommandToggleLiveview:
        if (cameraStatus.isInLiveView())
        {
            success = m_camera->stopLiveView();
        } else {
            success = m_camera->startLiveView();
        }
        break;

    case CommandIncreaseAperture:
        success = m_camera->increaseAperture();
        break;

    case CommandDecreaseAperture:
        success = m_camera->decreaseAperture();
        break;

    case CommandEnableIsoAuto:
        success = m_camera->setIsoAuto(true);
        break;
    case CommandDisableIsoAuto:
        success = m_camera->setIsoAuto(false);
        break;

    case CommandIncreaseShutterSpeed:
        success = m_camera->increaseShutterSpeed();
        break;
    case CommandDecreaseShutterSpeed:
        success = m_camera->decreaseShutterSpeed();
        break;

    case CommandSetIso:
        success = m_camera->setIso(command.value().toString());
        break;
    case CommandIncreaseIso:
        success = m_camera->increaseIso();
        break;
    case CommandDecreaseIso:
        success = m_camera->decreaseIso();
        break;


    case CommandIncreaseProgramShiftValue:
        programShiftValue = m_camera->programShiftValue();
        if (programShiftValue <= m_camera->programShiftValueMax() - m_camera->programShiftValueStep())
        {
            programShiftValue += m_camera->programShiftValueStep();
            success = m_camera->setProgramShiftValue(programShiftValue);
        }
        break;
    case CommandDecreaseProgramShiftValue:
        programShiftValue = m_camera->programShiftValue();
        if (programShiftValue >= m_camera->programShiftValueMin() + m_camera->programShiftValueStep())
        {
            programShiftValue -= m_camera->programShiftValueStep();
            success = m_camera->setProgramShiftValue(programShiftValue);
        }
        break;


    case CommandIncreaseExposureCompensation:
        success = m_camera->increaseExposureCompensation();
        break;
    case CommandDecreaseExposureCompensation:
        success = m_camera->decreaseExposureCompensation();
        break;


    case CommandExposureModePlus:
        success = m_camera->exposureModePlus();
        break;

    case CommandExposureModeMinus:
        success = m_camera->exposureModeMinus();
        break;
    case CommandIncreaseLvZoomRatio:
        success = m_camera->increaseLvZoomRatio();
        break;
    case CommandDecreaseLvZoomRatio:
        success = m_camera->decreaseLvZoomRatio();
        break;

    case CommandEnableExposurePreview:
        success = m_camera->setExposurePreview(true);
        break;
    case CommandDisableExposurePreview:
        success = m_camera->setExposurePreview(false);
        break;

    case CommandStartStopMovie:
        if (cameraStatus.isRecording()) {
            success = m_camera->stopRecordMovie();
        } else {
            success = m_camera->startRecordMovie();
        }
        break;
    case CommandStartMovie:
        if (!cameraStatus.isRecording())
        {
            success = m_camera->startRecordMovie();
        }
        break;
    case CommandStopMovie:
        if (cameraStatus.isRecording())
        {
            success = m_camera->stopRecordMovie();
        }
        break;

    case CommandCapturePhoto:
        success = m_camera->capturePhoto();
        break;

    case CommandChangeAfArea:
        success = m_camera->changeAfArea(command.x(), command.y());
        break;

    case CommandPhotoMode:
        success = m_camera->setCaptureMode(Camera::CaptureModePhoto);

        break;
    case CommandVideoMode:
        success = m_camera->setCaptureMode(Camera::CaptureModeVideo);
        break;

    case CommandAfDrive:
        success = m_camera->afDrive();
        break;

    case CommandSetProperty:
        qInfo() << "Set property" << command.propertyName().toStdString().c_str() << "to" << command.value();
        success = m_camera->setProperty(command.propertyName().toStdString().c_str(), command.value());
        break;
    default: break;
    }

    if (!success)
    {
        qInfo() << "Command failed";
    }

    return m_camera->status();
}

