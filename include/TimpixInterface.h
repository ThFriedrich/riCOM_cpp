#ifndef TI_H
#define TI_H

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "ricom_types.h"

PACK(struct e_event
{
    uint32_t index;
    uint64_t toa;
    uint8_t overflow;
    uint8_t ftoa;
    uint16_t tot;
});

class TimpixInterface
{
private:
    RICOM::modes mode;
    void open_file();
    std::ifstream t3p_stream;
    size_t ds_timpix;
    
public:
    void read_com_ti(int &idx, std::vector<size_t> &dose_map,
                     std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, int first_frame, int end_frame);
    void read_data_from_file_ti(e_event &ev);
    void timepix_init(RICOM::modes mode);
    void close_file();
    void timepix_end();
    
    std::string t3p_path;
    size_t nx_timpix;
    size_t ny_timpix;
    int dwell_time; // unit: ns
    int probe_position_now;

    TimpixInterface() : mode(), t3p_stream(), ds_timpix(65536), t3p_path(), nx_timpix(256), ny_timpix(256), dwell_time(1000), probe_position_now(0){};
};
#endif // __TI_H__