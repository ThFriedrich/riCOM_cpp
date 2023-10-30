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

#ifndef CHEETAH_WRAPPER_H
#define CHEETAH_WRAPPER_H

#include "Camera.h"
#include "CheetahInterface.h"
#include "Ricom.h"

using namespace CAMERA;

// Default constructor and definition of default configurations
template <>
Camera<CheetahInterface, EVENT_BASED>::Camera()
{
    model = CHEETAH;
    type = EVENT_BASED;
    nx_cam = 512;
    ny_cam = 512;
    swap_endian = false;
};
template <>
Camera<CheetahInterface, FRAME_BASED>::Camera(){};

// Constructor with camera_base as argument
template <>
Camera<CheetahInterface, EVENT_BASED>::Camera(Camera_BASE &cam)
{
    // Copy members of Camera_BASE for use in Ricom.cpp
    type = cam.type;
    model = cam.model;
    nx_cam = cam.nx_cam;
    ny_cam = cam.ny_cam;
    swap_endian = cam.swap_endian;
    u = cam.u;
    v = cam.v;
    group_size = cam.group_size;
    // Assign additional members used in CheetahInterface.cpp
    nx = cam.nx_cam;
    ny = cam.ny_cam;
    gs = cam.group_size;
};
template <>
Camera<CheetahInterface, FRAME_BASED>::Camera(Camera_BASE &cam) { (void)cam; };

// read_frame_com method wrapper
template <>
void Camera<CheetahInterface, EVENT_BASED>::read_frame_com(
    std::vector<size_t> &dose_map, 
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
    std::vector<float> &stem_map, bool &b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    int &processor_line, int &preprocessor_line, size_t &first_frame, size_t &end_frame
)
{
    CheetahInterface::read_frame_com(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, processor_line, preprocessor_line);
}

// read_frame_com_cbed method wrapper
template <> 
void Camera<CheetahInterface, EVENT_BASED>::read_frame_com_cbed(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool &b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
    int &processor_line, int &preprocessor_line, size_t &first_frame, size_t &end_frame
    )
{
    CheetahInterface::read_frame_com(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, frame, frame_id_plot_cbed, processor_line, preprocessor_line);
}

// Run method wrapper
template <>
void Camera<CheetahInterface, EVENT_BASED>::run(Ricom *ricom)
{
    switch (ricom->mode)
    {
    case RICOM::FILE:
        CheetahInterface::init_interface(ricom->file_path);
        ricom->process_data<CheetahInterface>(this);
        close_interface();
        break;
    case RICOM::TCP:
        ricom->socket.socket_type = Socket_type::SERVER;
        CheetahInterface::init_interface(&ricom->socket);
        ricom->process_data<CheetahInterface>(this);
        close_interface();
    }
};
template <>
void Camera<CheetahInterface, FRAME_BASED>::run(Ricom *ricom) { (void)ricom; };

#endif // CHEETAH_WRAPPER_H