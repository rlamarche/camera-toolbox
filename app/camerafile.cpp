#include "camerafile.h"

hpis::CameraFile::CameraFile()
{

}


hpis::CameraFile::CameraFile(QString path, QString name) :
    m_path(path), m_name(name)
{

}

void hpis::CameraFile::setPath(QString path)
{
    this->m_path = path;
}

void hpis::CameraFile::setName(QString name)
{
    this->m_name = name;
}

QString hpis::CameraFile::path()
{
    return m_path;
}

QString hpis::CameraFile::name()
{
    return m_name;
}
