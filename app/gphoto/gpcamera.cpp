#include "gpcamera.h"
#include "gpcamerapreview.h"

#include <QDebug>

using namespace hpis;

GPCamera::GPCamera(QString cameraModel, QString cameraPort, QObject *parent) : Camera(parent), m_camera(0), m_cameraModel(cameraModel), m_cameraPort(cameraPort)
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

GPCamera::~GPCamera()
{
    qWarning() << "Calling GPCamera destructor";

    gp_port_info_list_free(m_portInfoList);
    gp_abilities_list_free(m_abilitiesList);
    gp_context_unref(m_context);
}

void GPCamera::reportError(QString error)
{
    qWarning() << "Error :" << error;
}


// Camera Init
bool GPCamera::init()
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

    readCameraSettings();

    setRecordingMedia(RecordingMediaCard);
    setCaptureTarget(CaptureTargetCard);
    setStillCaptureMode(StillCaptureModeSingleShot);
    //gpSetRangeWidget("burstnumber", 1);

    //applyCameraSettings();

    m_isInLiveView = false;
    m_isRecording = false;

    return true;
}


// Camera shutdown
void GPCamera::shutdown()
{
    gp_camera_free(m_camera);
}


// Capture preview
bool GPCamera::capturePreview(CameraPreview** cameraPreview)
{
    CameraFile *file;

    int ret = gp_file_new(&file);
    if (ret < GP_OK) {
        reportError(QString("Unable to create camera file for preview: %1").arg(errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_capture_preview(m_camera, file, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to capture preview: %1").arg(errorCodeToString(ret)));
        gp_file_free(file);
        return false;
    }

    *cameraPreview = new GPCameraPreview(file);
    gp_file_unref(file);

    return true;
}

///////////////////////////////////////// Camera control

bool GPCamera::startRecordMovie()
{
    int ret = gpSetToggleWidget("movie", 1);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to start record a movie: %1").arg(errorCodeToString(ret)));
        return false;
    } else {
        m_isRecording = true;
        return true;
    }
}

bool GPCamera::stopRecordMovie()
{
    int ret = gpSetToggleWidget("movie", 0);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to stop record a movie: %1").arg(errorCodeToString(ret)));
        return false;
    } else {
        m_isRecording = false;
        return true;
    }
}

bool GPCamera::isRecording()
{
    return m_isRecording;
}


bool GPCamera::capturePhoto()
{
    //CameraFilePath camera_file_path;
    CameraEventType	evtype;
    void* data;

    //strcpy(camera_file_path.folder, "/");
    //strcpy(camera_file_path.name, "foo.jpg");

    int ret = gp_camera_trigger_capture(m_camera, m_context);
    //int ret = gp_camera_capture(m_camera, GP_CAPTURE_IMAGE, &camera_file_path, m_context);

    if (ret != GP_OK) {
        reportError(QString("Unable to capture photo: %1").arg(errorCodeToString(ret)));
        return false;
    }

    qDebug() << "Capture success.";
    //qDebug() << QString().sprintf("Path on the camera: %s/%s", camera_file_path.folder, camera_file_path.name);

    //return true;

    bool captureComplete = false;
    bool fileAdded = false;

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
            if (fileAdded)
            {
                qDebug() << "Event: Capture complete";
            } else {
                qDebug() << "Event: Capture failure";
                if (data != NULL)
                {
                    qDebug() << "Data :" << (char*) data;
                }
            }
            break;
        case GP_EVENT_UNKNOWN:
            qDebug() << "Event: Unknown";
            if (data != NULL)
            {
                qDebug() << "Unknown data :" << (char*) data;
            }
            break;
        case GP_EVENT_TIMEOUT:
            qDebug() << "Event: Timeout";
            break;
        case GP_EVENT_FOLDER_ADDED:
            qDebug() << "Event: Folder added";
            break;
        case GP_EVENT_FILE_ADDED:
            qDebug() << "Event: File added";
            fileAdded = true;

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

/////////////////////////////////// Camera live view

bool GPCamera::startLiveView()
{
    int ret = gpSetToggleWidget(viewfinderWidgetName(), 1);
    if (ret == GP_OK)
    {
        m_isInLiveView = true;
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool GPCamera::stopLiveView()
{
    int ret = gpSetToggleWidget(viewfinderWidgetName(), 0);
    if (ret == GP_OK)
    {
        m_isInLiveView = false;
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

bool GPCamera::isInLiveView()
{
    return m_isInLiveView;
}

bool GPCamera::readCameraSettings()
{
    int ret;
    QString currentCaptureMode;
    gpGetRadioWidgetValue(captureModeWidgetName(), currentCaptureMode);
    if (!currentCaptureMode.isNull())
    {
        if (currentCaptureMode == "1")
        {
            m_captureMode = CaptureModeVideo;
        } else {
            m_captureMode = CaptureModePhoto;
        }
    } else {
        m_captureMode = CaptureModePhoto;
    }

    extractWidgetChoices(apertureWidgetName(), m_cameraApertures);

    QString currentAperture;
    gpGetRadioWidgetValue(apertureWidgetName(), currentAperture);
    if (!currentAperture.isNull())
    {
        //qDebug() << "m_cameraApertures" << m_cameraApertures << "current" << currentAperture;;
        m_cameraAperture = m_cameraApertures.indexOf(currentAperture);
    } else {
        m_cameraAperture = -1;
    }

    extractWidgetChoices(shutterSpeedWidgetName(), m_cameraShutterSpeeds);

    QString currentShutterSpeed;
    gpGetRadioWidgetValue(shutterSpeedWidgetName(), currentShutterSpeed);
    if (!currentShutterSpeed.isNull())
    {
        //qDebug() << "m_cameraShutterSpeeds" << m_cameraShutterSpeeds << "current" << currentShutterSpeed;
        m_cameraShutterSpeed = m_cameraShutterSpeeds.indexOf(currentShutterSpeed);
    } else {
        m_cameraShutterSpeed = -1;
    }

    extractWidgetChoices(isoWidgetName(), m_cameraIsos);

    QString currentIso;
    gpGetRadioWidgetValue(isoWidgetName(), currentIso);
    if (!currentIso.isNull())
    {
        //qDebug() << "m_cameraIsos" << m_cameraIsos << "current" << currentIso;
        m_cameraIso = m_cameraIsos.indexOf(currentIso);
    } else {
        m_cameraIso = -1;
    }

    extractWidgetChoices(exposureModeWidgetName(), m_exposureModes);

    QString currentExposureMode;
    gpGetRadioWidgetValue(exposureModeWidgetName(), currentExposureMode);
    if (!currentExposureMode.isNull())
    {
        //qDebug() << "m_exposureModes" << m_exposureModes << "current" << currentExposureMode;
        m_exposureMode = m_exposureModes.indexOf(currentExposureMode);
    } else {
        m_exposureMode = -1;
    }

    extractWidgetChoices(lvZoomRatioWidgetName(), m_lvZoomRatios);

    QString currentLvZoomRatio;
    gpGetRadioWidgetValue(lvZoomRatioWidgetName(), currentLvZoomRatio);
    if (!currentLvZoomRatio.isNull())
    {
        //qDebug() << "m_lvZoomRatios" << m_lvZoomRatios << "current" << currentLvZoomRatio;
        m_lvZoomRatio = m_lvZoomRatios.indexOf(currentLvZoomRatio);
    } else {
        m_lvZoomRatio = -1;
    }

    extractWidgetChoices(recordingMediaWidgetName(), m_recordingMedias);

    QString currentRecordingMedia;
    gpGetRadioWidgetValue(recordingMediaWidgetName(), currentRecordingMedia);
    if (!currentRecordingMedia.isNull())
    {
        //qDebug() << "m_recordingMedias" << m_recordingMedias << "current" << currentRecordingMedia;
        m_recordingMedia = m_recordingMedias.indexOf(currentRecordingMedia);
    } else {
        m_recordingMedia = -1;
    }

    extractWidgetChoices(captureTargetWidgetName(), m_captureTargets);

    QString currentCaptureTarget;
    gpGetRadioWidgetValue(captureTargetWidgetName(), currentCaptureTarget);
    if (!currentCaptureTarget.isNull())
    {
        //qDebug() << "m_captureTargets" << m_captureTargets << "current" << currentCaptureTarget;
        m_captureTarget = m_captureTargets.indexOf(currentCaptureTarget);
    } else {
        m_captureTarget = -1;
    }

    extractWidgetChoices(stillCaptureModeWidgetName(), m_stillCaptureModes);

    QString currentStillCaptureMode;
    gpGetRadioWidgetValue(stillCaptureModeWidgetName(), currentStillCaptureMode);
    if (!currentStillCaptureMode.isNull())
    {
        //qDebug() << "m_stillCaptureModes" << m_stillCaptureModes << "current" << currentStillCaptureMode;
        m_stillCaptureMode = m_stillCaptureModes.indexOf(currentStillCaptureMode);
    } else {
        m_stillCaptureMode = -1;
    }

    QString currentExposurePreview;
    gpGetRadioWidgetValue(exposurePreviewWidgetName(), currentExposurePreview);
    if (currentExposurePreview == "1") {
        m_exposurePreview = true;
    } else if (currentExposurePreview == "0") {
        m_exposurePreview = false;
    }

    QString currentIsoAuto;
    gpGetRadioWidgetValue(isoAutoWidgetName(), currentIsoAuto);
    if (currentIsoAuto == "On") {
        m_cameraIsoAuto = true;
    } else if (currentIsoAuto == "Off") {
        m_cameraIsoAuto = false;
    }

    int currentViewFinder;
    gpGetToggleWidgetValue(viewfinderWidgetName(), &currentViewFinder);
    if (currentViewFinder == 1)
    {
        m_viewfinder = true;
    } else {
        m_viewfinder = false;
    }

    float min, max, step;

    float currentProgramShiftValue;
    ret = gpGetRangeWidgetValue(programShiftValueWidgetName(), &currentProgramShiftValue);
    if (ret == GP_OK)
    {
        m_programShiftValue = (int) currentProgramShiftValue;
    }
    ret = gpGetRangeWidgetInfo(programShiftValueWidgetName(), &min, &max, &step);
    if (ret == GP_OK)
    {
        m_programShiftValueMin = (int) min;
        m_programShiftValueMax = (int) max;
        m_programShiftValueStep = (int) step;
    }


    extractWidgetChoices(exposureCompensationWidgetName(), m_exposureCompensations);

    QString currentExposureCompensation;
    gpGetRadioWidgetValue(exposureCompensationWidgetName(), currentExposureCompensation);
    if (!currentExposureCompensation.isNull())
    {
        m_exposureCompensation = m_exposureCompensations.indexOf(currentExposureCompensation);
    } else {
        m_exposureCompensation = -1;
    }


    return true;
}

////////////////////////////////// GPhoto

int GPCamera::gpGetRangeWidgetInfo(QString widgetName, float* min, float* max, float* increment)
{
    int ret;
    CameraWidget* widget;
    ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    ret = gp_widget_get_range(widget, min, max, increment);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get range on widget %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    return ret;
}

bool GPCamera::extractWidgetChoices(QString widgetName, QList<QString>& choices)
{
    int ret;
    CameraWidget* widget;
    ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return false;
    }
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

    gp_widget_free(widget);

    return true;
}

int GPCamera::gpSetToggleWidget(QString widgetName, int toggleValue)
{
    int ret;
    CameraWidget* widget ;
    ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }
    ret = gp_widget_set_value(widget, &toggleValue);
    if (ret < GP_OK) {
        reportError(QString("Unable to toggle widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    ret = gp_camera_set_single_config(m_camera, widgetName.toStdString().c_str(), widget, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to set radio value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    return GP_OK;
}

int GPCamera::gpSetRangeWidget(QString widgetName, float rangeValue)
{
    CameraWidget* widget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    qInfo() << "Setting value :" << rangeValue << "to widget :" << widgetName;
    ret = gp_widget_set_value(widget, &rangeValue);
    if (ret < GP_OK) {
        reportError(QString("Unable to set range value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return false;
    }

    ret = gp_camera_set_single_config(m_camera, widgetName.toStdString().c_str(), widget, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to set radio value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    return GP_OK;
}

int GPCamera::gpSetRadioWidget(QString widgetName, QString radioValue)
{
    CameraWidget* widget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    qInfo() << "Setting value :" << radioValue << "to widget :" << widgetName;
    ret = gp_widget_set_value(widget, radioValue.toStdString().c_str());
    if (ret < GP_OK) {
        reportError(QString("Unable to set radio value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    ret = gp_camera_set_single_config(m_camera, widgetName.toStdString().c_str(), widget, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to set radio value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }


    return GP_OK;
}

int GPCamera::gpSetTextWidget(QString widgetName, QString textValue)
{
    CameraWidget* widget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    qInfo() << "Setting value :" << textValue << "to widget :" << widgetName;
    ret = gp_widget_set_value(widget, textValue.toStdString().c_str());
    if (ret < GP_OK) {
        reportError(QString("Unable to set text value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    ret = gp_camera_set_single_config(m_camera, widgetName.toStdString().c_str(), widget, m_context);
    if (ret < GP_OK) {
        reportError(QString("Unable to set text value to widget: %1 error: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    return GP_OK;
}

int GPCamera::gpGetToggleWidgetValue(QString widgetName, int* value)
{
    CameraWidget* cameraWidget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &cameraWidget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }
    gp_widget_get_value(cameraWidget, value);
    gp_widget_free(cameraWidget);

    return GP_OK;
}

int GPCamera::gpGetRadioWidgetValue(QString widgetName, QString& value)
{
    CameraWidget* cameraWidget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &cameraWidget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        value = QString::null;
        return ret;
    }
    const char* valuePtr;
    ret = gp_widget_get_value(cameraWidget, &valuePtr);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        value = QString::null;
        return ret;
    }
    value = QString(valuePtr);
    gp_widget_free(cameraWidget);

    return GP_OK;
}

int GPCamera::gpGetRangeWidgetValue(QString widgetName, float* value)
{
    CameraWidget* cameraWidget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &cameraWidget, m_context);
    if (ret < GP_OK)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }
    gp_widget_get_value(cameraWidget, value);
    gp_widget_free(cameraWidget);

    return GP_OK;
}

///////////////////////////////////////////// Widget names

QString GPCamera::viewfinderWidgetName()
{
    return "viewfinder";
}

QString GPCamera::captureModeWidgetName()
{
    return "d1a6";
}

QString GPCamera::apertureWidgetName()
{
    if (m_viewfinder && m_captureMode == CaptureModeVideo)
    {
        return "movief-number"; // d1a9
    }
    else
    {
        return "f-number"; // 5007
    }
}

QString GPCamera::shutterSpeedWidgetName()
{
    if (m_viewfinder)
    {
        if (m_captureMode == CaptureModePhoto)
        {
            return "shutterspeed2"; // d100
        } else {
            return "movieshutterspeed"; // d1a8
        }
    } else {
        return "shutterspeed"; // 500d
    }
}

QString GPCamera::isoWidgetName()
{
    if (m_viewfinder && m_captureMode == CaptureModeVideo)
    {
        return "movieiso"; // d1aa
    } else {
        return "iso"; // 500f
    }
}

QString GPCamera::isoAutoWidgetName()
{
    return "autoiso"; // d054
}


QString GPCamera::liveviewSelectorWidgetName()
{
    return "d1a6";
}

QString GPCamera::afModeWidgetName()
{
    return "liveviewaffocus"; // d061
}

QString GPCamera::lvZoomRatioWidgetName()
{
    return "d1a3";
}

QString GPCamera::exposureModeWidgetName()
{
    return "expprogram"; // 500e
}

QString GPCamera::afAreaWidgetName()
{
    return "changeafarea"; // 9205
}

QString GPCamera::afAtWidgetName()
{
    return "liveviewafmode"; // d05d
}

QString GPCamera::afDriveWidgetName()
{
    return "autofocusdrive"; // 90c1
}

QString GPCamera::recordingMediaWidgetName()
{
    return "recordingmedia"; // d10b
}

QString GPCamera::captureTargetWidgetName()
{
    return "capturetarget";
}

QString GPCamera::stillCaptureModeWidgetName()
{
    return "capturemode";
}

QString GPCamera::exposurePreviewWidgetName()
{
    return "d1a5";
}

QString GPCamera::exposureCompensationWidgetName()
{
    if (m_viewfinder && m_captureMode == CaptureModeVideo)
    {
        return "d1ab";
    } else {
        return "exposurecompensation"; // exposurecompensation2 // 5010
    }
}

QString GPCamera::programShiftValueWidgetName()
{
    return "flexibleprogram"; // d109
}

QString GPCamera::exposureIndicatorWidgetName()
{
    return "lightmeter"; // d1b1
}

////////////////////////////////////// Camera control

hpis::Camera::CaptureMode GPCamera::captureMode()
{
    return m_captureMode;
}


// ISO Auto

bool GPCamera::isoAuto()
{
    return m_cameraIsoAuto;
}

bool GPCamera::setIsoAuto(bool isoAuto)
{
    int ret;
    if (isoAuto)
    {
        ret = gpSetRadioWidget(isoAutoWidgetName(), QString("On"));
    }
    else
    {
        ret = gpSetRadioWidget(isoAutoWidgetName(), QString("Off"));
    }

    if (ret == GP_OK)
    {
        m_cameraIsoAuto = isoAuto;
        return true;
    } else {
        return false;
    }
}

// Recording media

bool GPCamera::setRecordingMedia(RecordingMedia recordingMedia)
{
    int ret = gpSetRadioWidget(recordingMediaWidgetName(), m_recordingMedias[recordingMedia]);
    if (ret == GP_OK)
    {
        m_recordingMedia = recordingMedia;
        return true;
    } else {
        return false;
    }
}

QString GPCamera::recordingMedia()
{
    if (m_recordingMedia > -1 && m_recordingMedia < m_recordingMedias.size())
    {
        return m_recordingMedias[m_recordingMedia];
    } else {
        return QString::null;
    }
}

// Capture target

bool GPCamera::setCaptureTarget(CaptureTarget captureTarget)
{
    return gpSetRadioWidget(captureTargetWidgetName(), m_captureTargets[captureTarget]);
}

QString GPCamera::captureTarget()
{
    if (m_captureTarget > -1 && m_captureTarget < m_captureTargets.size())
    {
        return m_captureTargets[m_captureTarget];
    } else {
        return QString::null;
    }
}

// Still capture mode

bool GPCamera::setStillCaptureMode(StillCaptureMode stillCaptureMode)
{
    int ret = gpSetRadioWidget(stillCaptureModeWidgetName(), m_stillCaptureModes[stillCaptureMode]);
    if (ret == GP_OK)
    {
        m_stillCaptureMode = stillCaptureMode;
        return true;
    } else {
        return false;
    }
}

QString GPCamera::stillCaptureMode()
{
    if (m_stillCaptureMode > -1 && m_stillCaptureMode < m_stillCaptureModes.size())
    {
        return m_stillCaptureModes[m_stillCaptureMode];
    } else {
        return QString::null;
    }
}

// Exposure preview

bool GPCamera::setExposurePreview(bool exposurePreview)
{
    int ret;
    if (exposurePreview)
    {
        ret = gpSetRadioWidget(exposurePreviewWidgetName(), QString("1"));
    } else {
        ret = gpSetRadioWidget(exposurePreviewWidgetName(), QString("0"));
    }

    if (ret == GP_OK)
    {
        m_exposurePreview = exposurePreview;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::exposurePreview()
{
    return m_exposurePreview;
}

// Capture mode

bool GPCamera::setCaptureMode(CaptureMode captureMode)
{
    int ret;
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
        ret = gpSetRadioWidget(captureModeWidgetName(), value);
    } else {
        return false;
    }

    if (ret == GP_OK)
    {
        m_captureMode = captureMode;
        readCameraSettings();
        return true;
    } else {
        return false;
    }
}

// Aperture

QList<QString> GPCamera::apertures()
{
    return m_cameraApertures;
}

QString GPCamera::aperture()
{
    if (m_cameraAperture > -1 && m_cameraAperture < m_cameraApertures.size())
    {
        return m_cameraApertures[m_cameraAperture];
    } else {
        return QString::null;
    }
}

bool GPCamera::setAperture(QString aperture)
{
    qInfo() << "Received aperture" << aperture;
    int i = m_cameraApertures.indexOf(aperture);
    if (i == -1)
    {
        return false;
    }

    int ret = gpSetRadioWidget(apertureWidgetName(), m_cameraApertures[i]);
    if (ret == GP_OK)
    {
        m_cameraAperture = i;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::increaseAperture()
{
    if (m_cameraAperture < m_cameraApertures.length() - 1)
    {
        int ret = gpSetRadioWidget(apertureWidgetName(), m_cameraApertures[m_cameraAperture + 1]);

        if (ret == GP_OK)
        {
            m_cameraAperture = m_cameraAperture + 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::decreaseAperture()
{
    if (m_cameraAperture > 0)
    {
        int ret = gpSetRadioWidget(apertureWidgetName(), m_cameraApertures[m_cameraAperture - 1]);
        if (ret == GP_OK)
        {
            m_cameraAperture = m_cameraAperture - 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Shutter speed

QList<QString> GPCamera::shutterSpeeds()
{
    return m_cameraShutterSpeeds;
}

QString GPCamera::shutterSpeed()
{
    if (m_cameraShutterSpeed > -1 && m_cameraShutterSpeed < m_cameraShutterSpeeds.size())
    {
        return m_cameraShutterSpeeds[m_cameraShutterSpeed];
    } else {
        return QString::null;
    }
}

bool GPCamera::setShutterSpeed(QString shutterSpeed)
{
    int i = m_cameraShutterSpeeds.indexOf(shutterSpeed);
    if (i == -1)
    {
        return false;
    }

    int ret = gpSetRadioWidget(shutterSpeedWidgetName(), m_cameraShutterSpeeds[i]);
    if (ret == GP_OK)
    {
        m_cameraShutterSpeed = i;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::increaseShutterSpeed()
{
    if (m_cameraShutterSpeed < m_cameraShutterSpeeds.length() - 1)
    {
        int ret = gpSetRadioWidget(shutterSpeedWidgetName(), m_cameraShutterSpeeds[m_cameraShutterSpeed + 1]);
        if (ret == GP_OK)
        {
            m_cameraShutterSpeed = m_cameraShutterSpeed + 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::decreaseShutterSpeed()
{
    if (m_cameraShutterSpeed > 0)
    {
        int ret = gpSetRadioWidget(shutterSpeedWidgetName(), m_cameraShutterSpeeds[m_cameraShutterSpeed - 1]);
        if (ret == GP_OK)
        {
            m_cameraShutterSpeed = m_cameraShutterSpeed - 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// ISO

QList<QString> GPCamera::isos()
{
    return m_cameraIsos;
}

QString GPCamera::iso()
{
    if (m_cameraIso > -1 && m_cameraIso < m_cameraIsos.size())
    {
        return m_cameraIsos[m_cameraIso];
    } else {
        return QString::null;
    }
}

bool GPCamera::setIso(QString iso)
{
    int i = m_cameraIsos.indexOf(iso);
    if (i == -1)
    {
        return false;
    }

    int ret = gpSetRadioWidget(isoWidgetName(), m_cameraIsos[i]);
    if (ret == GP_OK)
    {
        m_cameraIso = i;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::increaseIso()
{
    if (m_cameraIso < m_cameraIsos.length() - 1)
    {
        int ret = gpSetRadioWidget(isoWidgetName(), m_cameraIsos[m_cameraIso + 1]);
        if (ret == GP_OK)
        {
            m_cameraIso = m_cameraIso + 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::decreaseIso()
{
    if (m_cameraIso > 0)
    {
        int ret = gpSetRadioWidget(isoWidgetName(), m_cameraIsos[m_cameraIso - 1]);
        if (ret == GP_OK)
        {
            m_cameraIso = m_cameraIso - 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Exposure mode

QList<QString> GPCamera::exposureModes()
{
    return m_exposureModes;
}

QString GPCamera::exposureMode()
{
    if (m_exposureMode > -1 && m_exposureMode < m_exposureModes.size())
    {
        return m_exposureModes[m_exposureMode];
    } else {
        return QString::null;
    }
}

bool GPCamera::setExposureMode(QString exposureMode)
{
    int i = m_exposureModes.indexOf(exposureMode);
    if (i == -1)
    {
        return false;
    }

    int ret = gpSetRadioWidget(exposureModeWidgetName(), m_exposureModes[i]);
    if (ret == GP_OK)
    {
        m_exposureMode = i;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::exposureModePlus()
{
    if (m_exposureMode < m_exposureModes.length() - 1)
    {
        int ret = gpSetRadioWidget(exposureModeWidgetName(), m_exposureModes[m_exposureMode + 1]);
        if (ret == GP_OK)
        {
            m_exposureMode = m_exposureMode + 1;
            readCameraSettings();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::exposureModeMinus()
{
    if (m_exposureMode > 0)
    {
        int ret = gpSetRadioWidget(exposureModeWidgetName(), m_exposureModes[m_exposureMode - 1]);
        if (ret == GP_OK)
        {
            m_exposureMode = m_exposureMode - 1;
            readCameraSettings();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Live view zoom ratio

QString GPCamera::lvZoomRatio()
{
    if (m_lvZoomRatio > -1 && m_lvZoomRatio < m_lvZoomRatios.size())
    {
        return m_lvZoomRatios[m_lvZoomRatio];
    } else {
        return QString::null;
    }
}

bool GPCamera::increaseLvZoomRatio()
{
    if (m_lvZoomRatio < m_lvZoomRatios.length() - 1)
    {
        int ret = gpSetRadioWidget(lvZoomRatioWidgetName(), m_lvZoomRatios[m_lvZoomRatio + 1]);
        if (ret == GP_OK)
        {
            m_lvZoomRatio = m_lvZoomRatio + 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::decreaseLvZoomRatio()
{
    if (m_lvZoomRatio > 0)
    {
        int ret = gpSetRadioWidget(lvZoomRatioWidgetName(), m_lvZoomRatios[m_lvZoomRatio - 1]);
        if (ret == GP_OK)
        {
            m_lvZoomRatio = m_lvZoomRatio - 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Focus

bool GPCamera::afDrive()
{
    int ret = gpSetToggleWidget(afDriveWidgetName(), 1);
    if (ret != GP_OK)
    {
        return false;
    }
    ret = gpSetToggleWidget(afDriveWidgetName(), 0);
    if (ret != GP_OK)
    {
        return false;
    }

    return true;
}

bool GPCamera::changeAfArea(int x, int y)
{
    int ret;
    //setRadioWidget(afAtWidgetName(), "3");
    //setRadioWidget(afModeWidgetName(), "0");
    //applyCameraSettings();
    QString textValue = QString().sprintf("%dx%d", x * 7360 / 640, y * 4912 / 426);
    ret = gpSetTextWidget(afAreaWidgetName(), textValue);
    if (ret != GP_OK)
    {
        return false;
    }
    //applyCameraSettings();
    //refreshCameraSettings();
    return afDrive();
}

// Program shift value

int GPCamera::programShiftValue()
{
    return m_programShiftValue;
}

bool GPCamera::setProgramShiftValue(int value)
{
    float fValue = (float) value;

    int ret = gpSetRangeWidget(programShiftValueWidgetName(), fValue);
    if (ret == GP_OK)
    {
        m_programShiftValue = value;
        return true;
    } else {
        return false;
    }
}

int GPCamera::programShiftValueMin()
{
    return m_programShiftValueMin;
}

int GPCamera::programShiftValueMax()
{
    return m_programShiftValueMax;
}

int GPCamera::programShiftValueStep()
{
    return m_programShiftValueStep;
}

// Exposure compensation

QString GPCamera::exposureCompensation()
{
    if (m_exposureCompensation > -1 && m_exposureCompensation < m_exposureCompensations.size())
    {
        return m_exposureCompensations[m_exposureCompensation];
    } else {
        return QString::null;
    }
}

bool GPCamera::setExposureCompensation(QString value)
{
    int i = m_exposureCompensations.indexOf(value);
    if (i == -1)
    {
        return false;
    }

    int ret = gpSetRadioWidget(exposureCompensationWidgetName(), m_exposureCompensations[i]);
    if (ret == GP_OK)
    {
        m_exposureCompensation = i;
        return true;
    } else {
        return false;
    }
}

bool GPCamera::increaseExposureCompensation()
{
    if (m_exposureCompensation < m_exposureCompensations.length() - 1)
    {
        int ret = gpSetRadioWidget(exposureCompensationWidgetName(), m_exposureCompensations[m_exposureCompensation + 1]);
        if (ret == GP_OK)
        {
            m_exposureCompensation = m_exposureCompensation + 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::decreaseExposureCompensation()
{
    if (m_exposureCompensation > 0)
    {
        int ret = gpSetRadioWidget(exposureCompensationWidgetName(), m_exposureCompensations[m_exposureCompensation - 1]);
        if (ret == GP_OK)
        {
            m_exposureCompensation = m_exposureCompensation - 1;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// GPhoto

QString GPCamera::errorCodeToString(int errorCode)
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
