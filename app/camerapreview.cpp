#include "camerapreview.h"

using namespace hpis;

CameraPreview::CameraPreview(QByteArray &data, QString mimetype) : m_data(data), m_mimetype(mimetype)
{

}


const QByteArray CameraPreview::data()
{
    return m_data;
}

QString CameraPreview::mimetype()
{
    return m_mimetype;
}
