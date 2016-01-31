#ifndef LIVEVIEWTHREAD_H
#define LIVEVIEWTHREAD_H

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
#define HPIS_CONFIG_KEY_APERTURE "f-number"

class DecoderThread;

class CameraThread : public QThread
{
    friend class DecoderThread;

    Q_OBJECT
public:
    enum Command {
        CommandStartLiveview,
        CommandStopLiveview,
        CommandToggleLiveview,
        CommandIncreaseAperture,
        CommandDecreaseAperture
    };

    explicit CameraThread(QObject *parent = 0);
    void stop();
    void executeCommand(Command executeCommand);



protected:
    // Thread main loop
    void run();

    // Init & Shutdown
    void init();
    void shutdown();
    bool lookupCamera();
    int lookupWidgets(CameraWidget* widget);


    // Widget utils
    QList<QString> extractWidgetChoices(CameraWidget* widget);


    // Camera control
    void extractCameraCapabilities();
    void refreshCameraSettings();
    void doCommand(Command executeCommand);


    void doCapturePreview();
    int setToggleWidget(QString widgetName, int toggleValue);
    int setRangeWidget(QString widgetName, float rangeValue);
    int updateConfig();


    // Image decoding
    void previewDecoded(QImage image);
    QImage decodeImage(const char *data, unsigned long int size);
    QImage decodeImageTurbo(const char *data, unsigned long int size);
private:
    // GPhoto context
    GPContext*              m_context;
    CameraAbilitiesList*    m_abilitiesList;
    GPPortInfoList*         m_portInfoList;
    Camera*                 m_camera;
    CameraWidget*           m_cameraWindow;

    QMap<QString, CameraWidget*> m_widgets;

    // Thread control
    bool m_stop;
    bool m_liveview;

    // Thread synchronization
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Thread commands
    QQueue<Command> m_commandQueue;

    // Decoder
    DecoderThread* m_decoderThread;

    // Camera infos
    CameraAbilities m_cameraAbilities;
    QString m_cameraModel;
    QString m_cameraPort;

    QList<QString> m_cameraApertures;
    int m_cameraAperture;

signals:
    void previewAvailable(QPixmap preview);
    void imageAvailable(QImage preview);
public slots:
    void capturePreview();
};

#endif // LIVEVIEWTHREAD_H
