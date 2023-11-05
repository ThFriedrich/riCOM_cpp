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

#ifndef ADVAPIX_H
#define ADVAPIX_H

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

#include "FileConnector.h"
#include "Timepix.h"

namespace ADVAPIX_ADDITIONAL
{
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

class ADVAPIX : public TIMEPIX
{ 
public:
    inline void read_file();
    inline void read_socket();
    int buffer_size = BUFFER_SIZE;
    int n_buffer = N_BUFFER;
    std::array<std::array<EVENT, BUFFER_SIZE>, N_BUFFER> buffer;
    using event = EVENT;

    void run();

    ADVAPIX(
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
    ) : TIMEPIX(
            nx,
            ny,
            n_cam,
            dt,
            b_vSTEM,
            b_ricom,
            b_e_mag,
            b_airpi,
            p_radius,
            p_offset,
            p_stem_data,
            p_ricom_data,
            p_comx_data,
            p_comy_data,
            p_dose_data,
            p_sumx_data,
            p_sumy_data,
            p_frame,
            p_airpi_data,
            p_processor_line,
            p_preprocessor_line,
            mode,
            file_path,
            p_socket
        ) {}
};
}; // end of namespace

#endif // ADVAPIX_H