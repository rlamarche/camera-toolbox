#include "camerathread.h"

#include "decoderthread.h"

#ifdef USE_LIBJPEG
#include <jpeglib.h>
#include <setjmp.h>
#endif

#ifdef USE_LIBTURBOJPEG
#include <turbojpeg.h>
#endif

#include <gphoto2/gphoto2-port-info-list.h>

#include <QDebug>

#include <QTime>


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



CameraThread::CameraThread(hpis::Camera* camera, QObject *parent) : QThread(parent),
    m_camera(camera), m_stop(false), m_liveview(false), m_recording(false), m_decoderThread(0)
{
    refreshTimeoutMs = 1000;
}


void CameraThread::init()
{
    m_decoderThread = new DecoderThread(this);
    m_decoderThread->start();

    m_camera->init();
}

void CameraThread::shutdown()
{
    m_camera->shutdown();
    m_decoderThread->stop();
    m_decoderThread->wait();
}

/*
int CameraThread::lookupWidgets(CameraWidget* widget) {
    int n = gp_widget_count_children(widget);
    CameraWidget* child;
    const char* widgetName;

    gp_widget_get_name(widget, &widgetName);
    qDebug() << "Found widget" << widgetName;

    m_widgets[widgetName] = widget;

    for (int i = 0; i < n; i ++) {
        int ret = gp_widget_get_child(widget, i, &child);
        if (ret < GP_OK) {
            return ret;
        }

        ret = lookupWidgets(child);
        if (ret < GP_OK) {
            return ret;
        }
    }

    return GP_OK;
}

void CameraThread::extractCameraCapabilities()
{
    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_APERTURE))) {
        m_cameraApertures = extractWidgetChoices(m_widgets[HPIS_CONFIG_KEY_APERTURE]);
    }
    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_SHUTTERSPEED))) {
        m_cameraShutterSpeeds = extractWidgetChoices(m_widgets[HPIS_CONFIG_KEY_SHUTTERSPEED]);
    }
}

void CameraThread::refreshCameraSettings()
{
    char* currentValue;

    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_APERTURE))) {

        gp_widget_get_value(m_widgets[HPIS_CONFIG_KEY_APERTURE], &currentValue);
        m_cameraAperture = m_cameraApertures.indexOf(QString(currentValue));
        qInfo() << "Current aperture" << currentValue << "index" << m_cameraAperture;
    }

    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_SHUTTERSPEED))) {

        gp_widget_get_value(m_widgets[HPIS_CONFIG_KEY_SHUTTERSPEED], &currentValue);
        m_cameraShutterSpeed = m_cameraShutterSpeeds.indexOf(QString(currentValue));
        qInfo() << "Current shutter speed" << currentValue << "index" << m_cameraShutterSpeed;
    }
}

QList<QString> CameraThread::extractWidgetChoices(CameraWidget* widget)
{
    QList<QString> choices;

    const char* choiceLabel;
    int n = gp_widget_count_choices(widget);

    for (int i = 0; i < n; i ++) {
        gp_widget_get_choice(widget, i, &choiceLabel);
        choices.append(QString(choiceLabel));

        qInfo() << "Found choice :" << QString(choiceLabel);
    }

    return choices;
}
*/



void CameraThread::run()
{
    qInfo() << "Start camera thread";
    init();

    Command command;
    QTime time;
    time.start();
    int timeout;
    emit cameraStatus(m_camera->status());
    while (!m_stop) {
        if (time.elapsed() > refreshTimeoutMs)
        {
            time.restart();
//            m_camera->refreshCameraSettings();
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
            }
            m_camera->applyCameraSettings();
        }
        m_mutex.unlock();


    }

    shutdown();
    qInfo() << "Stop camera thread";
}

void CameraThread::doCapturePreview()
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

    /*
    CameraFile *file;

    int ret = gp_file_new(&file);
    if (ret < GP_OK) {
        qWarning() << "Unable to create camera file for preview";
        return;
    }

    ret = gp_camera_capture_preview(m_camera, file, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to capture preview";
        gp_file_free(file);
        return;
    }

    unsigned long int size;
    const char *data;

    gp_file_get_data_and_size(file, &data, &size);

    //QImage image = decodeImageTurbo(data, size);
    //QImage image = decodeImage(data, size);
    //emit imageAvailable(image);
    if (!m_decoderThread->decodePreview(file))
    {
        gp_file_free(file);
    }

    //m_preview.loadFromData((uchar*) data, size, "JPG");
    //emit previewAvailable(m_preview);

*/
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



