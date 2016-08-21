#ifndef GPNIKONCAMERA_H
#define GPNIKONCAMERA_H

#include "../gpcamera.h"

#include <QObject>

#define CAMERA_NIKON_D800 "D800"

namespace hpis {

class GPNikonCamera : public GPCamera
{
    Q_OBJECT
public:
    explicit GPNikonCamera(QString cameraModel, QString cameraPort, QObject *parent = 0);
    virtual QSet<CameraCapability> capabilities();
    bool capturePreview(CameraPreview& cameraPreview);

protected:
    // ---------- Read
    virtual bool gpReadCaptureMode();
    virtual bool gpReadIsoAuto();
    virtual bool gpReadLvZoomRatio();
    virtual bool gpReadExposurePreview();

    // ---------- Get / Set
    virtual bool setCaptureMode(CaptureMode captureMode);
    virtual bool setIsoAuto(bool isoAuto);

    virtual bool increaseLvZoomRatio();
    virtual bool decreaseLvZoomRatio();

    // Exposure preview
    virtual bool setExposurePreview(bool exposurePreview);

    // ---------- Widget names
    virtual QString viewfinderWidgetName();
    virtual QString apertureWidgetName();
    virtual QString shutterSpeedWidgetName();
    virtual QString isoWidgetName();

    virtual QString exposurePreviewWidgetName();

    virtual QString exposureModeWidgetName();
    virtual QString captureModeWidgetName();
    virtual QString liveviewSelectorWidgetName();
    virtual QString focusModeWidgetName();
    virtual QString focusMeteringWidgetName();
    virtual QString lvZoomRatioWidgetName();
    virtual QString isoAutoWidgetName();

    virtual QString exposureCompensationWidgetName();
signals:

public slots:
};


struct NikonLiveViewHeader {
    uint16_t jpg_width;
    uint16_t jpg_height;

    uint16_t width;
    uint16_t height;

    uint16_t display_width;
    uint16_t display_height;

    uint16_t center_x;
    uint16_t center_y;

    uint16_t af_width;
    uint16_t af_height;

    uint16_t af_center_x;
    uint16_t af_center_y;

    uint32_t reserved_01;

    uint8_t focus_area;
    uint8_t rotation_direction;

    uint8_t focus_driving_status;

    uint8_t reserved_02;
    uint32_t reserved_03;
    uint16_t reserved_04;

    uint16_t countdown_time;

    uint8_t focusing_judgement_result;
    uint8_t af_driving_enabled_status;

    uint16_t reserved_05;

    uint32_t rolling;
    uint32_t pitching;
    uint32_t yawing;

    uint32_t remaining_time_movie_recording;
    uint8_t movie_recording;

    uint8_t af_mode_face_detection_status;
    uint8_t nb_detected_faces;

    // fixed to 0 for D800/D800E
    uint8_t af_area;


    // Faces

    uint8_t detected_face_width_01;
    uint8_t detected_face_height_01;
    uint8_t detected_face_center_x_01;
    uint8_t detected_face_center_y_01;

    uint8_t detected_face_width_02;
    uint8_t detected_face_height_02;
    uint8_t detected_face_center_x_02;
    uint8_t detected_face_center_y_02;

    uint8_t detected_face_width_03;
    uint8_t detected_face_height_03;
    uint8_t detected_face_center_x_03;
    uint8_t detected_face_center_y_03;

    uint8_t detected_face_width_04;
    uint8_t detected_face_height_04;
    uint8_t detected_face_center_x_04;
    uint8_t detected_face_center_y_04;

    uint8_t detected_face_width_05;
    uint8_t detected_face_height_05;
    uint8_t detected_face_center_x_05;
    uint8_t detected_face_center_y_05;

    uint8_t detected_face_width_06;
    uint8_t detected_face_height_06;
    uint8_t detected_face_center_x_06;
    uint8_t detected_face_center_y_06;

    uint8_t detected_face_width_07;
    uint8_t detected_face_height_07;
    uint8_t detected_face_center_x_07;
    uint8_t detected_face_center_y_07;

    uint8_t detected_face_width_08;
    uint8_t detected_face_height_08;
    uint8_t detected_face_center_x_08;
    uint8_t detected_face_center_y_08;

    uint8_t detected_face_width_09;
    uint8_t detected_face_height_09;
    uint8_t detected_face_center_x_09;
    uint8_t detected_face_center_y_09;

    uint8_t detected_face_width_10;
    uint8_t detected_face_height_10;
    uint8_t detected_face_center_x_10;
    uint8_t detected_face_center_y_10;

    uint8_t detected_face_width_11;
    uint8_t detected_face_height_11;
    uint8_t detected_face_center_x_11;
    uint8_t detected_face_center_y_11;

