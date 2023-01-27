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

#ifndef CHEETAH_INTERFAC_H
#define CHEETAH_INTERFAC_H

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
#include <algorithm>

#include "SocketConnector.h"
#include "FileConnector.h"

PACK(struct e_event
     {
         uint32_t kx;
         uint32_t ky;
         uint32_t rx;
         uint32_t ry;
     });

class CheetahInterface
{
private:
    SocketConnector *socket;
    FileConnector file;
    e_event carryover_ev = {0,0,0,0};
    int data_threshold = 1024; // when reach data_threshold, fill_buffer stop at the beginning of next frame
    int frame_upperbound = 1024;
    int max_frame;

protected:
    enum Mode
    {
        MODE_FILE,
        MODE_TCP
    };
    Mode mode;

    int nx;
    int ny;
    int n_worker;

public:
    int user_buffer_size = INT32_MAX;
    void fill_buffer(std::vector<e_event> &buffer);
    void set_max_frame();

    inline void read_event(e_event &ev);
    //void init_interface(std::string &t3p_path);
    //void close_interface();

    CheetahInterface() : nx(512), ny(512){
        set_max_frame();
    };
};
#endif // CHEETAH_INTERFAC_H