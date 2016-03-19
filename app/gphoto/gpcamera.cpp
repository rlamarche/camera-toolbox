#include "gpcamera.h"
#include "gpcamerapreview.h"

#include <QDebug>

hpis::GPCamera::GPCamera(QString cameraModel, QString cameraPort, QObject *parent) : hpis::Camera(parent), m_camera(0), m_cameraModel(cameraModel), m_cameraPort(cameraPort)
{
    m_configChanged = false;
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
    qWarning() << "Error :" << error;
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

    refreshCameraSettings();
    setRecordingMedia(RecordingMediaBoth);
    setCaptureTarget(CaptureTargetSDRAM);


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

bool hpis::GPCamera::capturePhoto()
{
    CameraFilePath camera_file_path;
    CameraEventType	evtype;
    void* data;

    strcpy(camera_file_path.folder, "/");
    strcpy(camera_file_path.name, "foo.jpg");

    int ret = gp_camera_trigger_capture(m_camera, m_context);
    //int ret = gp_camera_capture(m_camera, GP_CAPTURE_IMAGE, &camera_file_path, m_context);

    if (ret < GP_OK) {
        reportError(QString("Unable to capture photo: %1").arg(errorCodeToString(ret)));
        return false;
    }

    qDebug() << "Capture success.";
    qDebug() << QString().sprintf("Path on the camera: %s/%s", camera_file_path.folder, camera_file_path.name);

    bool captureComplete = true;
    while (!captureComplete)
    {
        ret = gp_camera_wait_for_event(m_camera, 100, &evtype, &data, m_context);
        if (ret < GP_OK) {
            reportError(QString("Unable to get next event: %1").arg(errorCodeToString(ret)));
            return false;
        }

        switch (evtype) {
        case GP_EVENT_CAPTURE_COMPLETE:
            captureComplete = true;
            qDebug() << "Event: Capture complete";
            break;
        case GP_EVENT_UNKNOWN:
        case GP_EVENT_TIMEOUT:
            qDebug() << "Event: Unknown or timeout";
            break;
        case GP_EVENT_FOLDER_ADDED:
            qDebug() << "Event: Folder added";
            break;
        case GP_EVENT_FILE_ADDED:
            qDebug() << "Event: File added";

            /*
            fprintf (stderr, "File %s / %s added to queue.\n", path->folder, path->name);
            if (nrofqueue) {
                struct queue_entry *q;
                q = realloc(queue, sizeof(struct queue_entry)*(nrofqueue+1));
                if (!q) return GP_ERROR_NO_MEMORY;
                queue = q;
            } else {
                queue = malloc (sizeof(struct queue_entry));
                if (!queue) return GP_ERROR_NO_MEMORY;
            }
            memcpy (&queue[nrofqueue].path, path, sizeof(CameraFilePath));
            queue[nrofqueue].offset = 0;
            nrofqueue++;*/
            break;
        }
    }



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
        return QObject::tr("Unknown error: %1").arg(QString().sprintf("%d", errorCode));
    }
}

