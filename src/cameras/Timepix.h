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
#include <functional>

#include "SocketConnector.h"
#include "FileConnector.h"


namespace TIMEPIX_ADDITIONAL{
    const size_t BUFFER_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
    PACK(struct EVENT
    {
        uint32_t index;
        uint64_t toa;
        uint8_t overflow;
        uint8_t ftoa;
        uint16_t tot;
    });
}


class TIMEPIX
{
private:
    int sleep = 1;

    // process methods
    void vstem();
    void com();
    void airpi();

protected:
    // access by child class
    SocketConnector *socket;
    FileConnector file;
    std::thread read_guy;
    std::thread proc_guy;

    int n_buffer_filled=0;
    int n_buffer_processed=0;
    uint64_t current_line = 0;
    uint64_t probe_position;
    uint64_t probe_position_total;
    uint16_t kx;
    uint16_t ky;
    float d2;
    void flush_next_line(int line_id);
    void reset();

public:
    // to be overwritten
    using event = TIMEPIX_ADDITIONAL::EVENT;
    inline void read_file();
    inline void read_socket();
    inline void process_buffer();
    inline void process_event(event *packet);
    void run();

    // to be called
    void terminate();

    // init variables
    int nx;
    int ny;
    int nxy;
    int n_cam;
    int dt; // unit: ns
    bool b_vSTEM;
    bool b_ricom;
    bool b_e_mag;
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
    SocketConnector *p_socket;
    int buffer_size = TIMEPIX_ADDITIONAL::BUFFER_SIZE;
    int n_buffer = TIMEPIX_ADDITIONAL::N_BUFFER;
    std::array<std::array<TIMEPIX_ADDITIONAL::EVENT, TIMEPIX_ADDITIONAL::BUFFER_SIZE>, TIMEPIX_ADDITIONAL::N_BUFFER> buffer;

    std::vector<std::function<void()>> process;
    int n_proc = 0;
    std::vector<std::vector<size_t>*> p_images_t;
    int n_images_t = 0;
    std::vector<std::vector<float>*> p_images_f;
    int n_images_f = 0;


    TIMEPIX(
        int &nx,
        int &ny,
        int &n_cam,
        int &dt, // unit: n,
        bool &b_vSTEM,
        bool &b_ricom,
        bool &b_e_mag,
        bool &b_airpi,
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
        int &mode,
        std::string &file_path,  
        SocketConnector *p_socket
    ) : 
        nx(nx),
        ny(ny),
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
        p_processor_line(p_processor_line),
        p_preprocessor_line(p_preprocessor_line),
        mode(mode),
        file_path(file_path), 
        p_socket(p_socket)
    {
        nxy = nx*ny;
        n_proc = 0;
        n_images_t = 0;
        n_images_f = 0;
        if (b_vSTEM) 
        { 
            process.push_back(std::bind(&TIMEPIX::vstem, this));
            p_images_f.push_back(p_stem_data);
            ++n_proc;
            ++n_images_f;
        }
        if (b_ricom || b_e_mag) 
        { 
            process.push_back(std::bind(&TIMEPIX::com, this));
            p_images_f.push_back(p_ricom_data);
            p_images_f.push_back(p_comx_data);
            p_images_f.push_back(p_comy_data);
            p_images_t.push_back(p_sumx_data);
            p_images_t.push_back(p_sumy_data);
            p_images_t.push_back(p_dose_data);
            ++n_proc; 
            n_images_f += 3;
            n_images_t += 3;
        }
        if (b_airpi) { 
            process.push_back(std::bind(&TIMEPIX::airpi, this)); // not implemented yet
            p_images_f.push_back(p_airpi_data);
            ++n_proc; 
            ++n_images_f;
        } 
    }
};
#endif // TIMEPIX_H