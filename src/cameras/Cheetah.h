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

#include "FileConnector.h"
#include "Timepix.h.h"

namespace CHEETAH_ADDITIONAL{
    const size_t PRELOC_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
    using event = uint64_t;
}

template <typename T>
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
    uint64_t probe_position;
    uint64_t pack_44;
    uint16_t kx;
    uint16_t ky;
    int address_multiplier[4] = {1,-1,-1,1};
    int address_bias_x[4] = {256, 511, 255, 0};
    int address_bias_y[4] = {0, 511, 511, 0};

    inline int which_type(event *packet);
    inline void process_tdc(event *packet);

public:
    inline void process_event();
    inline void process_buffer();
};


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