    uint8_t detected_face_width_12;
    uint8_t detected_face_height_12;
    uint8_t detected_face_center_x_12;
    uint8_t detected_face_center_y_12;

    uint8_t detected_face_width_13;
    uint8_t detected_face_height_13;
    uint8_t detected_face_center_x_13;
    uint8_t detected_face_center_y_13;

    uint8_t detected_face_width_14;
    uint8_t detected_face_height_14;
    uint8_t detected_face_center_x_14;
    uint8_t detected_face_center_y_14;

    uint8_t detected_face_width_15;
    uint8_t detected_face_height_15;
    uint8_t detected_face_center_x_15;
    uint8_t detected_face_center_y_15;

    uint8_t detected_face_width_16;
    uint8_t detected_face_height_16;
    uint8_t detected_face_center_x_16;
    uint8_t detected_face_center_y_16;

    uint8_t detected_face_width_17;
    uint8_t detected_face_height_17;
    uint8_t detected_face_center_x_17;
    uint8_t detected_face_center_y_17;

    uint8_t detected_face_width_18;
    uint8_t detected_face_height_18;
    uint8_t detected_face_center_x_18;
    uint8_t detected_face_center_y_18;

    uint8_t detected_face_width_19;
    uint8_t detected_face_height_19;
    uint8_t detected_face_center_x_19;
    uint8_t detected_face_center_y_19;

    uint8_t detected_face_width_20;
    uint8_t detected_face_height_20;
    uint8_t detected_face_center_x_20;
    uint8_t detected_face_center_y_20;

    uint8_t detected_face_width_21;
    uint8_t detected_face_height_21;
    uint8_t detected_face_center_x_21;
    uint8_t detected_face_center_y_21;

    uint8_t detected_face_width_22;
    uint8_t detected_face_height_22;
    uint8_t detected_face_center_x_22;
    uint8_t detected_face_center_y_22;

    uint8_t detected_face_width_23;
    uint8_t detected_face_height_23;
    uint8_t detected_face_center_x_23;
    uint8_t detected_face_center_y_23;

    uint8_t detected_face_width_24;
    uint8_t detected_face_height_24;
    uint8_t detected_face_center_x_24;
    uint8_t detected_face_center_y_24;

    uint8_t detected_face_width_25;
    uint8_t detected_face_height_25;
    uint8_t detected_face_center_x_25;
    uint8_t detected_face_center_y_25;

    uint8_t detected_face_width_26;
    uint8_t detected_face_height_26;
    uint8_t detected_face_center_x_26;
    uint8_t detected_face_center_y_26;

    uint8_t detected_face_width_27;
    uint8_t detected_face_height_27;
    uint8_t detected_face_center_x_27;
    uint8_t detected_face_center_y_27;

    uint8_t detected_face_width_28;
    uint8_t detected_face_height_28;
    uint8_t detected_face_center_x_28;
    uint8_t detected_face_center_y_28;

    uint8_t detected_face_width_29;
    uint8_t detected_face_height_29;
    uint8_t detected_face_center_x_29;
    uint8_t detected_face_center_y_29;

    uint8_t detected_face_width_30;
    uint8_t detected_face_height_30;
    uint8_t detected_face_center_x_30;
    uint8_t detected_face_center_y_30;

    uint8_t detected_face_width_31;
    uint8_t detected_face_height_31;
    uint8_t detected_face_center_x_31;
    uint8_t detected_face_center_y_31;

    uint8_t detected_face_width_32;
    uint8_t detected_face_height_32;
    uint8_t detected_face_center_x_32;
    uint8_t detected_face_center_y_32;

    uint8_t detected_face_width_33;
    uint8_t detected_face_height_33;
    uint8_t detected_face_center_x_33;
    uint8_t detected_face_center_y_33;

    uint8_t detected_face_width_34;
    uint8_t detected_face_height_34;
    uint8_t detected_face_center_x_34;
    uint8_t detected_face_center_y_34;

    uint8_t detected_face_width_35;
    uint8_t detected_face_height_35;
    uint8_t detected_face_center_x_35;
    uint8_t detected_face_center_y_35;

    // Sound
    uint8_t sound_indicator_peak_l;
    uint8_t sound_indicator_peak_r;

    uint8_t sound_indicator_current_l;
    uint8_t sound_indicator_current_r;

    uint8_t reserved_06;

    uint8_t use_white_balance;

    uint32_t reserved_07;
    uint32_t reserved_08;
    uint32_t reserved_09;
    uint32_t reserved_10;
    uint32_t reserved_11;
    uint32_t reserved_12;
    uint32_t reserved_13;
    uint32_t reserved_14;

    uint16_t reserved_15;
};


}
#endif // GPNIKONCAMERA_H
