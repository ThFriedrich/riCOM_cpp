/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu,
 * University of Antwerp - All Rights Reserved.
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Authors:
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#include "Camera.h"
#include "MerlinInterface.h"
#include "TimepixInterface.h"
#include "CheetahInterface.h"

using namespace CAMERA;

void Camera_BASE::init_uv_default()
{
    u.resize(nx_cam);
    v.resize(ny_cam);

    for (int i = 0; i < nx_cam; i++)
    {
        u[i] = i;
    }

    for (int i = 0; i < ny_cam; i++)
    {
        v[i] = i;
    }
};

Default_configurations::Default_configurations()
{
    hws_ptr = &hws[0];
    hws[MERLIN] = Camera<MerlinInterface, FRAME_BASED>();
    hws[TIMEPIX] = Camera<TimepixInterface, EVENT_BASED>();
    hws[CHEETAH] = Camera<CheetahInterface, EVENT_BASED>();
};

CAMERA::Camera_BASE &Default_configurations::operator[](unsigned int index)
{
    if (index >= hws.size())
    {
        perror("CAMERA::Default_configurations::operator[]: Index out of range");
    }
    return hws_ptr[index];
}
