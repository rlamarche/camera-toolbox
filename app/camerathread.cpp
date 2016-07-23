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


QString
hpis_gp_port_result_as_string (int result);

hpis::CameraThread::Command::Command()
{

}

hpis::CameraThread::Command::Command(CommandType commandType) : m_commandType(commandType)
{

}

int hpis::CameraThread::Command::x()
{
    return m_x;
}

int hpis::CameraThread::Command::y()
{
    return m_y;
}

QString hpis::CameraThread::Command::value()
{
    return m_value;
}

hpis::CameraThread::CommandType hpis::CameraThread::Command::type()
{
    return m_commandType;
}

hpis::CameraThread::Command hpis::CameraThread::Command::changeAfArea(int x, int y)
{
    Command command(hpis::CameraThread::CommandChangeAfArea);
    command.m_x = x;
    command.m_y = y;

    return command;
}

hpis::CameraThread::Command hpis::CameraThread::Command::setIso(QString value)
{
    Command command(hpis::CameraThread::CommandSetIso);
    command.m_value = value;

    return command;
}



hpis::CameraThread::CameraThread(hpis::Camera* camera, QObject *parent) : QThread(parent),
    m_camera(camera), m_stop(false), m_liveview(false), m_recording(false), m_decoderThread(0)
{
    refreshTimeoutMs = 2000;
}


bool hpis::CameraThread::init()
{
    m_decoderThread = new hpis::DecoderThread(this);
    m_decoderThread->start();

    return m_camera->init();
}

void hpis::CameraThread::shutdown()
{
    m_camera->shutdown();
    m_decoderThread->stop();
    m_decoderThread->wait();
}


void hpis::CameraThread::run()
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
    emit cameraStatus(m_camera->status());
    while (!m_stop) {
        if (time.elapsed() > refreshTimeoutMs)
        {
            time.restart();
            m_camera->readCameraSettings();
            emit cameraStatus(m_camera->status());
        }
        /*
        if (m_commandQueue.isEmpty())
        {
            m_mutex.lock();
            m_condition.wait(&m_mutex);
            m_mutex.unlock();
        }*/

        if (!m_stop && m_liveview) {
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
                doCommand(command);
                emit cameraStatus(m_camera->status());
            }
        }
        m_mutex.unlock();


    }

    shutdown();
    qInfo() << "Stop camera thread";
}

void hpis::CameraThread::doCapturePreview()
{
    CameraPreview* cameraPreview;
    if (m_camera->capturePreview(&cameraPreview))
    {
        if (!m_decoderThread->decodePreview(cameraPreview))
        {
            delete cameraPreview;
        }
    } else {
        //m_liveview = false;
        qInfo() << "The camera is not ready, try again later.";
    }
}

void hpis::CameraThread::stop()
{
    m_mutex.lock();
    m_stop = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void hpis::CameraThread::executeCommand(Command command)
{
    m_mutex.lock();
    m_commandQueue.append(command);
    m_condition.wakeOne();
    m_mutex.unlock();
}

void hpis::CameraThread::capturePreview()
{
    m_condition.wakeOne();
}

void hpis::CameraThread::previewDecoded(QImage image)
{
    emit imageAvailable(image);
}



void hpis::CameraThread::doCommand(Command command)
{
    int programShiftValue;
    switch (command.type()) {
    case CommandStartLiveview:
        if (m_camera->startLiveView())
        {
            m_liveview = true;
        }
        break;

    case CommandStopLiveview:
        if (m_camera->stopLiveview())
        {
            m_liveview = false;
        }
        break;
    case CommandToggleLiveview:
        if (m_liveview)
        {
            if (m_camera->stopLiveview())
            {
                m_liveview = !m_liveview;
            }
        } else {
            if (m_camera->startLiveView())
            {
                m_liveview = !m_liveview;
            }
        }
        break;

    case CommandIncreaseAperture:
        m_camera->increaseAperture();
        break;

    case CommandDecreaseAperture:
        m_camera->decreaseAperture();
        break;

    case CommandEnableIsoAuto:
        m_camera->setIsoAuto(true);
        break;
    case CommandDisableIsoAuto:
        m_camera->setIsoAuto(false);
        break;

    case CommandIncreaseShutterSpeed:
        m_camera->increaseShutterSpeed();
        break;
    case CommandDecreaseShutterSpeed:
        m_camera->decreaseShutterSpeed();
        break;

    case CommandSetIso:
        m_camera->setIso(command.value());
        break;
    case CommandIncreaseIso:
        m_camera->increaseIso();
        break;
    case CommandDecreaseIso:
        m_camera->decreaseIso();
        break;


    case CommandIncreaseProgramShiftValue:
        programShiftValue = m_camera->programShiftValue();
        if (programShiftValue <= m_camera->programShiftValueMax() - m_camera->programShiftValueStep())
        {
            programShiftValue += m_camera->programShiftValueStep();
            m_camera->setProgramShiftValue(programShiftValue);
        }
        break;
    case CommandDecreaseProgramShiftValue:
        programShiftValue = m_camera->programShiftValue();
        if (programShiftValue >= m_camera->programShiftValueMin() + m_camera->programShiftValueStep())
        {
            programShiftValue -= m_camera->programShiftValueStep();
            m_camera->setProgramShiftValue(programShiftValue);
        }
        break;

    case CommandExposureModePlus:
        m_camera->exposureModePlus();
        break;

    case CommandExposureModeMinus:
        m_camera->exposureModeMinus();
        break;
    case CommandIncreaseLvZoomRatio:
        m_camera->increaseLvZoomRatio();
        break;
    case CommandDecreaseLvZoomRatio:
        m_camera->decreaseLvZoomRatio();
        break;

    case CommandEnableExposurePreview:
        m_camera->setExposurePreview(true);
        break;
    case CommandDisableExposurePreview:
        m_camera->setExposurePreview(false);
        break;

    case CommandStartMovie:

        if (m_recording) {
            if (m_camera->stopRecordMovie())
            {
                m_recording = false;
            }
        } else {
            if (m_camera->startRecordMovie())
            {
                m_recording = true;
            }
        }

        break;

    case CommandCapturePhoto:
        m_camera->capturePhoto();
        break;

    case CommandChangeAfArea:
        m_camera->changeAfArea(command.x(), command.y());
        break;

    case CommandPhotoMode:
        m_camera->setCaptureMode(hpis::Camera::CaptureModePhoto);
        break;
    case CommandVideoMode:
        m_camera->setCaptureMode(hpis::Camera::CaptureModeVideo);
        break;

    case CommandAfDrive:
        m_camera->afDrive();
        break;

    default: break;
    }
}
