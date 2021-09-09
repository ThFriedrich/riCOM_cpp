#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

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

void TimpixInterface::read_com_ti(std::vector<size_t> &dose_map,
                                  std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, size_t img_size)
{
    e_event ev;
    int probe_position;

    for (size_t idx = 0; idx < ds_timpix; idx++)
    {
        read_data_from_file_ti(ev);
        probe_position = floor(ev.toa / dwell_time);

        if (probe_position >= 0 && probe_position <= img_size)
        {
            dose_map[probe_position]++;
            sumx_map[probe_position] += ev.index % nx_timpix;
            sumy_map[probe_position] += floor(ev.index / nx_timpix);
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