#ifndef TI_H
#define TI_H

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "ricom_types.h"
#pragma pack(1)

struct e_event
    {
        unsigned int index;
        unsigned long toa;
        unsigned char overflow;
        unsigned char ftoa;
        unsigned short tot;
    };

class TimpixInterface
{
private:
    void open_file( std::string path );
    
public:
    void read_data_com_ti( std::vector<uint32_t> &dose_map, 
        std::vector<uint32_t> &sumx_map, std::vector<uint32_t> &sumy_map, int img_size );
    void read_data_from_file_ti( e_event &ev);
    void init_ti( std::string path );
    std::string t3p_path;
    std::ifstream t3p_stream;
    size_t nx_timpix = 256;
    size_t ny_timpix = 256;
    size_t ds_timpix = 256 * 256; 
    size_t dwell_time = 0; // unit: ns
    int probe_position_now = 0;
};
#endif // __TI_H__