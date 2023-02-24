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

#ifndef TIMEPIX_INTERFACE_H
#define TIMEPIX_INTERFACE_H

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

#include "FileConnector.h"

PACK(struct e_event
     {
         uint32_t index;
         uint64_t toa;
         uint8_t overflow;
         uint8_t ftoa;
         uint16_t tot;
     });

class TimepixInterface
{
private:
    FileConnector file;

protected:
    enum Mode
    {
        MODE_FILE,
        MODE_TCP
    };
    Mode mode;
    int nx;
    int ny;
    int dt; // unit: ns

public:
    void read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
                        std::vector<float> &stem_map, bool b_stem,
                        std::array<float, 2> &offset, std::array<float, 2> &radius,
                        size_t first_frame, size_t end_frame
                        );

    template <typename T>                    
    void read_frame_com(std::atomic<size_t> &idx, std::vector<size_t> &dose_map,
                        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
                        std::vector<float> &stem_map, bool b_stem,
                        std::array<float, 2> &offset, std::array<float, 2> &radius,
                        std::vector<T> &frame, size_t frame_id,
                        size_t first_frame, size_t end_frame);

    inline void read_event(e_event &ev);
    void init_interface(const std::string &t3p_path);
    void close_interface();

    TimepixInterface() : mode(MODE_FILE), nx(256), ny(256), dt(1000){};
};
#endif // TIMEPIX_INTERFACE_H