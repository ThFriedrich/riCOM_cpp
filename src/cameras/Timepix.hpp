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

template <typename event, int buffer_size, int n_buffer>
class TIMEPIX
{
private:
    int sleep = 1;

    // process methods
    void vstem()
    {
        d2 = pow((float)kx - (*p_offset)[0], 2) + pow((float)ky - (*p_offset)[1], 2);
        if (d2 > (*p_radius)[0] && d2 <= (*p_radius)[1])
        {
            (*p_stem_data)[id_image][probe_position]++;
        }
    };
    void com()
    {
        (*p_dose_data)[id_image][probe_position]++;
        (*p_sumy_data)[id_image][probe_position] += ky;
        (*p_sumx_data)[id_image][probe_position] += kx;
    };
    void airpi(){};

protected:
    SocketConnector *socket;
    FileConnector file;
    std::thread read_guy;
    std::thread proc_guy;

    int n_buf = n_buffer;
    int n_buffer_filled=0;
    int n_buffer_processed=0;
    int id_image;
    uint64_t current_line = 0;
    uint64_t probe_position;
    uint64_t probe_position_total;
    uint16_t kx;
    uint16_t ky;
    float d2;
    inline void read_file()
    {
        int buffer_id;
        while (*p_processor_line!=-1)
        {
            if ( (n_buffer_filled < (n_buffer + n_buffer_processed)) && (*p_preprocessor_line < (*p_processor_line + (int)(ny/2))) ) 
            {
                buffer_id = n_buffer_filled % n_buffer;
                file.read_data((char *)&(buffer[buffer_id]), sizeof(buffer[buffer_id]));
                ++n_buffer_filled;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        file.close_file();
    };
    inline void read_socket()
    {
        int buffer_id;
        while (*p_processor_line != -1)
        {
            if (n_buffer_filled < (n_buffer + n_buffer_processed))
            {
                buffer_id = n_buffer_filled % n_buffer;
                socket->read_data((char *)&(buffer[buffer_id]), sizeof(buffer[buffer_id]));
                ++n_buffer_filled;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        std::cout << "n_buffer_filled" << std::endl;
    };
    void flush_image(int id_img)
    {
        if (!*b_cumulative)
        {
            for (int i_image = 0; i_image < n_images; i_image++)
            {
                std::fill(
                    (*(p_images[i_image]))[id_img].begin(),
                    (*(p_images[i_image]))[id_img].end(), 0);
            }
        }
    };
    void reset()
    {
        n_buffer_filled = 0;
        n_buffer_processed = 0;
        current_line = 0;
    };

public:
    void terminate()
    {
        while ( (!read_guy.joinable()) || (!proc_guy.joinable()) ) 
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        read_guy.join();
        proc_guy.join();
    };

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
    bool *b_cumulative;
    std::array<float, 2> *p_radius;
    std::array<float, 2> *p_offset;
    std::vector<size_t> (*p_dose_data)[2];
    std::vector<size_t> (*p_sumx_data)[2];
    std::vector<size_t> (*p_sumy_data)[2];
    std::vector<size_t> (*p_stem_data)[2];
    std::vector<size_t> *p_frame;
    int *p_processor_line;
    int *p_preprocessor_line;
    int mode;
    std::string file_path;
    SocketConnector *p_socket;
    std::array<std::array<event, buffer_size>, n_buffer> buffer;

    std::vector<std::function<void()>> process;
    int n_proc = 0;
    std::vector<std::vector<size_t> (*)[2]> p_images;
    int n_images = 0;


    TIMEPIX(
        int &nx,
        int &ny,
        int &n_cam,
        int &dt, // unit: ns
        bool &b_vSTEM,
        bool &b_ricom,
        bool &b_e_mag,
        bool &b_airpi,
        bool *b_cumulative,
        std::array<float, 2> *p_radius,
        std::array<float, 2> *p_offset,
        std::vector<size_t> (*p_dose_data)[2],
        std::vector<size_t> (*p_sumx_data)[2],
        std::vector<size_t> (*p_sumy_data)[2],
        std::vector<size_t> (*p_stem_data)[2],
        std::vector<size_t> *p_frame,
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
        b_cumulative(b_cumulative),
        p_radius(p_radius),
        p_offset(p_offset),
        p_dose_data(p_dose_data),
        p_sumx_data(p_sumx_data),
        p_sumy_data(p_sumy_data),
        p_stem_data(p_stem_data),
        p_frame(p_frame),
        p_processor_line(p_processor_line),
        p_preprocessor_line(p_preprocessor_line),
        mode(mode),
        file_path(file_path), 
        p_socket(p_socket)
    {
        nxy = nx*ny;
        n_proc = 0;
        n_images = 0;
        if (b_vSTEM) 
        { 
            process.push_back(std::bind(&TIMEPIX::vstem, this));
            p_images.push_back(p_stem_data);
            ++n_proc;
            ++n_images;
        }
        if (b_ricom || b_e_mag) 
        { 
            process.push_back(std::bind(&TIMEPIX::com, this));
            p_images.push_back(p_sumx_data);
            p_images.push_back(p_sumy_data);
            p_images.push_back(p_dose_data);
            ++n_proc; 
            n_images += 3;
        }
        if (b_airpi) { 
            process.push_back(std::bind(&TIMEPIX::airpi, this)); // not implemented yet
            ++n_proc; 
        } 
    }
};
#endif // TIMEPIX_H