#ifndef LIVEVIEWTHREAD_H
#define LIVEVIEWTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QPixmap>
#include <QImage>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>

class LiveviewDecoderThread;

class CameraThread : public QThread
{
    friend class LiveviewDecoderThread;

    Q_OBJECT
public:
    explicit CameraThread(GPContext* context, Camera* camera, QObject *parent = 0);
    void stop();
protected:
    void run();
    void doCapturePreview();
    void previewDecoded(QImage image);
    QImage decodeImage(const char *data, unsigned long int size);
    QImage decodeImageTurbo(const char *data, unsigned long int size);
private:
    // Gphoto context
    GPContext*              m_context;
    Camera*                 m_camera;

    // Thread control
    bool m_stop;

    // Thread synchronization
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Data
    QPixmap m_preview;

    // Decoder
    LiveviewDecoderThread* m_decoderThread;
signals:
    void previewAvailable(QPixmap preview);
    void imageAvailable(QImage preview);
public slots:
    void capturePreview();
};

#endif // LIVEVIEWTHREAD_H
