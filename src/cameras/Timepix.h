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

#ifndef TIMEPIX_H
#define TIMEPIX_H

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <atomic>
#include <vector>
#include <array>
#include <thread>
#include <chrono>

#include "SocketConnector.h"
#include "FileConnector.h"

template <typename T>
class TIMEPIX
{
private:
    using event = T;
    SocketConnector *socket;
    FileConnector file;
    int sleep = 1;
    // read buffer
    std::thread read_guy;
    // process buffer
    std::thread proc_guy;
    int n_buffer_filled=0;
    int n_buffer_processed=0;
    uint64_t current_line = 0;
    // event
    uint64_t probe_position;
    uint64_t probe_position_total;
    uint16_t kx;
    uint16_t ky;
    float d2;

    // read methods
    inline void read_file();
    inline void read_socket();
    
    // process methods
    void vstem();
    void com();
    void airpi();
    void reset();

public:
    // to be overwritten
    inline void process_buffer();
    inline void process_event(event *packet);

    // init variables
    int nx;
    int ny;
    int n_cam;
    int dt; // unit: ns
    bool b_vSTEM;
    bool b_ricom;
    bool b_airpi;
    std::array<float, 2> *p_radius;
    std::array<float, 2> *p_offset;
    std::vector<float> *p_stem_data;
    std::vector<float> *p_ricom_data;
    std::vector<float> *p_comx_data;
    std::vector<float> *p_comy_data;
    std::vector<size_t> *p_dose_data;
    std::vector<size_t> *p_sumx_data;
    std::vector<size_t> *p_sumy_data;
    std::vector<size_t> *p_frame;
    std::vector<float> *p_airpi_data;
    int *p_processor_line;
    int *p_preprocessor_line;
    int mode;
    std::string file_path;
    SocketConnector *p_socket,
    int buffer_size;
    int n_buffer;
    std::vector<std::vector<T>> *p_buffer;

    std::function<void()> process[] = {};
    int n_proc;
    std::vector<std::vector<size_t>*> p_images[] = {};
    int n_images;

    TIMEPIX(
        int nx,
        int ny;,
        int n_cam,
        int dt; // unit: n,
        bool b_vSTEM,
        bool b_ricom,
        bool b_e_mag,
        bool b_airpi,
        std::array<float, 2> *p_radius,
        std::array<float, 2> *p_offset,
        std::vector<float> *p_stem_data,
        std::vector<float> *p_ricom_data,
        std::vector<float> *p_comx_data,
        std::vector<float> *p_comy_data,
        std::vector<size_t> *p_dose_data,
        std::vector<size_t> *p_sumx_data,
        std::vector<size_t> *p_sumy_data,
        std::vector<size_t> *p_frame,
        std::vector<float> *p_airpi_data,
        int *p_processor_line,
        int *p_preprocessor_line,
        int mode,
        std::string file_path,  
        SocketConnector *p_socket,
        int buffer_size,
        int n_buffer,
        std::vector<std::vector<T>> *p_buffer,
    ) : 
        nx(nx),
        ny(ny);,
        n_cam(n_cam),
        dt(dt),
        b_vSTEM(b_vSTEM),
        b_ricom(b_ricom),
        b_e_mag(b_e_mag),
        b_airpi(b_airpi),
        p_radius(p_radius),
        p_offset(p_offset),
        p_stem_data(p_stem_data),
        p_ricom_data(p_ricom_data),
        p_comx_data(p_comx_data),
        p_comy_data(p_comy_data),
        p_dose_data(p_dose_data),
        p_sumx_data(p_sumx_data),
        p_sumy_data(p_sumy_data),
        p_frame(p_frame),
        p_airpi_data(p_airpi_data),
        p_processor_line(p_preprocessor_line),
        p_preprocessor_line(p_preprocessor_line),
        mode(mode),
        file_path(file_path), 
        p_socket(p_socket),
        buffer_size(buffer_size),
        n_buffer(n_buffer),
        p_buffer(p_buffer),
    {
        n_proc = 0;
        n_images = 0;
        if (b_vSTEM) 
        { 
            process[n_proc++] = process_vstem; 
            p_images[n_images++] = p_stem_data;
        }
        if (b_ricom || b_e_mag) 
        { 
            process[n_proc++] = process_com; 
            p_images[n_images++] = p_ricom_data;
            p_images[n_images++] = p_comx_data;
            p_images[n_images++] = p_comy_data;
            p_images[n_images++] = p_sumx_data;
            p_images[n_images++] = p_sumy_data;
            p_images[n_images++] = p_dose_data;
        }
        if (b_airpi) { 
            process[n_proc++] = process_airpi; // not implemented yet
            p_images[n_images++] = p_airpi_data;
        } 
    }

    run();
};
#endif // TIMEPIX_H