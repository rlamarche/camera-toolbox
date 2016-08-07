#ifndef CAMERAAPIV1_H
#define CAMERAAPIV1_H

#include <QObject>

#include "camerathread.h"

#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>


namespace hpis {

class CameraApiV1 : public QObject
{
    Q_OBJECT
public:
    explicit CameraApiV1(quint64 id, qhttp::server::QHttpRequest* m_req, qhttp::server::QHttpResponse* m_res, CameraThread* cameraThread);

    void processRequest();

protected:
    QJsonDocument success();
private:
    quint64 m_connectionId;
    qhttp::server::QHttpRequest* m_req;
    qhttp::server::QHttpResponse* m_res;
    CameraThread* m_cameraThread;
    QByteArray m_body;

    bool m_singlePreview = false;
signals:

public slots:
    void previewAvailable(hpis::CameraPreview cameraPreview);
};

}

#endif // CAMERAAPIV1_H
