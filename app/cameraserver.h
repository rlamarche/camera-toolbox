#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include <QObject>

#include "camerathread.h"

#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>

namespace hpis {

class CameraServer : public QObject
{
    Q_OBJECT
public:
    explicit CameraServer(CameraThread* cameraThread, QObject *parent = 0);
    
protected:
    void processRequest(qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res);
    
    void ctrlSet(QMap<QString, QString> params);
    
private:
    CameraThread* m_cameraThread;
    qhttp::server::QHttpServer m_httpServer;
signals:

public slots:
};

}

#endif // CAMERASERVER_H
