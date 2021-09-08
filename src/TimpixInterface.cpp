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
void TimpixInterface::open_file(std::string path)
{
    t3p_stream.open(path, std::ios::in | std::ios::binary);
    if (!t3p_stream.is_open())
    {
        std::cout << "Cannot open T3P file!" << std::endl;
    }
}

// public methods
inline void TimpixInterface::read_data_from_file_ti(RICOM::e_event &ev)
{
    t3p_stream.read((char *)&ev, sizeof(ev));
}

void TimpixInterface::read_data_com_ti(std::vector<uint32_t> &dose_map,
                                       std::vector<uint32_t> &sumx_map, std::vector<uint32_t> &sumy_map, int img_size)
{
    RICOM::e_event ev;
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

void TimpixInterface::init_ti(std::string path)
{
    t3p_path = path;
    open_file(t3p_path);
}
