#ifndef LIVEVIEWDECODERTHREAD_H
#define LIVEVIEWDECODERTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <gphoto2/gphoto2-camera.h>

class CameraThread;

class LiveviewDecoderThread : public QThread
{
    Q_OBJECT
public:
    explicit LiveviewDecoderThread(CameraThread* m_cameraThread, QObject *parent = 0);
    void stop();
    bool decodePreview(CameraFile* cameraFile);
protected:
    void run();
    void doDecodePreview();
    QImage decodeImage(const char *m_data, unsigned long int m_size);
    QImage decodeImageTurbo(const char *m_data, unsigned long int m_size);
private:
    CameraThread* m_cameraThread;

    // Thread control
    bool m_stop;

    // Thread synchronization
    QMutex m_mutex;
    QWaitCondition m_condition;

    // Data
    CameraFile* m_cameraFile;
signals:

public slots:
};

#endif // LIVEVIEWDECODERTHREAD_H