bool hpis::GPCamera::lookupWidgets(CameraWidget *widget, QString path)
{
    int n = gp_widget_count_children(widget);
    if (n < GP_OK) {
        reportError(QString("Unable to count widget children: %1").arg(errorCodeToString(n)));
        return false;
    }

    CameraWidget* child;
    const char* widgetName;
    const char* widgetLabel;
    const char* widgetInfo;
    int widgetId;


    int ret = gp_widget_get_name(widget, &widgetName);
    if (ret < GP_OK) {
        reportError(QString("Unable to get widget name: %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_widget_get_label(widget, &widgetLabel);
    if (ret < GP_OK) {
        reportError(QString("Unable to get widget label: %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_widget_get_info(widget, &widgetInfo);
    if (ret < GP_OK) {
        reportError(QString("Unable to get widget info: %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_widget_get_id(widget, &widgetId);
    if (ret < GP_OK) {
        reportError(QString("Unable to get widget id: %1").arg(errorCodeToString(ret)));
        return false;
    }


    path = path.append("/").append(widgetName);

#ifdef QT_DEBUG
    qDebug() << "Found widget:" << path << "with label:" << widgetLabel << "and id:" << widgetId;
#endif

    m_widgets[path] = widget;

    for (int i = 0; i < n; i ++) {
        ret = gp_widget_get_child(widget, i, &child);
        if (ret < GP_OK) {
            reportError(QString("Unable to get widget child %1: %2").arg(QString().sprintf("%d", i), errorCodeToString(ret)));
            return false;
        }

        if (!lookupWidgets(child, path)) {
            return false;
        }
    }

    return true;
}


bool hpis::GPCamera::refreshCameraSettings()
{
    int ret = gp_camera_get_config(m_camera, &m_cameraWindow, m_context);
    lookupWidgets(m_cameraWindow, QString());
    if (ret < GP_OK) {
        reportError(QString("Unable to get camera config: %1").arg(errorCodeToString(ret)));
        return false;
    }

    extractWidgetChoices(m_widgets[apertureWidgetName()], m_cameraApertures);
    extractWidgetChoices(m_widgets[shutterSpeedWidgetName()], m_cameraShutterSpeeds);
    extractWidgetChoices(m_widgets[exposureModeWidgetName()], m_exposureModes);
    extractWidgetChoices(m_widgets[lvZoomRatioWidgetName()], m_lvZoomRatios);
    extractWidgetChoices(m_widgets[recordingMediaWidgetName()], m_recordingMedias);
    extractWidgetChoices(m_widgets[captureTargetWidgetName()], m_captureTargets);


    return readCameraSettings();
}

bool hpis::GPCamera::readCameraSettings()
{
    QString currentAperture = aperture();
    if (!currentAperture.isNull())
    {
        qDebug() << "m_cameraApertures" << m_cameraApertures << "current" << currentAperture;;
        m_cameraAperture = m_cameraApertures.indexOf(currentAperture);
    } else {
        m_cameraAperture = -1;
    }

    QString currentShutterSpeed = shutterSpeed();
    if (!currentShutterSpeed.isNull())
    {
        qDebug() << "m_cameraShutterSpeeds" << m_cameraShutterSpeeds << "current" << currentShutterSpeed;
        m_cameraShutterSpeed = m_cameraShutterSpeeds.indexOf(currentShutterSpeed);
    } else {
        m_cameraShutterSpeed = -1;
    }


    QString currentExposureMode = exposureMode();
    if (!currentExposureMode.isNull())
    {
        qDebug() << "m_exposureModes" << m_exposureModes << "current" << currentExposureMode;
        m_exposureMode = m_exposureModes.indexOf(currentExposureMode);
    } else {
        m_exposureMode = -1;
    }

    QString currentLvZoomRatio = lvZoomRatio();
    if (!currentLvZoomRatio.isNull())
    {
        qDebug() << "m_lvZoomRatios" << m_lvZoomRatios << "current" << currentLvZoomRatio;
        m_lvZoomRatio = m_lvZoomRatios.indexOf(currentLvZoomRatio);
    } else {
        m_lvZoomRatio = -1;
    }

    QString currentRecordingMedia = recordingMedia();
    if (!currentRecordingMedia.isNull())
    {
        qDebug() << "m_recordingMedias" << m_recordingMedias << "current" << currentRecordingMedia;
        m_recordingMedia = m_recordingMedias.indexOf(currentRecordingMedia);
    } else {
        m_recordingMedia = -1;
    }

    QString currentCaptureTarget = captureTarget();
    if (!currentCaptureTarget.isNull())
    {
        qDebug() << "m_captureTarget" << m_captureTargets << "current" << currentCaptureTarget;
        m_captureTarget = m_captureTargets.indexOf(currentCaptureTarget);
    } else {
        m_captureTarget = -1;
    }

    /*
    char* currentValue;

    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_SHUTTERSPEED))) {

        gp_widget_get_value(m_widgets[HPIS_CONFIG_KEY_SHUTTERSPEED], &currentValue);
        m_cameraShutterSpeed = m_cameraShutterSpeeds.indexOf(QString(currentValue));
        qInfo() << "Current shutter speed" << currentValue << "index" << m_cameraShutterSpeed;
    }*/

    return true;
}

bool hpis::GPCamera::applyCameraSettings()
{
    if (m_configChanged)
    {
        qDebug() << "Apply camera settings";
        int ret = gp_camera_set_config(m_camera, m_cameraWindow, m_context);
        if (ret < GP_OK)
        {
            reportError(QString("Unable to apply camera settings: %1").arg(errorCodeToString(ret)));
            return false;
        }
        m_configChanged = false;
    }
    return true;
}

bool hpis::GPCamera::refreshWidget(const QString& widgetName)
{
    if (m_widgets.contains(widgetName)) {
        //gp_widget_free(m_widgets[widgetName]);
        //gp_camera_get_config(m_camera, &(m_widgets[widgetName]), m_context);

        const char* valueStr;
        gp_widget_get_value(m_widgets[widgetName], (void*)&valueStr);
        qInfo() << "Aperture:" << valueStr;
        //m_cameraAperture = m_cameraApertures.indexOf(QString(currentValue));
        //qInfo() << "Current aperture" << currentValue << "index" << m_cameraAperture;
    }

    return true;
}

bool hpis::GPCamera::extractWidgetChoices(CameraWidget* widget, QList<QString>& choices)
{
    choices.clear();

    const char* choiceLabel;
    int n = gp_widget_count_choices(widget);
    if (n < GP_OK) {
        reportError(QString("Unable to count widget choices: %1").arg(errorCodeToString(n)));
        return false;
    }

    for (int i = 0; i < n; i ++) {
        int ret = gp_widget_get_choice(widget, i, &choiceLabel);
        if (ret < GP_OK) {
            reportError(QString("Unable to get choice: %1").arg(errorCodeToString(n)));
            return false;
        }

        choices.append(QString(choiceLabel));
    }

    return true;
}





bool hpis::GPCamera::setToggleWidget(QString widgetName, int toggleValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &toggleValue);
        if (ret < GP_OK) {
            reportError(QString("Unable to toggle widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
            return false;
        }

        m_configChanged = true;
        return true;
    } else {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return false;
    }
}

bool hpis::GPCamera::setRangeWidget(QString widgetName, float rangeValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &rangeValue);
        if (ret < GP_OK) {
            reportError(QString("Unable to set range value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
            return false;
        }

        m_configChanged = true;
        return true;
    } else {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return false;
    }

    return GP_OK;
}

bool hpis::GPCamera::setRadioWidget(QString widgetName, QString radioValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    gp_widget_get_child_by_name (m_cameraWindow, widgetName.toStdString().c_str(), &widget);

    if (widget)
    {
        qInfo() << "Setting value :" << radioValue << "to widget :" << widgetName;
        int ret = gp_widget_set_value(widget, radioValue.toStdString().c_str());
        if (ret < GP_OK) {
            reportError(QString("Unable to set radio value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
            return false;
        }

        m_configChanged = true;
        return true;
    } else {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return false;
    }

    return GP_OK;
}

bool hpis::GPCamera::setTextWidget(QString widgetName, QString textValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    gp_widget_get_child_by_name (m_cameraWindow, widgetName.toStdString().c_str(), &widget);

    if (widget)
    {
        qInfo() << "Setting value :" << textValue << "to widget :" << widgetName;
        int ret = gp_widget_set_value(widget, textValue.toStdString().c_str());
        if (ret < GP_OK) {
            reportError(QString("Unable to set text value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
            return false;
        }

        m_configChanged = true;
        return true;
    } else {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return false;
    }

    return GP_OK;
}



QString hpis::GPCamera::getRadioWidgetValue(QString widgetName)
{
    CameraWidget* cameraWidget = m_widgets[widgetName];
    if (!cameraWidget)
    {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return QString();
    }
    const char* value;

    gp_widget_get_value(cameraWidget, &value);

    return QString(value);
}

float hpis::GPCamera::getRangeWidgetValue(QString widgetName)
{
    CameraWidget* cameraWidget = m_widgets[widgetName];
    if (!cameraWidget)
    {
        reportError(QString("Widget not found: %1").arg(widgetName));
        return 0.0f;
    }
    float value;

    gp_widget_get_value(cameraWidget, &value);

    return value;
}

QString hpis::GPCamera::captureModeWidgetName()
{
    return "/main/other/d1a6";
}

QString hpis::GPCamera::apertureWidgetName()
{

    QString liveviewSelector = getRadioWidgetValue(liveviewSelectorWidgetName());
/*    QString afMode = getRadioWidgetValue(afModeWidgetName());
    qDebug() << "AF Mode:" << afMode;


    QString zoomRatio = getRadioWidgetValue(lvZoomRatioWidgetName());
    qDebug() << "Zoom ratio:" << zoomRatio;
*/
    if (liveviewSelector == "1")
    {
        return "/main/other/d1a9";
    }
    else
    {
        return "/main/other/5007";
    }
}

QString hpis::GPCamera::shutterSpeedWidgetName()
{

    QString liveviewSelector = getRadioWidgetValue(liveviewSelectorWidgetName());

    if (liveviewSelector == "1")
    {
        return "/main/other/d1a8";
    }
    else
    {
        return "/main/other/500d";
    }
}

QString hpis::GPCamera::liveviewSelectorWidgetName()
{
    return "/main/other/d1a6";
}

QString hpis::GPCamera::afModeWidgetName()
{
    return "/main/other/d061";
}

QString hpis::GPCamera::lvZoomRatioWidgetName()
{
    return "/main/other/d1a3";
}

QString hpis::GPCamera::exposureModeWidgetName()
{
    // TODO
    return "/main/other/500e";
}

QString hpis::GPCamera::afAreaWidgetName()
{
    return "/main/actions/changeafarea";
}

QString hpis::GPCamera::afAtWidgetName()
{
    return "/main/other/d05d";
}

QString hpis::GPCamera::afDriveWidgetName()
{
    return "/main/actions/autofocusdrive";
}

QString hpis::GPCamera::recordingMediaWidgetName()
{
    return "/main/settings/recordingmedia";
}

QString hpis::GPCamera::captureTargetWidgetName()
{
    return "/main/settings/capturetarget";
}

hpis::Camera::CaptureMode hpis::GPCamera::captureMode()
{
    return m_captureMode;
}

QString hpis::GPCamera::aperture()
{
    return getRadioWidgetValue(apertureWidgetName());
}

QString hpis::GPCamera::shutterSpeed()
{
    return getRadioWidgetValue(shutterSpeedWidgetName());
}

QString hpis::GPCamera::exposureMode()
{
    return getRadioWidgetValue(exposureModeWidgetName());
}

QString hpis::GPCamera::lvZoomRatio()
{
    return getRadioWidgetValue(lvZoomRatioWidgetName());
}

bool hpis::GPCamera::setRecordingMedia(RecordingMedia recordingMedia)
{
    return setRadioWidget(recordingMediaWidgetName(), m_recordingMedias[recordingMedia]);
}

QString hpis::GPCamera::recordingMedia()
{
    return getRadioWidgetValue(recordingMediaWidgetName());
}

bool hpis::GPCamera::setCaptureTarget(CaptureTarget captureTarget)
{
    return setRadioWidget(captureTargetWidgetName(), m_captureTargets[captureTarget]);
}

QString hpis::GPCamera::captureTarget()
{
    return getRadioWidgetValue(captureTargetWidgetName());
}

bool hpis::GPCamera::setCaptureMode(CaptureMode captureMode)
{
    QString value;
    switch (captureMode)
    {
    case CaptureModePhoto:
        value = "0";
        break;
    case CaptureModeVideo:
        value = "1";
        break;
    }

    if (!value.isNull())
    {
        return setRadioWidget(captureModeWidgetName(), value);
    } else {
        return false;
    }
}

bool hpis::GPCamera::increaseAperture()
{
    readCameraSettings();

    if (m_cameraAperture < m_cameraApertures.length() - 1)
    {
        setRadioWidget(apertureWidgetName(), m_cameraApertures[m_cameraAperture + 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::decreaseAperture()
{
    readCameraSettings();

    if (m_cameraAperture > 0)
    {
        setRadioWidget(apertureWidgetName(), m_cameraApertures[m_cameraAperture - 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::increaseShutterSpeed()
{
    readCameraSettings();

    if (m_cameraShutterSpeed < m_cameraShutterSpeeds.length() - 1)
    {
        setRadioWidget(shutterSpeedWidgetName(), m_cameraShutterSpeeds[m_cameraShutterSpeed + 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::decreaseShutterSpeed()
{
    readCameraSettings();

    if (m_cameraShutterSpeed > 0)
    {
        setRadioWidget(shutterSpeedWidgetName(), m_cameraShutterSpeeds[m_cameraShutterSpeed - 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}


bool hpis::GPCamera::exposureModePlus()
{
    readCameraSettings();

    if (m_exposureMode < m_exposureModes.length() - 1)
    {
        setRadioWidget(exposureModeWidgetName(), m_exposureModes[m_exposureMode + 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::exposureModeMinus()
{
    readCameraSettings();

    if (m_exposureMode > 0)
    {
        setRadioWidget(exposureModeWidgetName(), m_exposureModes[m_exposureMode - 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}



bool hpis::GPCamera::increaseLvZoomRatio()
{
    readCameraSettings();

    if (m_lvZoomRatio < m_lvZoomRatios.length() - 1)
    {
        setRadioWidget(lvZoomRatioWidgetName(), m_lvZoomRatios[m_lvZoomRatio + 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::decreaseLvZoomRatio()
{
    readCameraSettings();

    if (m_lvZoomRatio > 0)
    {
        setRadioWidget(lvZoomRatioWidgetName(), m_lvZoomRatios[m_lvZoomRatio - 1]);
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool hpis::GPCamera::changeAfArea(int x, int y)
{
    setRadioWidget(afAtWidgetName(), "3");
    setRadioWidget(afModeWidgetName(), "0");
    //applyCameraSettings();
    QString textValue = QString().sprintf("%dx%d", x, y);
    setTextWidget(afAreaWidgetName(), textValue);
    applyCameraSettings();
    //refreshCameraSettings();
    setToggleWidget(afDriveWidgetName(), 1);
    setToggleWidget(afDriveWidgetName(), 0);
    applyCameraSettings();
    return true;
}
