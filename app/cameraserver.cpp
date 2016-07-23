#include "cameraserver.h"

#include <QUrlQuery>
#include <QPair>

hpis::CameraServer::CameraServer(CameraThread* cameraThread, QObject *parent) : m_cameraThread(cameraThread), m_httpServer(this), QObject(parent)
{
    m_httpServer.listen(QHostAddress::Any, 8080, [this](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {
        this->processRequest(req, res);
    });

    if ( !m_httpServer.isListening() ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
    }
}

void hpis::CameraServer::ctrlSet(QMap<QString, QString> params)
{
    QString value;

    if (params.contains("iso"))
    {
        value = params["iso"];
        if (value == "Auto")
        {
            m_cameraThread->executeCommand(hpis::CameraThread::CommandEnableIsoAuto);
        }
        else
        {
            m_cameraThread->executeCommand(hpis::CameraThread::CommandDisableIsoAuto);
            m_cameraThread->executeCommand(hpis::CameraThread::Command::setIso(value));
        }
    }
}

void hpis::CameraServer::processRequest(qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res)
{
    QUrl url = req->url();

    QUrlQuery query(url);
    QMap<QString, QString> params;

    QList<QPair<QString, QString> > queryItems = query.queryItems();
    QList<QPair<QString, QString> >::iterator i;
    for (i = queryItems.begin(); i != queryItems.end(); ++i)
    {
        QPair<QString, QString> item = *i;
        params[item.first] = item.second;
    }


    QString path = url.path();
    if (path == "/ctrl/set") {
        ctrlSet(params);
    }

    res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
    res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
    res->end("Hello World!\n");
}
