#include "cameraserver.h"

#include <QUrlQuery>
#include <QPair>

using namespace hpis;

CameraServer::CameraServer(CameraThread* cameraThread, QObject *parent) : m_cameraThread(cameraThread), m_httpServer(this), QObject(parent)
{
    m_httpServer.listen(QHostAddress::Any, 8080, [this](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {
        this->processRequest(req, res);
    });

    if ( !m_httpServer.isListening() ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
    }
}

void CameraServer::ctrlSet(QMap<QString, QString> params)
{
    QString value;

    if (params.contains("iso"))
    {
        value = params["iso"];
        if (value == "Auto")
        {
            m_cameraThread->executeCommand(CameraThread::CommandEnableIsoAuto);
        }
        else
        {
            m_cameraThread->executeCommand(CameraThread::CommandDisableIsoAuto);
            m_cameraThread->executeCommand(CameraThread::Command::setIso(value));
        }
    }
}

void CameraServer::ctrlMode(QMap<QString, QString> params)
{
    QString value = params["action"];

    if (value == "to_rec") {
        m_cameraThread->executeCommand(CameraThread::CommandStartLiveview);
        m_cameraThread->executeCommand(CameraThread::CommandVideoMode);
    } else if (value == "to_cap") {
        m_cameraThread->executeCommand(CameraThread::CommandStartLiveview);
        m_cameraThread->executeCommand(CameraThread::CommandPhotoMode);
    } else if (value == "off") {
        m_cameraThread->executeCommand(CameraThread::CommandStopLiveview);
    }
}

void CameraServer::ctrlShutdown()
{
    m_cameraThread->executeCommand(CameraThread::CommandStopLiveview);
}

void CameraServer::processRequest(qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res)
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
    if (path == "/ctrl/set")
    {
        ctrlSet(params);
    }
    else if (path == "/ctrl/mode")
    {
        ctrlMode(params);
    }
    else if (path == "/ctrl/shutdown")
    {
        ctrlShutdown();
    }
    else if (path == "/ctrl/rec")
    {
        CameraStatus cameraStatus = m_cameraThread->cameraStatus();
        QString value = params["action"];

        if (cameraStatus.captureMode() == Camera::CaptureModeVideo)
        {
            if (value == "start")
            {
                m_cameraThread->executeCommand(CameraThread::CommandStartMovie);
            }
            else if (value == "stop")
            {
                m_cameraThread->executeCommand(CameraThread::CommandStopMovie);
            }
        } else {
            m_cameraThread->executeCommand(CameraThread::CommandCapturePhoto);
        }
    }

    res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
    res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
    res->end("Hello World!\n");
}
