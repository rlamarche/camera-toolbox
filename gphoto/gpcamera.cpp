#include "gpcamera.h"
#include "gpcamerapreview.h"

#include <QDebug>

hpis::GPCamera::GPCamera(QString cameraModel, QString cameraPort, QObject *parent) : hpis::Camera(parent), m_camera(0), m_cameraModel(cameraModel), m_cameraPort(cameraPort)
{
    // Create gphoto context
    m_context = gp_context_new();

    // Load abilities list
    gp_abilities_list_new    (&m_abilitiesList);
    gp_abilities_list_load(m_abilitiesList, m_context);

    // Load port info list
    gp_port_info_list_new(&m_portInfoList);
    gp_port_info_list_load(m_portInfoList);
}

hpis::GPCamera::~GPCamera()
{
    qWarning() << "Calling GPCamera destructor";

    gp_port_info_list_free(m_portInfoList);
    gp_abilities_list_free(m_abilitiesList);
    gp_context_unref(m_context);
}

void hpis::GPCamera::reportError(QString error)
{
    qWarning() << error;
}

bool hpis::GPCamera::init()
{
    int ret;

    gp_camera_new(&m_camera);

    qInfo() << "Open camera :" << m_cameraModel << "at port" << m_cameraPort;

    int model = gp_abilities_list_lookup_model(m_abilitiesList, m_cameraModel.toStdString().c_str());
    if (model < GP_OK) {
        reportError(QString("Model not supported (yet) : %1").arg(errorCodeToString(model)));
        return false;
    }

    ret = gp_abilities_list_get_abilities(m_abilitiesList, model, &m_cameraAbilities);
    if (ret < GP_OK) {
        reportError(QString("Unable to get abilities list : %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_set_abilities(m_camera, m_cameraAbilities);
    if (ret < GP_OK) {
        reportError(QString("Unable to set abilities on camera : %1").arg(errorCodeToString(ret)));
        return false;
    }

    // Then associate the camera with the specified port
    int port = gp_port_info_list_lookup_path(m_portInfoList, m_cameraPort.toStdString().c_str());

    if (port < GP_OK) {
        reportError(QString("Unable to lookup port : %1").arg(errorCodeToString(ret)));
        return false;
    }

    GPPortInfo portInfo;
    ret = gp_port_info_list_get_info (m_portInfoList, port, &portInfo);
    if (ret < GP_OK) {
        reportError(QString("Unable to get info on port : %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_set_port_info (m_camera, portInfo);
    if (ret < GP_OK) {
        reportError(QString("Unable to set port info on camera : %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_get_config(m_camera, &m_cameraWindow, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to get root widget : %1").arg(errorCodeToString(ret)));
        return false;
    }

    return true;
}

void hpis::GPCamera::shutdown()
{
    gp_camera_free(m_camera);
}

bool hpis::GPCamera::capturePreview(CameraPreview** cameraPreview)
{
    CameraFile *file;

    int ret = gp_file_new(&file);
    if (ret < GP_OK) {
        reportError(QString("Unable to create camera file for preview : %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_capture_preview(m_camera, file, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to capture preview : %1").arg(errorCodeToString(ret)));
        gp_file_free(file);
        return false;
    }

    *cameraPreview = new GPCameraPreview(file);
    gp_file_unref(file);

    return true;
}



QString hpis::GPCamera::errorCodeToString(int errorCode)
{
    switch (errorCode) {
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
        return QObject::tr("Unknown error %1").arg(QString().sprintf("%d", errorCode));
    }
}
