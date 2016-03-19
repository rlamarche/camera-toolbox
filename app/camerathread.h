#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include "camera.h"
#include "camerastatus.h"

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QImage>
#include <QMap>
#include <QList>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>

#define HPIS_CONFIG_KEY_VIEWFINDER "viewfinder"
#define HPIS_CONFIG_KEY_APERTURE "d1a9"
//#define HPIS_CONFIG_KEY_APERTURE "5007"
//#define HPIS_CONFIG_KEY_APERTURE "f-number"
#define HPIS_CONFIG_KEY_SHUTTERSPEED "d1a8"
//#define HPIS_CONFIG_KEY_SHUTTERSPEED "500d"
//#define HPIS_CONFIG_KEY_SHUTTERSPEED "shutterspeed"
//#define HPIS_CONFIG_KEY_SHUTTERSPEED "shutterspeed2"
#define HPIS_CONFIG_KEY_EXPPROGRAM "expprogram"

#define HPIS_CONFIG_KEY_START_MOVIE "movie"
#define HPIS_CONFIG_KEY_STOP_MOVIE "920b"


class DecoderThread;


class CameraThread : public QThread
{
    friend class DecoderThread;

    Q_OBJECT
public:
    enum CommandType {
        CommandStartLiveview,
        CommandStopLiveview,
        CommandIncreaseAperture,
        CommandDecreaseAperture,
        CommandIncreaseShutterSpeed,
        CommandDecreaseShutterSpeed,
        CommandExposureModePlus,
        CommandExposureModeMinus,
        CommandIncreaseLvZoomRatio,
        CommandDecreaseLvZoomRatio,
        CommandChangeAfArea,
        CommandPhotoMode,
        CommandVideoMode,

        CommandToggleLiveview,
        CommandStartMovie,
        CommandStopMovie,
        CommandCapturePhoto
    };

    class Command {
    public:
        Command();
        Command(CommandType commandType);
        static Command changeAfArea(int x, int y);

        CommandType type();
        int x();
        int y();
    private:
        CommandType m_commandType;
        int m_x;
        int m_y;
    };

    explicit CameraThread(hpis::Camera* camera, QObject *parent = 0);
    void stop();
    void executeCommand(Command executeCommand);





protected:
    // Thread main loop
    void run();

    // Init & Shutdown
    void init();
    void shutdown();
    /*
    bool lookupCamera();
    int lookupWidgets(CameraWidget* widget);
*/

    // Widget utils
    //QList<QString> extractWidgetChoices(CameraWidget* widget);


    // Camera control
    /*void extractCameraCapabilities();
    void refreshCameraSettings();*/
    void doCommand(Command executeCommand);


    void doCapturePreview();

    /*
    int setToggleWidget(QString widgetName, int toggleValue);
    int setRangeWidget(QString widgetName, float rangeValue);
    int setRadioWidget(QString widgetName, const char* radioValue);
    int updateConfig();
*/

    // Image decoding
    void previewDecoded(QImage image);
    /*
    QImage decodeImage(const char *data, unsigned long int size);
    QImage decodeImageTurbo(const char *data, unsigned long int size);*/
private:
    // GPhoto context
    /*
    GPContext*              m_context;
    CameraAbilitiesList*    m_abilitiesList;
    GPPortInfoList*         m_portInfoList;
    Camera*                 m_camera;
    CameraWidget*           m_cameraWindow;

    QMap<QString, CameraWidget*> m_widgets;
*/

    hpis::Camera* m_camera;

    // Thread control
    bool m_stop;
    bool m_liveview;
    bool m_recording;

    int refreshTimeoutMs;


    // Thread synchronization
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Thread commands
    QQueue<Command> m_commandQueue;

    // Decoder
    DecoderThread* m_decoderThread;


    // Camera infos
    //CameraAbilities m_cameraAbilities;
    QString m_cameraModel;
    QString m_cameraPort;
/*
    QList<QString> m_cameraApertures;
    int m_cameraAperture;

    QList<QString> m_cameraShutterSpeeds;
    int m_cameraShutterSpeed;
*/
signals:
    void previewAvailable(QPixmap preview);
    void imageAvailable(QImage preview);
    void cameraStatus(hpis::CameraStatus cameraStatus);
public slots:
    void capturePreview();
};

#endif // CAMERATHREAD_H
