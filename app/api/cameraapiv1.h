#ifndef CAMERAAPIV1_H
#define CAMERAAPIV1_H

#include <QObject>

#include "camerathread.h"

#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>

#include <QWebSocket>

namespace hpis {

class CameraApiV1 : public QObject
{
    Q_OBJECT
public:
    explicit CameraApiV1(quint64 id, qhttp::server::QHttpRequest* m_req, qhttp::server::QHttpResponse* m_res, CameraThread* cameraThread);
    explicit CameraApiV1(quint64 id, QWebSocket* pWebSocket, CameraThread* cameraThread);

    void processRequest();
    void processWebSocket();

protected:
    QJsonDocument success();
private:
    bool m_isWebSocketRequest;
    quint64 m_connectionId;
    qhttp::server::QHttpRequest* m_req;
    qhttp::server::QHttpResponse* m_res;

    QWebSocket* m_pWebSocket;

    CameraThread* m_cameraThread;
    QByteArray m_body;

    bool m_singlePreview = false;
    bool m_allBytesWritten = true;
signals:

public slots:
    void previewAvailable(hpis::CameraPreview cameraPreview);
    void cameraStatusAvailable(hpis::CameraStatus cameraStatus);

    void webSocketDisconnected();
    void onAllBytesWritten();
};

}

#endif // CAMERAAPIV1_H
