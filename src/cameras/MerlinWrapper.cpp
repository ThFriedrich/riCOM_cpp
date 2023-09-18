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

#ifndef MERLIN_WRAPPER_H
#define MERLIN_WRAPPER_H

#include "Camera.h"
#include "MerlinInterface.h"
#include "Ricom.h"

using namespace CAMERA;

// Default constructor and definition of default configurations
template <>
Camera<MerlinInterface, FRAME_BASED>::Camera()
{
    model = MERLIN;
    type = FRAME_BASED;
    data_depth = 1;
    nx = 256;
    ny = 256;
    swap_endian = true;
};
template <>
Camera<MerlinInterface, EVENT_BASED>::Camera(){};

// Constructor with camera_base as argument
template <>
Camera<MerlinInterface, FRAME_BASED>::Camera(Camera_BASE &cam)
{
    type = FRAME_BASED;
    data_depth = cam.depth;
    nx = cam.nx_cam;
    ny = cam.ny_cam;
    swap_endian = true;
    u = cam.u;
    v = cam.v;
};
template <>
Camera<MerlinInterface, EVENT_BASED>::Camera(Camera_BASE &cam) { (void)cam; };

// Read frame method wrapper
template <>
template <typename T>
void Camera<MerlinInterface, FRAME_BASED>::read_frame(std::vector<T> &data, bool b_first)
{
    MerlinInterface::read_frame<T>(data, b_first);
};
// Template Specializations to avoid linker issues
template void Camera<MerlinInterface, FRAME_BASED>::read_frame(std::vector<uint8_t> &data, bool dump_head);
template void Camera<MerlinInterface, FRAME_BASED>::read_frame(std::vector<uint16_t> &data, bool dump_head);
template void Camera<MerlinInterface, FRAME_BASED>::read_frame(std::vector<uint32_t> &data, bool dump_head);

// Run method wrapper
template <>
void Camera<MerlinInterface, FRAME_BASED>::run(Ricom *ricom)
{
    switch (ricom->mode)
    {
    case RICOM::modes::FILE:
        MerlinInterface::init_interface(ricom->file_path);
        break;
    case RICOM::modes::TCP:
        MerlinInterface::init_interface(&ricom->socket);
        break;
    }
    int bits = MerlinInterface::pre_run(ricom->camera.u, ricom->camera.v);
    switch (bits)
    {
    case 8:
        ricom->process_data<uint8_t, MerlinInterface>(this);
        break;
    case 16:
        ricom->process_data<uint16_t, MerlinInterface>(this);
        break;
    default:
        perror("Camera<MerlinInterface, FRAME_BASED>::run: pre_run returned an error!");
        break;
    }
    close_interface();
};
template <>
void Camera<MerlinInterface, EVENT_BASED>::run(Ricom *ricom) { (void)ricom; };
#endif // MERLIN_WRAPPER_H