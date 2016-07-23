#include "cameraserver.h"

#include <QUrlQuery>
#include <QPair>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qhttpserverconnection.hpp>

using namespace hpis;

CameraServer::CameraServer(CameraThread* cameraThread, QObject *parent) : m_cameraThread(cameraThread), m_httpServer(this), QObject(parent)
{
    m_httpServer.listen(QHostAddress::Any, 8080, [this](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {
        this->processRequest(req, res);
    });

    if ( !m_httpServer.isListening() ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
    }

    connect(cameraThread, SIGNAL(previewAvailable(CameraPreview::Format,QByteArray)), this, SLOT(previewAvailable(CameraPreview::Format,QByteArray)));
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

QJsonDocument CameraServer::ctrlMode(QMap<QString, QString> params)
{
    QJsonObject response;
    QString value = params["action"];

    if (value == "to_rec") {
        m_cameraThread->executeCommand(CameraThread::CommandStartLiveview);
        m_cameraThread->executeCommand(CameraThread::CommandVideoMode);
    } else if (value == "to_cap") {
        m_cameraThread->executeCommand(CameraThread::CommandStartLiveview);
        m_cameraThread->executeCommand(CameraThread::CommandPhotoMode);
    } else if (value == "query") {
        CameraStatus cameraStatus = m_cameraThread->cameraStatus();
        response = cameraStatus.toJsonObject();
    }

    return QJsonDocument(response);
}

void CameraServer::ctrlShutdown()
{
    m_cameraThread->executeCommand(CameraThread::CommandStopLiveview);
}

void CameraServer::ctrlRec(QMap<QString, QString> params)
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
    QJsonDocument jsonResponse;

    if (path == "/ctrl/set")
    {
        ctrlSet(params);
    }
    else if (path == "/ctrl/mode")
    {
        jsonResponse = ctrlMode(params);
    }
    else if (path == "/ctrl/shutdown")
    {
        ctrlShutdown();
    }
    else if (path == "/ctrl/rec")
    {
        ctrlRec(params);
    }
    else if (path == "/liveView")
    {
        m_LiveViewListMutex.lock();
        m_liveViewList.append(res);
        m_LiveViewListMutex.unlock();
        connect(res, SIGNAL(destroyed(QObject*)), this, SLOT(responseDestroyed(QObject*)));
        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("Content-type","multipart/x-mixed-replace; boundary=--jpgboundary");
        return;
    }

    res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
    res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
    res->addHeader("Content-Type", "application/json");
    res->end(jsonResponse.toJson());
}

#include <QDebug>

void CameraServer::responseDestroyed(QObject *resp)
{
    m_LiveViewListMutex.lock();
    qInfo() << "Response destroyed";
    if (m_liveViewList.removeOne((qhttp::server::QHttpResponse*) resp))
    {
        qInfo() << "Removed success";
    }
    m_LiveViewListMutex.unlock();
}

void CameraServer::previewAvailable(CameraPreview::Format format, QByteArray bytes)
{
    QMutexLocker locker(&m_LiveViewListMutex);

    QList<qhttp::server::QHttpResponse*>::iterator i;
    for (i = m_liveViewList.begin(); i != m_liveViewList.end(); ++i)
    {
        qhttp::server::QHttpResponse* resp = *i;
        resp->write(QString("--jpgboundary").toLocal8Bit());
        resp->write(QString("Content-Type: image/jpeg\r\n").toLocal8Bit());
        resp->write(QString("Content-length: %1\r\n").arg(bytes.size()).toLocal8Bit());
        resp->write(QString("\r\n").toLocal8Bit());
        resp->write(bytes);
    }
}
