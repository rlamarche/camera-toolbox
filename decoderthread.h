#ifndef LIVEVIEWDECODERTHREAD_H
#define LIVEVIEWDECODERTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <gphoto2/gphoto2-camera.h>

#include "camerapreview.h"

#include "gpu/omximagedecoder.h"

//#include "hello_jpeg_v2/Logger.h"
//#include "hello_jpeg_v2/JPEG.h"

class CameraThread;

class DecoderThread : public QThread
{
    Q_OBJECT
public:
    explicit DecoderThread(CameraThread* m_cameraThread, QObject *parent = 0);
    ~DecoderThread();

    void stop();
    bool decodePreview(CameraPreview* cameraPreview);
protected:
    void run();
    void doDecodePreview();
    QImage decodeImage(const char *data, unsigned long int size);
    QImage decodeImageTurbo(const char *data, unsigned long int size);
    QImage decodeImageGPU(const char* data, unsigned long int size);
private:
    CameraThread* m_cameraThread;

    // Thread control
    bool m_stop;

    // Thread synchronization
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Data
    CameraPreview* m_cameraPreview;

    // Decoder (OMX)
    OMXImageDecoder m_omxDecoder;

    // Hello JPEG (OMX)
    //Logger m_helloLogstdout;
    //JPEG m_helloJpeg;
signals:

public slots:
};

#endif // LIVEVIEWDECODERTHREAD_H