void CameraThread::doCommand(Command command)
{
    int ret;
    switch (command.type()) {
    case CommandStartLiveview:
        m_liveview = true;
        /*
        ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
        if (ret == GP_OK) {
            m_liveview = true;
        }
*/
        break;

    case CommandStopLiveview:
        /*
        ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        if (ret == GP_OK) {
            m_liveview = false;
        }
        */
        break;
    case CommandToggleLiveview:
        m_liveview = !m_liveview;
        /*
        if (m_liveview) {
            ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        } else {
            ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
        }
        if (ret == GP_OK) {
            m_liveview = !m_liveview;
        }
        */
        break;

    case CommandIncreaseAperture:
        m_camera->increaseAperture();
        /*
        if (m_cameraAperture < m_cameraApertures.length() - 1) {
            QString newAperture = m_cameraApertures[m_cameraAperture + 1];
            ret = setRadioWidget(HPIS_CONFIG_KEY_APERTURE, newAperture.toStdString().c_str());
            if (ret == GP_OK) {
                m_cameraAperture ++;
            }
        }
        */
        break;
    case CommandDecreaseAperture:
        m_camera->decreaseAperture();
        /*
        if (m_cameraAperture > 0) {
            QString newAperture = m_cameraApertures[m_cameraAperture - 1];
            ret = setRadioWidget(HPIS_CONFIG_KEY_APERTURE, newAperture.toStdString().c_str());
            if (ret == GP_OK) {
                m_cameraAperture --;
            }
        }
        */
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

    case CommandIncreaseIso:
        m_camera->increaseIso();
        break;
    case CommandDecreaseIso:
        m_camera->decreaseIso();
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
        /*
        if (m_recording) {
            ret = setToggleWidget(HPIS_CONFIG_KEY_STOP_MOVIE, 1);
        } else {
            ret = setToggleWidget(HPIS_CONFIG_KEY_START_MOVIE, 1);
        }
        if (ret == GP_OK) {
            m_recording = !m_recording;
        }
        */
        break;

    case CommandCapturePhoto:
        //  m_liveview = false;
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

    default: break;
    }

    //updateConfig();
}


/*

int CameraThread::setToggleWidget(QString widgetName, int toggleValue)
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

int CameraThread::setRangeWidget(QString widgetName, float rangeValue)
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

int CameraThread::setRadioWidget(QString widgetName, const char* radioValue)
{
    CameraWidget* widget = m_widgets[widgetName];


    gp_widget_get_child_by_name (m_cameraWindow, widgetName.toStdString().c_str(), &widget);

    if (widget)
    {
        qInfo() << "Setting value :" << radioValue << "to widget :" << widgetName;
        int ret = gp_widget_set_value(widget, radioValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to set radio value to widget :" << hpis_gp_port_result_as_string(ret);
        }

        return ret;
    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

int CameraThread::updateConfig()
{
    int ret = gp_camera_set_config(m_camera, m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to update camera config :" << hpis_gp_port_result_as_string(ret);
    }
    return ret;
}

QString
hpis_gp_port_result_as_string (int result)
{
    switch (result) {
    case GP_OK:
        return QObject::tr("No error");
    case GP_ERROR:
        return QObject::tr("Unspecified error");
    case GP_ERROR_IO:
        return QObject::tr("I/O problem");
    case GP_ERROR_BAD_PARAMETERS:
        return QObject::tr("Bad parameters");
    case GP_ERROR_NOT_SUPPORTED:
        return QObject::tr("Unsupported operation");
    case  GP_ERROR_FIXED_LIMIT_EXCEEDED:
        return QObject::tr("Fixed limit exceeded");
    case GP_ERROR_TIMEOUT:
        return QObject::tr("Timeout reading from or writing to the port");
    case GP_ERROR_IO_SUPPORTED_SERIAL:
        return QObject::tr("Serial port not supported");
    case GP_ERROR_IO_SUPPORTED_USB:
        return QObject::tr("USB port not supported");
    case GP_ERROR_UNKNOWN_PORT:
        return QObject::tr("Unknown port");
    case GP_ERROR_NO_MEMORY:
        return QObject::tr("Out of memory");
    case GP_ERROR_LIBRARY:
        return QObject::tr("Error loading a library");
    case GP_ERROR_IO_INIT:
        return QObject::tr("Error initializing the port");
    case GP_ERROR_IO_READ:
        return QObject::tr("Error reading from the port");
    case GP_ERROR_IO_WRITE:
        return QObject::tr("Error writing to the port");
    case GP_ERROR_IO_UPDATE:
        return QObject::tr("Error updating the port settings");
    case GP_ERROR_IO_SERIAL_SPEED:
        return QObject::tr("Error setting the serial port speed");
    case GP_ERROR_IO_USB_CLEAR_HALT:
        return QObject::tr("Error clearing a halt condition on the USB port");
    case GP_ERROR_IO_USB_FIND:
        return QObject::tr("Could not find the requested device on the USB port");
    case GP_ERROR_IO_USB_CLAIM:
        return QObject::tr("Could not claim the USB device");
    case GP_ERROR_IO_LOCK:
        return QObject::tr("Could not lock the device");
    case GP_ERROR_HAL:
        return QObject::tr("libhal error");
    case GP_ERROR_CORRUPTED_DATA:
        return QObject::tr("Corrupted data received");
    case GP_ERROR_FILE_EXISTS:
        return QObject::tr("File already exists");
    case GP_ERROR_MODEL_NOT_FOUND:
        return QObject::tr("Specified camera model was not found");
    case GP_ERROR_DIRECTORY_NOT_FOUND:
        return QObject::tr("Specified directory was not found")        ;
    case GP_ERROR_FILE_NOT_FOUND:
        return QObject::tr("Specified directory was not found");
    case GP_ERROR_DIRECTORY_EXISTS:
        return QObject::tr("Specified directory already exists");
    case GP_ERROR_CAMERA_BUSY:
        return QObject::tr("The camera is already busy");
    case GP_ERROR_PATH_NOT_ABSOLUTE:
        return QObject::tr("Path is not absolute");
    case GP_ERROR_CANCEL:
        return QObject::tr("Cancellation successful");
    case GP_ERROR_CAMERA_ERROR:
        return QObject::tr("Unspecified camera error");
    case GP_ERROR_OS_FAILURE:
        return QObject::tr("Unspecified failure of the operating system");
    case GP_ERROR_NO_SPACE:
        return QObject::tr("Not enough space");
    default:
        return QObject::tr("Unknown error %1").arg(QString().sprintf("%d", result));
    }
}
*/
