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

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <atomic>

#include "TimpixInterface.h"

// private methods
void TimpixInterface::open_file()
{
    t3p_stream.open(t3p_path, std::ios::in | std::ios::binary);
    if (!t3p_stream.is_open())
    {
        std::cout << "Cannot open T3P file!" << std::endl;
    }
}

// public methods
inline void TimpixInterface::read_data_from_file_ti(e_event &ev)
{
    t3p_stream.read((char *)&ev, sizeof(ev));
}

void TimpixInterface::read_com_ti(std::atomic<unsigned int> &idx, std::vector<size_t> &dose_map,
                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, int unsigned first_frame, unsigned int end_frame)
{
    e_event ev;
    unsigned int probe_position;

    while (true)
    {
        read_data_from_file_ti(ev);
        probe_position = floor(ev.toa*25 / dwell_time);
        if (probe_position >= first_frame && probe_position < end_frame) //  && ev.toa * 25 % dwell_time < dwell_time/10
        {
            dose_map[probe_position - first_frame]++;
            sumx_map[probe_position - first_frame] += ev.index % nx_timpix;
            sumy_map[probe_position - first_frame] += floor(ev.index / nx_timpix);
        }
        if (probe_position > idx)
        {
            idx++;
            break;
        }
    }
}

void TimpixInterface::timepix_init(RICOM::modes mode)
{
    this->mode = mode;
    switch (mode)
    {
    case RICOM::LIVE:
    {
        // connect_socket();
        break;
    }
    case RICOM::FILE:
    {
        open_file();
        break;
    }
    default:
        break;
    }
};

void TimpixInterface::timepix_end()
{
    if (mode == RICOM::FILE)
    {
        close_file();
    }
    else
    {
        // close_socket();
    }
};

void TimpixInterface::close_file()
{
    if (t3p_stream.is_open())
    {
        t3p_stream.close();
    }
}