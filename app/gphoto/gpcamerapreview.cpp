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

#include "gpcamerapreview.h"

GPCameraPreview::GPCameraPreview(CameraFile* cameraFile) : CameraPreview(), m_cameraFile(cameraFile)
{
    gp_file_ref(m_cameraFile);
    gp_file_get_data_and_size(m_cameraFile, &m_data, &m_size);
}

GPCameraPreview::GPCameraPreview(const GPCameraPreview &copy)
{
    m_cameraFile = copy.m_cameraFile;
    m_data = copy.m_data;
    m_size = copy.m_size;

    gp_file_ref(m_cameraFile);
}

GPCameraPreview::~GPCameraPreview()
{
    gp_file_unref(m_cameraFile);
}


const char* GPCameraPreview::data()
{
    return m_data;
}

unsigned long GPCameraPreview::size()
{
    return m_size;
}

CameraPreview::Format GPCameraPreview::format()
{
    return CameraPreview::FormatJPG;
}
