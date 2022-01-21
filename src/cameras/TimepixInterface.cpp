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

#include "TimepixInterface.h"

// Read a frame and compute COM
void TimepixInterface::read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                                  size_t first_frame, size_t end_frame)
{
    e_event ev;
    size_t probe_position;

    while (true)
    {
        read_event(ev);
        probe_position = floor(ev.toa * 25 / dt);
        if (probe_position >= first_frame && probe_position < end_frame) //  && ev.toa * 25 % dt < dt/10
        {
            dose_map[probe_position - first_frame]++;
            sumx_map[probe_position - first_frame] += ev.index % nx;
            sumy_map[probe_position - first_frame] += floor(ev.index / nx);
        }
        if (probe_position > idx)
        {
            break;
        }
    }
}

// Read a frame and compute COM and create frame representation
template<typename T>
void TimepixInterface::read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                                  std::vector<T> &frame, 
                                  size_t first_frame, size_t end_frame)
{
    e_event ev;
    size_t probe_position;
    frame.assign(nx * ny, 0);
    unsigned int x, y;

    while (true)
    {
        read_event(ev);
        probe_position = floor(ev.toa * 25 / dt);
        if (probe_position >= first_frame && probe_position < end_frame) //  && ev.toa * 25 % dt < dt/10
        {
            x = ev.index % nx;
            y = floor(ev.index / nx);
            dose_map[probe_position - first_frame]++;
            sumx_map[probe_position - first_frame] += x;
            sumy_map[probe_position - first_frame] += y;
            frame[x + y * nx]++;
        }
        if (probe_position > idx)
        {
            break;
        }
    }
}
template void TimepixInterface::read_frame_com<uint8_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                                                  std::vector<uint8_t> &frame, 
                                                  size_t first_frame, size_t end_frame);
template void TimepixInterface::read_frame_com<uint16_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                                                  std::vector<uint16_t> &frame, 
                                                  size_t first_frame, size_t end_frame);
template void TimepixInterface::read_frame_com<uint32_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                                                  std::vector<uint32_t> &frame, 
                                                  size_t first_frame, size_t end_frame);
void TimepixInterface::read_event(e_event &ev)
{
    switch (mode)
    {
    case MODE_FILE:
        file.read_data((char *)&ev, sizeof(ev));
        break;
    case MODE_TCP: // not implemented yet
        break;
    }
}

void TimepixInterface::init_interface(std::string &t3p_path)
{
    mode = MODE_FILE;
    file.path = t3p_path;
    file.open_file();
};

void TimepixInterface::close_interface()
{
    file.close_file();
};