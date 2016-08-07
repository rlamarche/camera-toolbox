#include "gpnikoncamera.h"

#include <QSet>
#include <QDebug>

using namespace hpis;

GPNikonCamera::GPNikonCamera(QString cameraModel, QString cameraPort, QObject *parent) : GPCamera(cameraModel, cameraPort, parent)
{

}


QSet<GPCamera::CameraCapability> GPNikonCamera::capabilities()
{
    // TODO
    QSet<GPCamera::CameraCapability> capabilities;
    QString exposureMode = this->exposureMode();

    if (exposureMode == "M")
    {
        capabilities.insert(GPCamera::ChangeAperture);
        capabilities.insert(GPCamera::ChangeShutterSpeed);
        capabilities.insert(GPCamera::ChangeAperture);
        capabilities.insert(GPCamera::ChangeIso);
    }

    return capabilities;
}



// -------------------------- Capture preview


static uint32_t fixBytesOrder(uint32_t val);
static uint16_t fixBytesOrder(uint16_t val);
static uint8_t fixBytesOrder(uint8_t val);
static void fixBytesOrder(NikonLiveViewHeader* lvHeader);


// Capture preview
bool GPNikonCamera::capturePreview(CameraPreview& cameraPreview)
{
    if (m_cameraModel == CAMERA_NIKON_D800)
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

        const char *dataPtr;
        long unsigned int dataSize;

        gp_file_get_data_and_size(file, &dataPtr, &dataSize);


        NikonLiveViewHeader nikonLvHeader = *((NikonLiveViewHeader*) dataPtr);
        fixBytesOrder(&nikonLvHeader);
        // TODO do something with this liveview header !


        dataPtr = dataPtr + 384;
        QByteArray preview(dataPtr, dataSize - 384);

        cameraPreview = CameraPreview(preview, "application/jpeg");

        return true;
    }
    else
    {
        return GPCamera::capturePreview(cameraPreview);
    }
}





// -------------------------- Custom read methods

bool GPNikonCamera::gpReadCaptureMode()
{
    int ret;
    QString currentCaptureMode;
    ret = gpGetRadioWidgetValue(captureModeWidgetName(), currentCaptureMode);
    if (ret == GP_OK)
    {
        if (currentCaptureMode == "1")
        {
            m_captureMode = CaptureModeVideo;
        } else {
            m_captureMode = CaptureModePhoto;
        }
    } else {
        m_captureMode = CaptureModePhoto;
        return false;
    }

    return true;
}

bool GPNikonCamera::gpReadIsoAuto()
{
    QString currentIsoAuto;
    int ret = gpGetRadioWidgetValue(isoAutoWidgetName(), currentIsoAuto);
    if (ret == GP_OK)
    {
        if (currentIsoAuto == "On") {
            m_cameraIsoAuto = true;
        } else if (currentIsoAuto == "Off") {
            m_cameraIsoAuto = false;
        } else {
            return false;
        }

        return true;
    }
    else
    {
        m_cameraIsoAuto = false;
        return false;
    }
}


bool GPNikonCamera::gpReadLvZoomRatio()
{
    int ret = gpExtractWidgetChoices(lvZoomRatioWidgetName(), m_lvZoomRatios);
    if (ret < GP_OK)
    {
        m_lvZoomRatio = -1;
        return false;
    }


    QString currentLvZoomRatio;
    ret = gpGetRadioWidgetValue(lvZoomRatioWidgetName(), currentLvZoomRatio);
    if (ret == GP_OK)
    {
        m_lvZoomRatio = m_lvZoomRatios.indexOf(currentLvZoomRatio);
    } else {
        m_lvZoomRatio = -1;
        return false;
    }

    return true;
}

