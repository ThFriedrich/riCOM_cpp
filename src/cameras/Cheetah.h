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

#ifndef CHEETAH_H
#define CHEETAH_H

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
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "FileConnector.h"
#include "Timepix.h"

namespace CHEETAH_ADDITIONAL{
    const size_t BUFFER_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
    using EVENT = uint64_t;

class CHEETAH : public TIMEPIX
{
private:
    // header
    int chip_id;
    uint64_t tpx_header = 861425748; //(b'TPX3', 'little')
    // TDC
    uint64_t rise_t[4];
    uint64_t fall_t[4];
    bool rise_fall[4] = {false, false, false, false};
    int line_count[4];
    // int total_line = 0; // current_line
    int most_advanced_line = 0;
    uint64_t line_interval;
    uint64_t dwell_time;
    // event
    uint64_t toa;
    uint64_t pack_44;
    int address_multiplier[4] = {1,-1,-1,1};
    int address_bias_x[4] = {256, 511, 255, 0};
    int address_bias_y[4] = {0, 511, 511, 0};

    void reset();
    inline int which_type(EVENT *packet);
    inline void process_tdc(EVENT *packet);

public:

    inline void read_file();
    inline void read_socket();
    int buffer_size = BUFFER_SIZE;
    int n_buffer = N_BUFFER;
    std::array<std::array<EVENT, BUFFER_SIZE>, N_BUFFER> buffer;
    using event = EVENT;

    inline bool process_event(EVENT *packet);
    inline void process_buffer();
    void run();

    CHEETAH(
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
}; //end of namespace


class CheetahComm
{
private:
    std::string serverip = "localhost";
    std::string serverport = "8080";
    std::string rawip = "127.0.0.1";
    std::string rawport = "8451";
    std::string serverurl = "http://" + serverip + ":" + serverport;

public:
    void tpx3_det_config();
    void tpx3_cam_init();
    void tpx3_destination();
    void tpx3_acq_init();
    void start();
    void stop();
};


#endif // CHEETAH_H