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

#include "gpcamera.h"

#include <QDebug>
#include <QTime>

using namespace hpis;

GPCamera::GPCamera(QString cameraModel, QString cameraPort, QObject *parent) : Camera(parent), m_camera(0), m_model(cameraModel), m_cameraPort(cameraPort)
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
    qInfo() << "Error :" << error;
}

// Camera Infos
QString GPCamera::displayName()
{
    return m_model;
}

QString GPCamera::manufacturer()
{
    return m_manufacturer;
}

QString GPCamera::cameraModel()
{
    return m_cameraModel;
}

// Idle
bool GPCamera::idle(int timeout)
{
    bool statusChanged = false;
    QTime time;
    CameraEventType	evtype;
    void* data;
    CameraFilePath *camera_file_path;
    int timeElapsed = 0;
    bool stop = false;

    time.start();

    while ((timeElapsed < timeout || timeout == 0) && !stop) {
        int ret = gp_camera_wait_for_event(m_camera, timeout == 0 ? 0 : timeout - timeElapsed, &evtype, &data, m_context);

        if (ret < GP_OK) {
            reportError(QString("Unable to get next event: %1").arg(errorCodeToString(ret)));
        }

        switch (evtype) {
        case GP_EVENT_CAPTURE_COMPLETE:
            qDebug() << "Idle event: Capture complete";
            /*
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
            }*/
            break;
        case GP_EVENT_UNKNOWN:
            qDebug() << "Idle event: Unknown";
            if (data != NULL)
            {
                qDebug() << "Unknown data:" << (char*) data;

                QString msg((char*) data);
                if (msg.startsWith("PTP Property"))
                {
                    readCameraSettings();
                    statusChanged = true;
                }
            }
            break;
        case GP_EVENT_TIMEOUT:
            // qDebug() << "Idle event: Timeout";
            break;
        case GP_EVENT_FOLDER_ADDED:
            qDebug() << "Idle event: Folder added";
            break;
        case GP_EVENT_FILE_ADDED:
            qDebug() << "Idle event: File added";
            //fileAdded = true;
            camera_file_path = (CameraFilePath*) data;
            fprintf (stderr, "File %s / %s added to queue.\n", camera_file_path->folder, camera_file_path->name);
            emit(cameraFileAvailable(hpis::CameraFile(QString(camera_file_path->folder), QString(camera_file_path->name))));

            break;
        }
        timeElapsed = time.elapsed();
        if (timeout == 0)
        {
            stop = true;
        }
    }

    return statusChanged;
}

// Camera Init
bool GPCamera::init()
{
    int ret;

    gp_camera_new(&m_camera);

    qInfo() << "Open camera :" << m_model << "at port" << m_cameraPort;

    int model = gp_abilities_list_lookup_model(m_abilitiesList, m_model.toStdString().c_str());
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

    //gpSetRangeWidget("burstnumber", 1);

    m_isInLiveView = false;
    m_isRecording = false;

    ret = gpGetTextWidgetValue(manufacturerWidgetName(), m_manufacturer);
    if (ret < GP_OK) {
        m_manufacturer = "Unknown";
        reportError(QString("Unable to get manufacturer widget : %1").arg(errorCodeToString(ret)));
        //return false;
    }

    ret = gpGetTextWidgetValue(cameraModelWidgetName(), m_cameraModel);
    if (ret < GP_OK) {
        reportError(QString("Unable to get camera model widget : %1").arg(errorCodeToString(ret)));
        m_cameraModel = m_model;
        //return false;
    }

    return true;
}


// Camera shutdown
void GPCamera::shutdown()
{
    gp_camera_free(m_camera);
}


