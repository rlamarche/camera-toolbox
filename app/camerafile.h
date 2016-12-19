#ifndef CAMERAFILE_H
#define CAMERAFILE_H

#include <QByteArray>
#include <QString>
#include <QObject>

namespace hpis {

class CameraFile
{
public:
    CameraFile();
    CameraFile(QString path, QString name);

    void setPath(QString path);
    void setName(QString name);

    QString path();
    QString name();

private:
    QString m_path;
    QString m_name;
};

}

Q_DECLARE_METATYPE(hpis::CameraFile)

#endif // CAMERAFILE_H
