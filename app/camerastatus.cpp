/*
 * This file is part of Camera Toolbox.
 *   (https://github.com/rlamarche/camera-toolbox)
 * Copyright (c) 2016 Romain Lamarche.
 *
 * Camera Toolbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * Camera Toolbox is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "camerastatus.h"

#include <QJsonObject>
#include <QJsonArray>

using namespace hpis;

CameraStatus::CameraStatus()
{

}

Camera::CaptureMode CameraStatus::captureMode()
{
    return m_captureMode;
}

QList<QString> CameraStatus::exposureModes()
{
    return m_exposureModes;
}

QString CameraStatus::exposureMode()
{
    return m_exposureMode;
}

bool CameraStatus::isInLiveView()
{
    return m_isInLiveView;
}

bool CameraStatus::isRecording()
{
    return m_isRecording;
}

QList<QString> CameraStatus::apertures()
{
    return m_apertures;
}

QString CameraStatus::aperture()
{
    return m_aperture;
}

QList<QString> CameraStatus::isos()
{
    return m_isos;
}

QString CameraStatus::iso()
{
    return m_iso;
}

bool CameraStatus::isoAuto()
{
    return m_isoAuto;
}

QList<QString> CameraStatus::shutterSpeeds()
{
    return m_shutterSpeeds;
}

QString CameraStatus::shutterSpeed()
{
    return m_shutterSpeed;
}

bool CameraStatus::exposurePreview()
{
    return m_exposurePreview;
}

QJsonObject CameraStatus::toJsonObject()
{
    QJsonObject status;

    status["exposureModes"] = QJsonArray::fromStringList(m_exposureModes);
    status["exposureMode"] = m_exposureMode;

    status["captureMode"] = m_captureMode;
    status["isInLiveView"] = m_isInLiveView;
    status["isRecording"] = m_isRecording;

    status["apertures"] = QJsonArray::fromStringList(m_apertures);
    status["aperture"] = m_aperture;

    status["shutterSpeeds"] = QJsonArray::fromStringList(m_shutterSpeeds);
    status["shutterSpeed"] = m_shutterSpeed;

    status["isos"] = QJsonArray::fromStringList(m_isos);
    status["iso"] = m_iso;

    return status;
}
