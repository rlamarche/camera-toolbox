#include "cameraapiv1.h"

#include <QUrlQuery>
#include <QPair>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <qhttpserverconnection.hpp>

#include "camerasettings.h"

using namespace hpis;

CameraApiV1::CameraApiV1(quint64 id, qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res, CameraThread* cameraThread) : m_connectionId(id), m_req(req), m_res(res), m_cameraThread(cameraThread), QObject(req)
{
    m_isWebSocketRequest = false;
    processRequest();
}

CameraApiV1::CameraApiV1(quint64 id, QWebSocket *pWebSocket, CameraThread* cameraThread) : m_pWebSocket(pWebSocket), m_cameraThread(cameraThread), QObject(pWebSocket)
{
    m_isWebSocketRequest = true;
    processWebSocket();
}

void CameraApiV1::processRequest()
{
    QString prefix("/api/v1/");
    QUrl url = m_req->url();
    QString path = url.path();

    m_req->onData([this](const QByteArray& chunk) {
        m_body.append(chunk);
    });
    m_req->onEnd([this, prefix]() {
        QJsonDocument jsonBody = QJsonDocument::fromJson(m_body);

        QUrl url = m_req->url();
        QString path = url.path();

        path = path.mid(prefix.length() - 1);

        QUrlQuery query(url);
        QMap<QString, QString> params;

        QList<QPair<QString, QString> > queryItems = query.queryItems();
        QList<QPair<QString, QString> >::iterator i;
        for (i = queryItems.begin(); i != queryItems.end(); ++i)
        {
            QPair<QString, QString> item = *i;
            params[item.first] = item.second;
        }

        QJsonDocument response;
        bool endResponse = true;

        if (path == "/info")
        {
            CameraInfo cameraInfo = m_cameraThread->cameraInfo();
            response = QJsonDocument(cameraInfo.toJsonObject());
        }
        else if (path == "/status")
        {
            CameraStatus cameraStatus = m_cameraThread->cameraStatus();
            response = QJsonDocument(cameraStatus.toJsonObject());
        }
        else if (path == "/settings" && m_req->method() == qhttp::EHTTP_POST)
        {
            CameraSettings cameraSettings = CameraSettings::fromJsonObject(jsonBody.object());
            m_cameraThread->setCameraSettings(cameraSettings);

            response = success();
        }
        else if (path == "/liveview.mjpg")
        {
            endResponse = false;
            m_res->setStatusCode(qhttp::ESTATUS_OK);
            m_res->addHeader("Content-type","multipart/x-mixed-replace; boundary=--jpgboundary");
            m_res->addHeader("Cache-Control", "no-cache, must revalidate");
            connect(m_cameraThread, SIGNAL(previewAvailable(hpis::CameraPreview)), this, SLOT(previewAvailable(hpis::CameraPreview)));
        }
        else if (path == "/preview.jpg")
        {
            endResponse = false;
            m_singlePreview = true;
            m_res->setStatusCode(qhttp::ESTATUS_OK);
            m_res->addHeader("Content-type","image/jpeg");
            m_res->addHeader("Cache-Control", "no-cache, must revalidate");
            connect(m_cameraThread, SIGNAL(previewAvailable(hpis::CameraPreview)), this, SLOT(previewAvailable(hpis::CameraPreview)));
        }
        else if (path == "/ctrl/mode")
        {
        }
        else if (path == "/ctrl/shutdown")
        {
        }
        else if (path == "/ctrl/rec")
        {
        }
        else if (path == "/ctrl/focus")
        {
        }
        else if (path == "/preview.jpg" || path == "/preview.jpg")
        {
        }

        if (endResponse)
        {
            m_res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
            m_res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
            m_res->addHeader("Content-Type", "application/json");
            m_res->end(response.toJson());
        }
    });
}

void CameraApiV1::processWebSocket()
{
    connect(m_pWebSocket, &QWebSocket::disconnected, this, &CameraApiV1::webSocketDisconnected);
    QString prefix("/api/v1/");
    QUrl url = m_pWebSocket->requestUrl();
    QString path = url.path();
    path = path.mid(prefix.length() - 1);

    if (path == "/liveview.mjpg")
    {
        connect(m_cameraThread, SIGNAL(previewAvailable(hpis::CameraPreview)), this, SLOT(previewAvailable(hpis::CameraPreview)));
    }

}

void CameraApiV1::previewAvailable(CameraPreview cameraPreview)
{
    if (m_isWebSocketRequest)
    {
        m_pWebSocket->sendBinaryMessage(cameraPreview.data());
    }
    else
    {
        if (m_singlePreview)
        {
            m_res->end(cameraPreview.data());
        }
        else
        {
            m_res->write(QString("--jpgboundary").toLocal8Bit());
            m_res->write(QString("Content-Type: image/jpeg\r\n").toLocal8Bit());
            m_res->write(QString("Content-length: %1\r\n").arg(cameraPreview.data().size()).toLocal8Bit());
            m_res->write(QString("\r\n").toLocal8Bit());
            m_res->write(cameraPreview.data());
        }
    }
}

void CameraApiV1::webSocketDisconnected()
{
    m_pWebSocket->deleteLater();
    qDebug() << "socketDisconnected:" << m_pWebSocket;
}

QJsonDocument CameraApiV1::success()
{
    QJsonObject jsonObject;
    jsonObject["success"] = true;

    return QJsonDocument(jsonObject);
}