// Capture preview
bool GPCamera::capturePreview(CameraPreview& cameraPreview)
{
    ::CameraFile *file;

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

    const char *dataPtr;
    long unsigned int dataSize;

    gp_file_get_data_and_size(file, &dataPtr, &dataSize);

    unsigned char *data, *jpgStartPtr = NULL, *jpgEndPtr = NULL;
    uint32_t size;

    data = (unsigned char*) dataPtr;
    size = (uint32_t) dataSize;



    QByteArray preview((char*)dataPtr, dataSize);

    cameraPreview = CameraPreview(preview, QString("image/jpeg"));
    gp_file_free(file);

    return true;


    // The following code is in case I desactivate the search of jpeg header in libgphoto2



    // BEGIN CODE FROM camlibs/ptp2/library.c

    /* look for the JPEG SOI marker (0xFFD8) in data */
    jpgStartPtr = (unsigned char*)memchr(data, 0xff, size);
    while(jpgStartPtr && ((jpgStartPtr+1) < (data + size))) {
        if(*(jpgStartPtr + 1) == 0xd8) { /* SOI found */
            break;
        } else { /* go on looking (starting at next byte) */
            jpgStartPtr++;
            jpgStartPtr = (unsigned char*)memchr(jpgStartPtr, 0xff, data + size - jpgStartPtr);
        }
    }
    if(!jpgStartPtr) { /* no SOI -> no JPEG */
        reportError(QString("Unable to find jpg start in preview."));
        gp_file_free(file);
        return false;
    }
    /* if SOI found, start looking for EOI marker (0xFFD9) one byte after SOI
       (just to be sure we will not go beyond the end of the data array) */
    jpgEndPtr = (unsigned char*)memchr(jpgStartPtr+1, 0xff, data+size-jpgStartPtr-1);
    while(jpgEndPtr && ((jpgEndPtr+1) < (data + size))) {
        if(*(jpgEndPtr + 1) == 0xd9) { /* EOI found */
            jpgEndPtr += 2;
            break;
        } else { /* go on looking (starting at next byte) */
            jpgEndPtr++;
            jpgEndPtr = (unsigned char*)memchr(jpgEndPtr, 0xff, data + size - jpgEndPtr);
        }
    }
    if(!jpgEndPtr) { /* no EOI -> no JPEG */
        reportError(QString("Unable to find jpg end in preview."));
        gp_file_free(file);
        return false;
    }

    // END CODE FROM camlibs/ptp2/library.c

    QByteArray preview2((char*)jpgStartPtr, jpgEndPtr-jpgStartPtr);

    cameraPreview = CameraPreview(preview2, QString("image/jpeg"));
    gp_file_free(file);

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
    CameraFilePath *camera_file_path;
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

    /*
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
            camera_file_path = (CameraFilePath*) data;

            fprintf (stderr, "File %s / %s added to queue.\n", camera_file_path->folder, camera_file_path->name);
            emit(cameraFileAvailable(hpis::CameraFile(QString(camera_file_path->folder), QString(camera_file_path->name))));

            break;
        }
    }

*/

    return true;
}

/////////////////////////////////// Camera live view

bool GPCamera::isInLiveView()
{
    return m_isInLiveView;
}

