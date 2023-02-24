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

#include "TimepixInterface.h"

// Read a frame and compute COM
void TimepixInterface::read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                      std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                      std::vector<float> &stem_map, bool b_stem,
                                      std::array<float, 2> &offset, std::array<float, 2> &radius2,
                                      size_t first_frame, size_t end_frame)
{
    e_event ev;

    while (true)
    {
        read_event(ev);
        size_t probe_position = floor(ev.toa * 25 / dt);
        if (probe_position >= first_frame && probe_position < end_frame)
        {
            size_t x = ev.index % nx;
            size_t y = floor(ev.index / nx);
            size_t probe_position2 = probe_position - first_frame;
            dose_map[probe_position2]++;
            sumx_map[probe_position2] += x;
            sumy_map[probe_position2] += y;
            if (b_stem)
            {
                float d2 = pow((float)x - offset[0], 2) + pow((float)y - offset[1], 2);
                if (d2 > radius2[0] && d2 <= radius2[1])
                {
                    stem_map[probe_position2]++;
                }
            }
        }
        if (probe_position > idx)
        {
            break;
        }
    }
}

// Read a frame and compute COM and create frame representation
template <typename T>
void TimepixInterface::read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                      std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                      std::vector<float> &stem_map, bool b_stem,
                                      std::array<float, 2> &offset, std::array<float, 2> &radius2,
                                      std::vector<T> &frame, size_t frame_id,
                                      size_t first_frame, size_t end_frame)
{
    e_event ev;

    while (true)
    {
        read_event(ev);
        size_t probe_position = floor(ev.toa * 25 / dt);
        if (probe_position >= first_frame && probe_position < end_frame) //  && ev.toa * 25 % dt < dt/10
        {
            size_t x = ev.index % nx;
            size_t y = floor(ev.index / nx);
            size_t probe_position2 = probe_position - first_frame;
            dose_map[probe_position2]++;
            sumx_map[probe_position2] += x;
            sumy_map[probe_position2] += y;
            if (probe_position == frame_id)
                frame[x + y * nx]++;
            if (b_stem)
            {
                float d2 = pow((float)x - offset[0], 2) + pow((float)y - offset[1], 2);
                if (d2 > radius2[0] && d2 <= radius2[1])
                {
                    stem_map[probe_position2]++;
                }
            }
        }
        if (probe_position > idx)
        {
            break;
        }
    }
}
template void TimepixInterface::read_frame_com<uint8_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                                        std::vector<float> &stem_map, bool b_stem,
                                                        std::array<float, 2> &offset, std::array<float, 2> &radius,
                                                        std::vector<uint8_t> &frame, size_t frame_id,
                                                        size_t first_frame, size_t end_frame);
template void TimepixInterface::read_frame_com<uint16_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                         std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                                         std::vector<float> &stem_map, bool b_stem,
                                                         std::array<float, 2> &offset, std::array<float, 2> &radius,
                                                         std::vector<uint16_t> &frame, size_t frame_id,
                                                         size_t first_frame, size_t end_frame);
template void TimepixInterface::read_frame_com<uint32_t>(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                                                         std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                                                         std::vector<float> &stem_map, bool b_stem,
                                                         std::array<float, 2> &offset, std::array<float, 2> &radius,
                                                         std::vector<uint32_t> &frame, size_t frame_id,
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

void TimepixInterface::init_interface(const std::string &t3p_path)
{
    mode = MODE_FILE;
    file.path = t3p_path;
    file.open_file();
};

void TimepixInterface::close_interface()
{
    file.close_file();
};