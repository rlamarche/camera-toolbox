#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include <QObject>

#include "camerathread.h"

#include "api/cameraapiv1.h"

#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>

#include <QList>
#include <QMutex>

#include <QWebSocket>
#include <QWebSocketServer>

namespace hpis {

class CameraServer : public QObject
{
    Q_OBJECT
public:
    explicit CameraServer(CameraThread* cameraThread, QObject *parent = 0);
    
    
protected:
    void processRequest(qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res);

    void ctrlSet(QMap<QString, QString> params);
    QJsonDocument ctrlGet(QMap<QString, QString> params);
    QJsonDocument ctrlMode(QMap<QString, QString> params);
    void ctrlRec(QMap<QString, QString> params);
    void ctrlShutdown();


private:
    CameraThread* m_cameraThread;
    qhttp::server::QHttpServer m_httpServer;

    QMutex m_LiveViewListMutex;
    QList<qhttp::server::QHttpResponse*> m_liveViewList;
    QMutex m_previewListMutex;
    QList<qhttp::server::QHttpResponse*> m_previewList;
    quint64 m_connectionCounter = 0;

    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_webSocketClients;

signals:
    void webSocketClosed();

public slots:
    void responseDestroyed(QObject* resp);
    void previewAvailable(hpis::CameraPreview cameraPreview);

private slots:
    void onNewWebSocketConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
};

}

#endif // CAMERASERVER_H
