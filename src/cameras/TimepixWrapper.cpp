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

#ifndef TIMEPIX_WRAPPER_H
#define TIMEPIX_WRAPPER_H

#include "Camera.h"
#include "TimepixInterface.h"
#include "Ricom.h"

using namespace CAMERA;

// Default constructor and definition of default configurations
template <>
Camera<TimepixInterface, EVENT_BASED>::Camera()
{
    model = TIMEPIX;
    type = EVENT_BASED;
    nx_cam = 256;
    ny_cam = 256;
    swap_endian = false;
    dwell_time = 1000;
};
template <>
Camera<TimepixInterface, FRAME_BASED>::Camera(){};

// Constructor with camera_base as argument
template <>
Camera<TimepixInterface, EVENT_BASED>::Camera(Camera_BASE &cam)
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
    dt = cam.dwell_time;
    nx = cam.nx_cam;
    ny = cam.ny_cam;

};
template <>
Camera<TimepixInterface, FRAME_BASED>::Camera(Camera_BASE &cam) { (void)cam; };

// read_frame_com method wrapper
template <>
void Camera<TimepixInterface, EVENT_BASED>::read_frame_com(
    std::vector<size_t> &dose_map, 
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
    std::vector<float> &stem_map, bool b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    bool &b_stop, int &finished_line, size_t &first_frame, size_t &end_frame
)
{
    TimepixInterface::read_frame_com(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, b_stop, finished_line, first_frame, end_frame);
}

// read_frame_com method wrapper
template <> 
void Camera<TimepixInterface, EVENT_BASED>::read_frame_com_cbed(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
    bool &b_stop, int &finished_line, size_t &first_frame, size_t &end_frame
)
{
    TimepixInterface::read_frame_com(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, frame, frame_id_plot_cbed, b_stop, finished_line, first_frame, end_frame);
}

// Run method wrapper
template <>
void Camera<TimepixInterface, EVENT_BASED>::run(Ricom *ricom)
{
    switch (ricom->mode)
    {
    case RICOM::FILE:
        TimepixInterface::init_interface(ricom->file_path);
        ricom->process_data<TimepixInterface>(this);
        close_interface();
        break;
    case RICOM::TCP:
        // TimepixInterface::init_interface(ricom->socket);
        perror("TCP mode not supported for Timepix yet");
    }
};
template <>
void Camera<TimepixInterface, FRAME_BASED>::run(Ricom *ricom) { (void)ricom; };

#endif // TIMEPIX_WRAPPER_H