bool GPNikonCamera::gpReadExposurePreview()
{
    if (m_cameraModel == "D800")
    {
        QString currentExposurePreview;
        int ret = gpGetRadioWidgetValue(exposurePreviewWidgetName(), currentExposurePreview);
        if (ret == GP_OK)
        {
            if (currentExposurePreview == "1") {
                m_exposurePreview = true;
            } else if (currentExposurePreview == "0") {
                m_exposurePreview = false;
            }

            return true;
        }
        else
        {
            m_exposurePreview = false;
            return false;
        }
    } else {
        return false;
    }
}

// -------------------------- Get / Set

// Capture mode

bool GPNikonCamera::setCaptureMode(CaptureMode captureMode)
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

bool GPNikonCamera::setIsoAuto(bool isoAuto)
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


bool GPNikonCamera::increaseLvZoomRatio()
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

bool GPNikonCamera::decreaseLvZoomRatio()
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

bool GPNikonCamera::setExposurePreview(bool exposurePreview)
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


// Custom widget names

QString GPNikonCamera::captureModeWidgetName()
{
    return "d1a6";
}

QString GPNikonCamera::viewfinderWidgetName()
{
    return "viewfinder"; // canon eosviewfinder
}

QString GPNikonCamera::apertureWidgetName()
{
    // TODO refactoring
    if (m_model == "Nikon DSC D800")
    {
        if (m_viewfinder && m_captureMode == CaptureModeVideo)
        {
            return "movief-number"; // d1a9
        }
        else
        {
            return "f-number"; // 5007
        }
    } else {
        return "f-number";
    }
}

QString GPNikonCamera::shutterSpeedWidgetName()
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

QString GPNikonCamera::isoWidgetName()
{
    if (m_viewfinder && m_captureMode == CaptureModeVideo)
    {
        return "movieiso"; // d1aa
    } else {
        return "iso"; // 500f
    }
}

QString GPNikonCamera::isoAutoWidgetName()
{
    return "autoiso"; // d054
}

QString GPCamera::afModeWidgetName()
{
    return "liveviewaffocus"; // d061
}

QString GPCamera::afAreaWidgetName()
{
    return "changeafarea"; // 9205
}

QString GPCamera::afAtWidgetName()
{
    return "liveviewafmode"; // d05d
}

QString GPNikonCamera::exposureModeWidgetName()
{
    return "expprogram"; // 500e canon : autoexposuremode
}


QString GPNikonCamera::liveviewSelectorWidgetName()
{
    return "d1a6";
}

QString GPNikonCamera::lvZoomRatioWidgetName()
{
    return "d1a3";
}

QString GPNikonCamera::exposurePreviewWidgetName()
{
    return "d1a5";
}

QString GPCamera::stillCaptureModeWidgetName()
{
    return "capturemode"; // canon drivemode
}

QString GPCamera::recordingMediaWidgetName()
{
    return "recordingmedia"; // d10b
}


QString GPNikonCamera::exposureCompensationWidgetName()
{
    if (m_viewfinder && m_captureMode == CaptureModeVideo)
    {
        return "d1ab";
    } else {
        return "exposurecompensation"; // exposurecompensation2 // 5010
    }
}



// -------------------- Utils

static uint32_t fixBytesOrder(uint32_t val)
{
    return  ((0xff000000 & val) >> 24) |
            ((0x00ff0000 & val) >> 8) |
            ((0x0000ff00 & val) << 8) |
            ((0x000000ff & val) << 24);
}

static uint16_t fixBytesOrder(uint16_t val)
{
    return (val >> 8) | (val << 8);
}

static uint8_t fixBytesOrder(uint8_t val)
{
    return val;
}


