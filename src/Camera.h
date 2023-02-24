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

#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <atomic>
#include <array>
#include <stdlib.h>

// Only forward declaration for Ricom class
class Ricom;

namespace CAMERA
{
    enum Camera_model
    {
        MERLIN,
        TIMEPIX,
        MODELS_COUNT
    };

    enum Camera_type
    {
        FRAME_BASED,
        EVENT_BASED
    };

    class Camera_BASE
    {
    public:
        Camera_model model;
        Camera_type type;
        int nx_cam;
        int ny_cam;
        bool swap_endian;
        int depth;
        int dwell_time;
        std::vector<int> u;
        std::vector<int> v;
        void init_uv_default();

        Camera_BASE() : model(MERLIN),
                        type(FRAME_BASED),
                        nx_cam(256),
                        ny_cam(256),
                        swap_endian(true),
                        depth(1),
                        dwell_time(1000){};
    };

    // primary template
    template <class CameraInterface, CAMERA::Camera_type cam_type>
    class Camera : public CameraInterface, public Camera_BASE
    {
    };

    // specialization for frame based camera
    template <class CameraInterface>
    class Camera<CameraInterface, FRAME_BASED> : public CameraInterface, public Camera_BASE
    {
    public:
        Camera();
        explicit Camera(Camera_BASE &cam);
        void run(Ricom *ricom);
        template <typename T>
        void read_frame(std::vector<T> &data, bool b_first);
    };

    // specialization for event based camera
    template <class CameraInterface>
    class Camera<CameraInterface, EVENT_BASED> : public CameraInterface, public Camera_BASE
    {
    public:
        Camera();
        explicit Camera(Camera_BASE &cam);
        void run(Ricom *ricom);
        void read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                            std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                            std::vector<float> &stem_map, bool b_stem,
                            std::array<float, 2> &offset, std::array<float, 2> &radius,
                            size_t first_frame, size_t end_frame);
        void read_frame_com_cbed(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                 std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                 std::vector<float> &stem_map, bool b_stem,
                                 std::array<float, 2> &offset, std::array<float, 2> &radius,
                                 std::vector<uint16_t> &frame, size_t frame_id,
                                 size_t first_frame, size_t end_frame);
    };

    class Default_configurations
    {
    public:
        Default_configurations();
        CAMERA::Camera_BASE &operator[](unsigned int index);
        std::array<CAMERA::Camera_BASE, CAMERA::MODELS_COUNT> hws;

    private:
        CAMERA::Camera_BASE *hws_ptr;
    };
}

#endif // CAMERA_H