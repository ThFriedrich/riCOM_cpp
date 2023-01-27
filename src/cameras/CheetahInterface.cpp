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

#include "CheetahInterface.h"

void CheetahInterface::fill_buffer(std::vector<e_event> &buffer)
{
    e_event ev;
    int cnt_frame = 0;
    int cnt_data = 0;
    bool threshold_reached = false;
    uint32_t probe_position[2] = {carryover_ev.rx, carryover_ev.ry};

    if ((probe_position[0] != 0) || (probe_position[1] != 0))
    {
        buffer[0] = carryover_ev;
        ++cnt_data;
    }

    while (true)
    {
        read_event(ev);
        if ((ev.rx != probe_position[0]) || (ev.ry != probe_position[1]))
        {
            probe_position[0] = ev.rx;
            probe_position[1] = ev.ry;
            ++cnt_frame;
        }

        if (cnt_frame > max_frame)
        {
            carryover_ev = ev;
            break;
        }

        buffer[cnt_data] = ev;
        ++cnt_data;
        
        if ((cnt_data > data_threshold) && !(threshold_reached))
        {
            max_frame = cnt_frame + 1;
        }
    }

}

void CheetahInterface::set_max_frame()
{
    if (user_buffer_size < frame_upperbound) {max_frame = user_buffer_size;}
    else {max_frame = frame_upperbound;}
}

void CheetahInterface::read_event(e_event &ev)
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







