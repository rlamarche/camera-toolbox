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

#include "camera.h"
#include "camerastatus.h"
#include "camerainfo.h"

using namespace hpis;

Camera::Camera(QObject *parent) : QObject(parent)
{

}

CameraStatus hpis::Camera::status()
{
    CameraStatus cs;

    cs.m_exposureModes = exposureModes();
    cs.m_exposureMode = exposureMode();

    cs.m_captureMode = captureMode();
    cs.m_isInLiveView = isInLiveView();
    cs.m_isRecording = isRecording();

    cs.m_apertures = apertures();
    cs.m_aperture = aperture();

    cs.m_shutterSpeeds = shutterSpeeds();
    cs.m_shutterSpeed = shutterSpeed();

    cs.m_isos = isos();
    cs.m_iso = iso();

    cs.m_isoAuto = isoAuto();
    cs.m_exposurePreview = exposurePreview();


    return cs;
}

CameraInfo hpis::Camera::info()
{
    CameraInfo info;
    info.m_displayName = displayName();
    info.m_manufacturer = manufacturer();
    info.m_cameraModel = cameraModel();

    return info;
}
