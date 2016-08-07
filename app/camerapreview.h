#ifndef CAMERAPREVIEW_H
#define CAMERAPREVIEW_H

#include <QByteArray>
#include <QString>
#include <QObject>

namespace hpis {

class CameraPreview
{
public:
    CameraPreview() {}
    CameraPreview(QByteArray& data, QString mimetype);
    virtual ~CameraPreview() {}

    const QByteArray data();
    QString mimetype();

private:
    QByteArray m_data;
    QString m_mimetype;
};

}

Q_DECLARE_METATYPE(hpis::CameraPreview)

#endif // CAMERAPREVIEW_H