static void fixBytesOrder(NikonLiveViewHeader* lvHeader)
{


    lvHeader->jpg_width = fixBytesOrder(lvHeader->jpg_width);
    lvHeader->jpg_height = fixBytesOrder(lvHeader->jpg_height);

    lvHeader->width = fixBytesOrder(lvHeader->width);
    lvHeader->height = fixBytesOrder(lvHeader->height);

    lvHeader->display_width = fixBytesOrder(lvHeader->display_width);
    lvHeader->display_height = fixBytesOrder(lvHeader->display_height);

    lvHeader->center_x = fixBytesOrder(lvHeader->center_x);
    lvHeader->center_y = fixBytesOrder(lvHeader->center_y);

    lvHeader->af_width = fixBytesOrder(lvHeader->af_width);
    lvHeader->af_height = fixBytesOrder(lvHeader->af_height);

    lvHeader->af_center_x = fixBytesOrder(lvHeader->af_center_x);
    lvHeader->af_center_y = fixBytesOrder(lvHeader->af_center_y);

    lvHeader->reserved_01 = fixBytesOrder(lvHeader->reserved_01);

    lvHeader->focus_area = fixBytesOrder(lvHeader->focus_area);
    lvHeader->rotation_direction = fixBytesOrder(lvHeader->rotation_direction);

    lvHeader->focus_driving_status = fixBytesOrder(lvHeader->focus_driving_status);

    lvHeader->reserved_02 = fixBytesOrder(lvHeader->reserved_02);
    lvHeader->reserved_03 = fixBytesOrder(lvHeader->reserved_03);
    lvHeader->reserved_04 = fixBytesOrder(lvHeader->reserved_04);

    lvHeader->countdown_time = fixBytesOrder(lvHeader->countdown_time);

    lvHeader->focusing_judgement_result = fixBytesOrder(lvHeader->focusing_judgement_result);
    lvHeader->af_driving_enabled_status = fixBytesOrder(lvHeader->af_driving_enabled_status);

    lvHeader->reserved_05 = fixBytesOrder(lvHeader->reserved_05);

    lvHeader->rolling = fixBytesOrder(lvHeader->rolling);
    lvHeader->pitching = fixBytesOrder(lvHeader->pitching);
    lvHeader->yawing = fixBytesOrder(lvHeader->yawing);

    lvHeader->remaining_time_movie_recording = fixBytesOrder(lvHeader->remaining_time_movie_recording);
    lvHeader->movie_recording = fixBytesOrder(lvHeader->movie_recording);

    lvHeader->af_mode_face_detection_status = fixBytesOrder(lvHeader->af_mode_face_detection_status);
    lvHeader->nb_detected_faces = fixBytesOrder(lvHeader->nb_detected_faces);

    lvHeader->af_area = fixBytesOrder(lvHeader->af_area);


    lvHeader->detected_face_width_01 = fixBytesOrder(lvHeader->detected_face_width_01);
    lvHeader->detected_face_height_01 = fixBytesOrder(lvHeader->detected_face_height_01);
    lvHeader->detected_face_center_x_01 = fixBytesOrder(lvHeader->detected_face_center_x_01);
    lvHeader->detected_face_center_y_01 = fixBytesOrder(lvHeader->detected_face_center_y_01);

    lvHeader->detected_face_width_02 = fixBytesOrder(lvHeader->detected_face_width_02);
    lvHeader->detected_face_height_02 = fixBytesOrder(lvHeader->detected_face_height_02);
    lvHeader->detected_face_center_x_02 = fixBytesOrder(lvHeader->detected_face_center_x_02);
    lvHeader->detected_face_center_y_02 = fixBytesOrder(lvHeader->detected_face_center_y_02);

    lvHeader->detected_face_width_03 = fixBytesOrder(lvHeader->detected_face_width_03);
    lvHeader->detected_face_height_03 = fixBytesOrder(lvHeader->detected_face_height_03);
    lvHeader->detected_face_center_x_03 = fixBytesOrder(lvHeader->detected_face_center_x_03);
    lvHeader->detected_face_center_y_03 = fixBytesOrder(lvHeader->detected_face_center_y_03);

    lvHeader->detected_face_width_04 = fixBytesOrder(lvHeader->detected_face_width_04);
    lvHeader->detected_face_height_04 = fixBytesOrder(lvHeader->detected_face_height_04);
    lvHeader->detected_face_center_x_04 = fixBytesOrder(lvHeader->detected_face_center_x_04);
    lvHeader->detected_face_center_y_04 = fixBytesOrder(lvHeader->detected_face_center_y_04);

    lvHeader->detected_face_width_05 = fixBytesOrder(lvHeader->detected_face_width_05);
    lvHeader->detected_face_height_05 = fixBytesOrder(lvHeader->detected_face_height_05);
    lvHeader->detected_face_center_x_05 = fixBytesOrder(lvHeader->detected_face_center_x_05);
    lvHeader->detected_face_center_y_05 = fixBytesOrder(lvHeader->detected_face_center_y_05);

    lvHeader->detected_face_width_06 = fixBytesOrder(lvHeader->detected_face_width_06);
    lvHeader->detected_face_height_06 = fixBytesOrder(lvHeader->detected_face_height_06);
    lvHeader->detected_face_center_x_06 = fixBytesOrder(lvHeader->detected_face_center_x_06);
    lvHeader->detected_face_center_y_06 = fixBytesOrder(lvHeader->detected_face_center_y_06);

    lvHeader->detected_face_width_07 = fixBytesOrder(lvHeader->detected_face_width_07);
    lvHeader->detected_face_height_07 = fixBytesOrder(lvHeader->detected_face_height_07);
    lvHeader->detected_face_center_x_07 = fixBytesOrder(lvHeader->detected_face_center_x_07);
    lvHeader->detected_face_center_y_07 = fixBytesOrder(lvHeader->detected_face_center_y_07);

    lvHeader->detected_face_width_08 = fixBytesOrder(lvHeader->detected_face_width_08);
    lvHeader->detected_face_height_08 = fixBytesOrder(lvHeader->detected_face_height_08);
    lvHeader->detected_face_center_x_08 = fixBytesOrder(lvHeader->detected_face_center_x_08);
    lvHeader->detected_face_center_y_08 = fixBytesOrder(lvHeader->detected_face_center_y_08);

    lvHeader->detected_face_width_09 = fixBytesOrder(lvHeader->detected_face_width_09);
    lvHeader->detected_face_height_09 = fixBytesOrder(lvHeader->detected_face_height_09);
    lvHeader->detected_face_center_x_09 = fixBytesOrder(lvHeader->detected_face_center_x_09);
    lvHeader->detected_face_center_y_09 = fixBytesOrder(lvHeader->detected_face_center_y_09);

    lvHeader->detected_face_width_10 = fixBytesOrder(lvHeader->detected_face_width_10);
    lvHeader->detected_face_height_10 = fixBytesOrder(lvHeader->detected_face_height_10);
    lvHeader->detected_face_center_x_10 = fixBytesOrder(lvHeader->detected_face_center_x_10);
    lvHeader->detected_face_center_y_10 = fixBytesOrder(lvHeader->detected_face_center_y_10);

    lvHeader->detected_face_width_11 = fixBytesOrder(lvHeader->detected_face_width_11);
    lvHeader->detected_face_height_11 = fixBytesOrder(lvHeader->detected_face_height_11);
    lvHeader->detected_face_center_x_11 = fixBytesOrder(lvHeader->detected_face_center_x_11);
    lvHeader->detected_face_center_y_11 = fixBytesOrder(lvHeader->detected_face_center_y_11);

    lvHeader->detected_face_width_12 = fixBytesOrder(lvHeader->detected_face_width_12);
    lvHeader->detected_face_height_12 = fixBytesOrder(lvHeader->detected_face_height_12);
    lvHeader->detected_face_center_x_12 = fixBytesOrder(lvHeader->detected_face_center_x_12);
    lvHeader->detected_face_center_y_12 = fixBytesOrder(lvHeader->detected_face_center_y_12);

    lvHeader->detected_face_width_13 = fixBytesOrder(lvHeader->detected_face_width_13);
    lvHeader->detected_face_height_13 = fixBytesOrder(lvHeader->detected_face_height_13);
    lvHeader->detected_face_center_x_13 = fixBytesOrder(lvHeader->detected_face_center_x_13);
    lvHeader->detected_face_center_y_13 = fixBytesOrder(lvHeader->detected_face_center_y_13);

    lvHeader->detected_face_width_14 = fixBytesOrder(lvHeader->detected_face_width_14);
    lvHeader->detected_face_height_14 = fixBytesOrder(lvHeader->detected_face_height_14);
    lvHeader->detected_face_center_x_14 = fixBytesOrder(lvHeader->detected_face_center_x_14);
    lvHeader->detected_face_center_y_14 = fixBytesOrder(lvHeader->detected_face_center_y_14);

    lvHeader->detected_face_width_15 = fixBytesOrder(lvHeader->detected_face_width_15);
    lvHeader->detected_face_height_15 = fixBytesOrder(lvHeader->detected_face_height_15);
    lvHeader->detected_face_center_x_15 = fixBytesOrder(lvHeader->detected_face_center_x_15);
    lvHeader->detected_face_center_y_15 = fixBytesOrder(lvHeader->detected_face_center_y_15);

    lvHeader->detected_face_width_16 = fixBytesOrder(lvHeader->detected_face_width_16);
    lvHeader->detected_face_height_16 = fixBytesOrder(lvHeader->detected_face_height_16);
    lvHeader->detected_face_center_x_16 = fixBytesOrder(lvHeader->detected_face_center_x_16);
    lvHeader->detected_face_center_y_16 = fixBytesOrder(lvHeader->detected_face_center_y_16);

    lvHeader->detected_face_width_17 = fixBytesOrder(lvHeader->detected_face_width_17);
    lvHeader->detected_face_height_17 = fixBytesOrder(lvHeader->detected_face_height_17);
    lvHeader->detected_face_center_x_17 = fixBytesOrder(lvHeader->detected_face_center_x_17);
    lvHeader->detected_face_center_y_17 = fixBytesOrder(lvHeader->detected_face_center_y_17);

    lvHeader->detected_face_width_18 = fixBytesOrder(lvHeader->detected_face_width_18);
    lvHeader->detected_face_height_18 = fixBytesOrder(lvHeader->detected_face_height_18);
    lvHeader->detected_face_center_x_18 = fixBytesOrder(lvHeader->detected_face_center_x_18);
    lvHeader->detected_face_center_y_18 = fixBytesOrder(lvHeader->detected_face_center_y_18);

    lvHeader->detected_face_width_19 = fixBytesOrder(lvHeader->detected_face_width_19);
    lvHeader->detected_face_height_19 = fixBytesOrder(lvHeader->detected_face_height_19);
    lvHeader->detected_face_center_x_19 = fixBytesOrder(lvHeader->detected_face_center_x_19);
    lvHeader->detected_face_center_y_19 = fixBytesOrder(lvHeader->detected_face_center_y_19);

    lvHeader->detected_face_width_20 = fixBytesOrder(lvHeader->detected_face_width_20);
    lvHeader->detected_face_height_20 = fixBytesOrder(lvHeader->detected_face_height_20);
    lvHeader->detected_face_center_x_20 = fixBytesOrder(lvHeader->detected_face_center_x_20);
    lvHeader->detected_face_center_y_20 = fixBytesOrder(lvHeader->detected_face_center_y_20);

    lvHeader->detected_face_width_21 = fixBytesOrder(lvHeader->detected_face_width_21);
    lvHeader->detected_face_height_21 = fixBytesOrder(lvHeader->detected_face_height_21);
    lvHeader->detected_face_center_x_21 = fixBytesOrder(lvHeader->detected_face_center_x_21);
    lvHeader->detected_face_center_y_21 = fixBytesOrder(lvHeader->detected_face_center_y_21);

    lvHeader->detected_face_width_22 = fixBytesOrder(lvHeader->detected_face_width_22);
    lvHeader->detected_face_height_22 = fixBytesOrder(lvHeader->detected_face_height_22);
    lvHeader->detected_face_center_x_22 = fixBytesOrder(lvHeader->detected_face_center_x_22);
    lvHeader->detected_face_center_y_22 = fixBytesOrder(lvHeader->detected_face_center_y_22);

    lvHeader->detected_face_width_23 = fixBytesOrder(lvHeader->detected_face_width_23);
    lvHeader->detected_face_height_23 = fixBytesOrder(lvHeader->detected_face_height_23);
    lvHeader->detected_face_center_x_23 = fixBytesOrder(lvHeader->detected_face_center_x_23);
    lvHeader->detected_face_center_y_23 = fixBytesOrder(lvHeader->detected_face_center_y_23);

    lvHeader->detected_face_width_24 = fixBytesOrder(lvHeader->detected_face_width_24);
    lvHeader->detected_face_height_24 = fixBytesOrder(lvHeader->detected_face_height_24);
    lvHeader->detected_face_center_x_24 = fixBytesOrder(lvHeader->detected_face_center_x_24);
    lvHeader->detected_face_center_y_24 = fixBytesOrder(lvHeader->detected_face_center_y_24);

    lvHeader->detected_face_width_25 = fixBytesOrder(lvHeader->detected_face_width_25);
    lvHeader->detected_face_height_25 = fixBytesOrder(lvHeader->detected_face_height_25);
    lvHeader->detected_face_center_x_25 = fixBytesOrder(lvHeader->detected_face_center_x_25);
    lvHeader->detected_face_center_y_25 = fixBytesOrder(lvHeader->detected_face_center_y_25);

    lvHeader->detected_face_width_26 = fixBytesOrder(lvHeader->detected_face_width_26);
    lvHeader->detected_face_height_26 = fixBytesOrder(lvHeader->detected_face_height_26);
    lvHeader->detected_face_center_x_26 = fixBytesOrder(lvHeader->detected_face_center_x_26);
    lvHeader->detected_face_center_y_26 = fixBytesOrder(lvHeader->detected_face_center_y_26);

    lvHeader->detected_face_width_27 = fixBytesOrder(lvHeader->detected_face_width_27);
    lvHeader->detected_face_height_27 = fixBytesOrder(lvHeader->detected_face_height_27);
    lvHeader->detected_face_center_x_27 = fixBytesOrder(lvHeader->detected_face_center_x_27);
    lvHeader->detected_face_center_y_27 = fixBytesOrder(lvHeader->detected_face_center_y_27);

    lvHeader->detected_face_width_28 = fixBytesOrder(lvHeader->detected_face_width_28);
    lvHeader->detected_face_height_28 = fixBytesOrder(lvHeader->detected_face_height_28);
    lvHeader->detected_face_center_x_28 = fixBytesOrder(lvHeader->detected_face_center_x_28);
    lvHeader->detected_face_center_y_28 = fixBytesOrder(lvHeader->detected_face_center_y_28);

    lvHeader->detected_face_width_29 = fixBytesOrder(lvHeader->detected_face_width_29);
    lvHeader->detected_face_height_29 = fixBytesOrder(lvHeader->detected_face_height_29);
    lvHeader->detected_face_center_x_29 = fixBytesOrder(lvHeader->detected_face_center_x_29);
    lvHeader->detected_face_center_y_29 = fixBytesOrder(lvHeader->detected_face_center_y_29);

    lvHeader->detected_face_width_30 = fixBytesOrder(lvHeader->detected_face_width_30);
    lvHeader->detected_face_height_30 = fixBytesOrder(lvHeader->detected_face_height_30);
    lvHeader->detected_face_center_x_30 = fixBytesOrder(lvHeader->detected_face_center_x_30);
    lvHeader->detected_face_center_y_30 = fixBytesOrder(lvHeader->detected_face_center_y_30);

    lvHeader->detected_face_width_31 = fixBytesOrder(lvHeader->detected_face_width_31);
    lvHeader->detected_face_height_31 = fixBytesOrder(lvHeader->detected_face_height_31);
    lvHeader->detected_face_center_x_31 = fixBytesOrder(lvHeader->detected_face_center_x_31);
    lvHeader->detected_face_center_y_31 = fixBytesOrder(lvHeader->detected_face_center_y_31);

    lvHeader->detected_face_width_32 = fixBytesOrder(lvHeader->detected_face_width_32);
    lvHeader->detected_face_height_32 = fixBytesOrder(lvHeader->detected_face_height_32);
    lvHeader->detected_face_center_x_32 = fixBytesOrder(lvHeader->detected_face_center_x_32);
    lvHeader->detected_face_center_y_32 = fixBytesOrder(lvHeader->detected_face_center_y_32);

    lvHeader->detected_face_width_33 = fixBytesOrder(lvHeader->detected_face_width_33);
    lvHeader->detected_face_height_33 = fixBytesOrder(lvHeader->detected_face_height_33);
    lvHeader->detected_face_center_x_33 = fixBytesOrder(lvHeader->detected_face_center_x_33);
    lvHeader->detected_face_center_y_33 = fixBytesOrder(lvHeader->detected_face_center_y_33);

    lvHeader->detected_face_width_34 = fixBytesOrder(lvHeader->detected_face_width_34);
    lvHeader->detected_face_height_34 = fixBytesOrder(lvHeader->detected_face_height_34);
    lvHeader->detected_face_center_x_34 = fixBytesOrder(lvHeader->detected_face_center_x_34);
    lvHeader->detected_face_center_y_34 = fixBytesOrder(lvHeader->detected_face_center_y_34);

    lvHeader->detected_face_width_35 = fixBytesOrder(lvHeader->detected_face_width_35);
    lvHeader->detected_face_height_35 = fixBytesOrder(lvHeader->detected_face_height_35);
    lvHeader->detected_face_center_x_35 = fixBytesOrder(lvHeader->detected_face_center_x_35);
    lvHeader->detected_face_center_y_35 = fixBytesOrder(lvHeader->detected_face_center_y_35);

    lvHeader->sound_indicator_peak_l = fixBytesOrder(lvHeader->sound_indicator_peak_l);
    lvHeader->sound_indicator_peak_r = fixBytesOrder(lvHeader->sound_indicator_peak_r);

    lvHeader->sound_indicator_current_l = fixBytesOrder(lvHeader->sound_indicator_current_l);
    lvHeader->sound_indicator_current_r = fixBytesOrder(lvHeader->sound_indicator_current_r);

    lvHeader->reserved_06 = fixBytesOrder(lvHeader->reserved_06);

    lvHeader->use_white_balance = fixBytesOrder(lvHeader->use_white_balance);

    lvHeader->reserved_07 = fixBytesOrder(lvHeader->reserved_07);
    lvHeader->reserved_08 = fixBytesOrder(lvHeader->reserved_08);
    lvHeader->reserved_09 = fixBytesOrder(lvHeader->reserved_09);
    lvHeader->reserved_10 = fixBytesOrder(lvHeader->reserved_10);
    lvHeader->reserved_11 = fixBytesOrder(lvHeader->reserved_11);
    lvHeader->reserved_12 = fixBytesOrder(lvHeader->reserved_12);
    lvHeader->reserved_13 = fixBytesOrder(lvHeader->reserved_13);
    lvHeader->reserved_14 = fixBytesOrder(lvHeader->reserved_14);

    lvHeader->reserved_15 = fixBytesOrder(lvHeader->reserved_15);
}