bool GPCamera::gpReadExposureMode()
{
    QString currentExposureMode;
    int ret = gpExtractWidgetChoices(exposureModeWidgetName(), m_exposureModes);
    if (ret < GP_OK)
    {
        m_exposureMode = -1;
        return false;
    }

    ret = gpGetRadioWidgetValue(exposureModeWidgetName(), currentExposureMode);
    if (ret == GP_OK)
    {
        m_exposureMode = m_exposureModes.indexOf(currentExposureMode);
    } else {
        m_exposureMode = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadAperture()
{
    int ret = gpExtractWidgetChoices(apertureWidgetName(), m_cameraApertures);
    if (ret < GP_OK)
    {
        m_cameraAperture = -1;
        return false;
    }

    QString currentAperture;
    ret = gpGetRadioWidgetValue(apertureWidgetName(), currentAperture);
    if (ret == GP_OK)
    {
        m_cameraAperture = m_cameraApertures.indexOf(currentAperture);
    } else {
        m_cameraAperture = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadShutterSpeed()
{
    int ret = gpExtractWidgetChoices(shutterSpeedWidgetName(), m_cameraShutterSpeeds);
    if (ret < GP_OK)
    {
        m_cameraShutterSpeed = -1;
        return false;
    }

    QString currentShutterSpeed;
    ret = gpGetRadioWidgetValue(shutterSpeedWidgetName(), currentShutterSpeed);
    if (ret == GP_OK)
    {
        m_cameraShutterSpeed = m_cameraShutterSpeeds.indexOf(currentShutterSpeed);
    } else {
        m_cameraShutterSpeed = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadIso()
{
    int ret =  gpExtractWidgetChoices(isoWidgetName(), m_cameraIsos);
    if (ret < GP_OK)
    {
        m_cameraIso = -1;
        return false;
    }

    QString currentIso;
    ret = gpGetRadioWidgetValue(isoWidgetName(), currentIso);
    if (ret == GP_OK)
    {
        m_cameraIso = m_cameraIsos.indexOf(currentIso);
    } else {
        m_cameraIso = -1;
        return false;
    }

    return true;
}


bool GPCamera::gpReadRecordingMedia()
{
    int ret = gpExtractWidgetChoices(recordingMediaWidgetName(), m_recordingMedias);
    if (ret < GP_OK)
    {
        m_recordingMedia = -1;
        return false;
    }

    QString currentRecordingMedia;
    ret = gpGetRadioWidgetValue(recordingMediaWidgetName(), currentRecordingMedia);
    if (ret == GP_OK)
    {
        m_recordingMedia = m_recordingMedias.indexOf(currentRecordingMedia);
    } else {
        m_recordingMedia = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadCaptureTarget()
{
    int ret = gpExtractWidgetChoices(captureTargetWidgetName(), m_captureTargets);
    if (ret < GP_OK)
    {
        m_captureTarget = -1;
        return false;
    }

    QString currentCaptureTarget;
    ret = gpGetRadioWidgetValue(captureTargetWidgetName(), currentCaptureTarget);
    if (ret == GP_OK)
    {
        m_captureTarget = m_captureTargets.indexOf(currentCaptureTarget);
    } else {
        m_captureTarget = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadStillCaptureMode()
{
    int ret = gpExtractWidgetChoices(stillCaptureModeWidgetName(), m_stillCaptureModes);
    if (ret < GP_OK)
    {
        m_stillCaptureMode = -1;
        return false;
    }

    QString currentStillCaptureMode;
    ret = gpGetRadioWidgetValue(stillCaptureModeWidgetName(), currentStillCaptureMode);
    if (ret == GP_OK)
    {
        m_stillCaptureMode = m_stillCaptureModes.indexOf(currentStillCaptureMode);
    } else {
        m_stillCaptureMode = -1;
        return false;
    }

    return true;
}

bool GPCamera::gpReadViewFinder()
{
    int currentViewFinder;
    int ret = gpGetToggleWidgetValue(viewfinderWidgetName(), &currentViewFinder);

    if (ret == GP_OK)
    {
        if (currentViewFinder == 1)
        {
            m_viewfinder = true;
        } else if (currentViewFinder == 0) {
            m_viewfinder = false;
        } else {
            m_viewfinder = false;
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool GPCamera::gpReadProgramShiftValue()
{
    float min, max, step;

    float currentProgramShiftValue;
    int ret = gpGetRangeWidgetValue(programShiftValueWidgetName(), &currentProgramShiftValue);
    if (ret == GP_OK)
    {
        m_programShiftValue = (int) currentProgramShiftValue;
    }
    else
    {
        m_programShiftValueMin = 0;
        m_programShiftValueMax = 0;
        m_programShiftValueStep = 0;
        return false;
    }
    ret = gpGetRangeWidgetInfo(programShiftValueWidgetName(), &min, &max, &step);
    if (ret == GP_OK)
    {
        m_programShiftValueMin = (int) min;
        m_programShiftValueMax = (int) max;
        m_programShiftValueStep = (int) step;
    }
    else
    {
        m_programShiftValueMin = 0;
        m_programShiftValueMax = 0;
        m_programShiftValueStep = 0;
        return false;
    }

    return true;
}

bool GPCamera::gpReadExposureCompensation()
{
    int ret = gpExtractWidgetChoices(exposureCompensationWidgetName(), m_exposureCompensations);
    if (ret < GP_OK)
    {
        m_exposureCompensation = -1;
        return false;
    }

    QString currentExposureCompensation;
    ret = gpGetRadioWidgetValue(exposureCompensationWidgetName(), currentExposureCompensation);
    if (ret == GP_OK)
    {
        m_exposureCompensation = m_exposureCompensations.indexOf(currentExposureCompensation);
    } else {
        m_exposureCompensation = -1;
        return false;
    }

    return true;
}

int GPCamera::gpReadRadioWidget(QString widgetName, QStringList& list)
{
    QString current;
    int value;

    int ret = gpExtractWidgetChoices(widgetName, list);
    if (ret < GP_OK)
    {
        value = -1;
        return value;
    }

    ret = gpGetRadioWidgetValue(widgetName, current);
    if (ret == GP_OK)
    {
        value = list.indexOf(current);
    } else {
        value = -1;
        return value;
    }

    return value;
}

int GPCamera::gpWriteRadioWidget(QString widgetName, QStringList& list, QString value)
{
    int i = list.indexOf(value);
    if (i == -1)
    {
        return i;
    }

    int ret = gpSetRadioWidget(widgetName, list[i]);
    if (ret == GP_OK)
    {
        return i;
    } else {
        return -1;
    }
}



QString GPCamera::valueOrNull(QStringList& list, int index)
{
    if (index > -1 && index < list.size())
    {
        return list[index];
    } else {
        return QString::null;
    }
}

bool GPCamera::gpIncrementMode(QString widgetName, QStringList list, int index)
{
    if (index < list.length() - 1)
    {
        int ret = gpSetRadioWidget(widgetName, list[index + 1]);
        if (ret == GP_OK)
        {
            readCameraSettings();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool GPCamera::gpDecrementMode(QString widgetName, QStringList list, int index)
{
    if (index > 0)
    {
        int ret = gpSetRadioWidget(widgetName, list[index - 1]);
        if (ret == GP_OK)
        {
            readCameraSettings();
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
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

int GPCamera::gpExtractWidgetChoices(QString widgetName, QStringList& choices)
{
    int ret;
    CameraWidget* widget;
    choices.clear();
    ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &widget, m_context);
    if (ret != GP_OK || widget == 0x0)
    {
        reportError(QString("Unable to get single config %1: %2").arg(widgetName, errorCodeToString(ret)));
        return ret;
    }

    const char* choiceLabel;
    int n = gp_widget_count_choices(widget);
    if (n < GP_OK) {
        reportError(QString("Unable to count widget choices for %1: %2").arg(widgetName, errorCodeToString(n)));
        return n;
    }

    for (int i = 0; i < n; i ++) {
        int ret = gp_widget_get_choice(widget, i, &choiceLabel);
        if (ret < GP_OK) {
            reportError(QString("Unable to get choice: %1").arg(errorCodeToString(n)));
            return ret;
        }

        choices.append(QString(choiceLabel));
    }

    gp_widget_free(widget);

    return GP_OK;
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
    if (ret != GP_OK || widget == 0x0)
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
    return gpGetTextWidgetValue(widgetName, value);
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

int GPCamera::gpGetTextWidgetValue(QString widgetName, QString& value)
{
    CameraWidget* cameraWidget;
    int ret = gp_camera_get_single_config(m_camera, widgetName.toStdString().c_str(), &cameraWidget, m_context);
    if (ret != GP_OK || cameraWidget == 0x0)
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

///////////////////////////////////////////// Widget names

QString GPCamera::manufacturerWidgetName()
{
    return "manufacturer";
}

QString GPCamera::cameraModelWidgetName()
{
    return "cameramodel";
}

QString GPCamera::apertureWidgetName()
{
    return "aperture"; // valid for canon
}

QString GPCamera::shutterSpeedWidgetName()
{
    return "shutterspeed";
}

QString GPCamera::isoWidgetName()
{
    return "iso";
}

QString GPCamera::afDriveWidgetName()
{
    return "autofocusdrive"; // 90c1
}


QString GPCamera::captureTargetWidgetName()
{
    return "capturetarget";
}

QString GPCamera::exposureCompensationWidgetName()
{
    return "exposurecompensation";
}

QString GPCamera::focusModeWidgetName()
{
    return "focusmode"; // 500a
}

QString GPCamera::focusMeteringWidgetName()
{
    return "focusmetermode"; // 501c
}

QString GPCamera::afAreaWidgetName()
{
    return "changeafarea"; // 9205
}

QString GPCamera::programShiftValueWidgetName()
{
    return "flexibleprogram"; // d109
}

QString GPCamera::exposureIndicatorWidgetName()
{
    return "lightmeter"; // d1b1
}

QString GPCamera::recordingMediaWidgetName()
{
    return "recordingmedia"; // d10b
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

bool GPCamera::exposurePreview()
{
    return m_exposurePreview;
}

// Aperture

QStringList GPCamera::apertures()
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

QStringList GPCamera::shutterSpeeds()
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

QStringList GPCamera::isos()
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

QStringList GPCamera::exposureModes()
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

// Focus mode

QStringList GPCamera::focusModes()
{
    return m_focusModes;
}

QString GPCamera::focusMode()
{
    return valueOrNull(m_focusModes, m_focusMode);
}

bool GPCamera::setFocusMode(QString focusMode)
{
    if (int i = gpWriteRadioWidget(focusModeWidgetName(), m_focusModes, focusMode) > -1)
    {
        m_focusMode = i;
        return true;
    }
    else
    {
        return false;
    }
}

bool GPCamera::focusModePlus()
{
    if (gpIncrementMode(focusModeWidgetName(), m_focusModes, m_focusMode))
    {
        m_focusMode ++;
        return true;
    }
    else
    {
        return false;
    }
}

bool GPCamera::focusModeMinus()
{
    if (gpDecrementMode(focusModeWidgetName(), m_focusModes, m_focusMode))
    {
        m_focusMode --;
        return true;
    }
    else
    {
        return false;
    }
}

// Focus metering

QStringList GPCamera::focusMeterings()
{
    return m_focusMeterings;
}

QString GPCamera::focusMetering()
{
    return valueOrNull(m_focusMeterings, m_focusMetering);
}

bool GPCamera::setFocusMetering(QString focusMetering)
{
    if (int i = gpWriteRadioWidget(focusMeteringWidgetName(), m_focusMeterings, focusMetering) > -1)
    {
        m_focusMetering = i;
        return true;
    }
    else
    {
        return false;
    }
}

bool GPCamera::focusMeteringPlus()
{
    if (gpIncrementMode(focusMeteringWidgetName(), m_focusMeterings, m_focusMetering))
    {
        m_focusMetering ++;
        return true;
    }
    else
    {
        return false;
    }
}

bool GPCamera::focusMeteringMinus()
{
    if (gpDecrementMode(focusMeteringWidgetName(), m_focusMeterings, m_focusMetering))
    {
        m_focusMetering --;
        return true;
    }
    else
    {
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

    QString textValue = QString().sprintf("%dx%d", x * 7360 / 640, y * 4912 / 426);
    ret = gpSetTextWidget(afAreaWidgetName(), textValue);
    if (ret != GP_OK)
    {
        return false;
    }

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


QStringList GPCamera::listFilesInFolder(QString folder)
{
    QStringList list;
    CameraList* cameraList;
    const char* name;
    const char* value;

    gp_list_new(&cameraList);

    int ret = gp_camera_folder_list_folders(m_camera, folder.toStdString().c_str(), cameraList, m_context);
    if (ret != GP_OK)
    {
        reportError(QString("Unable to list folders in folder %1: %2").arg(folder, errorCodeToString(ret)));
        gp_list_free(cameraList);
        return list;
    }

    int count = gp_list_count(cameraList);
    if (count < GP_OK)
    {
        reportError(QString("Unable to list folders in folder %1: %2").arg(folder, errorCodeToString(ret)));
        gp_list_free(cameraList);
        return list;
    }

    for (int i = 0; i < count; ++i)
    {
        gp_list_get_name(cameraList, i, &name);
        gp_list_get_value(cameraList, i, &value);

        QString subFolder = QString(name);

        if (folder.endsWith("/"))
        {
            subFolder = folder + subFolder;
        } else {
            subFolder = folder + "/" + subFolder;
        }

        QStringList subList = listFilesInFolder(subFolder);
        for (int j = 0; j < subList.size(); ++j)
        {
            list.append(subList.at(j));
        }

    }

    gp_list_free(cameraList);

    gp_list_new(&cameraList);

    ret = gp_camera_folder_list_files(m_camera, folder.toStdString().c_str(), cameraList, m_context);
    if (ret != GP_OK)
    {
        reportError(QString("Unable to list files in folder %1: %2").arg(folder, errorCodeToString(ret)));
        gp_list_free(cameraList);
        return list;
    }

    count = gp_list_count(cameraList);
    if (count < GP_OK)
    {
        reportError(QString("Unable to list folders in folder %1: %2").arg(folder, errorCodeToString(ret)));
        gp_list_free(cameraList);
        return list;
    }

    for (int i = 0; i < count; ++i)
    {
        gp_list_get_name(cameraList, i, &name);
        gp_list_get_value(cameraList, i, &value);

        QString path = QString(name);
        if (folder.endsWith("/"))
        {
            path = folder + path;
        } else {
            path = folder + "/" + path;
        }

        list.append(path);
    }


    gp_list_free(cameraList);

    return list;
}

QStringList GPCamera::listFiles()
{
    QStringList list = listFilesInFolder("/");
    qDebug() << list;

    return list;
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
