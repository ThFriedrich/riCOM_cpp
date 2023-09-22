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
#include <thread>
#include <chrono>

#include "FileConnector.h"

PACK(struct e_event
     {
         uint32_t index;
         uint64_t toa;
         uint8_t overflow;
         uint8_t ftoa;
         uint16_t tot;
     });

const size_t PRELOC_SIZE = 1000*25;
const size_t N_BUFFER = 4;



class TimepixInterface
{
private:
    FileConnector file;
    int sleep = 1;
    // read buffer
    std::thread read_guy;
    bool read_started = false;
    // process buffer
    std::thread proc_guy;
    bool proc_started = false;
    int preloc_size = PRELOC_SIZE;
    int n_buffer = N_BUFFER;
    std::array<std::array<e_event, PRELOC_SIZE>, N_BUFFER> buffer;
    int n_buffer_filled=0;
    int n_buffer_processed=0;
    uint64_t current_line = 0;
    // event
    uint64_t probe_position;
    uint64_t probe_position_total;
    uint16_t kx;
    uint16_t ky;

    // read methods
    inline void read_file(FileConnector *file, int *processor_line, int *preprocessor_line);
    
    // process methods
    inline void process_buffer(
        std::vector<size_t> *dose_map,
        std::vector<size_t> *sumx_map,
        std::vector<size_t> *sumy_map,
        std::vector<float> *stem_map,
        bool *b_stem,
        std::array<float, 2> *offset,
        std::array<float, 2> *radius,
        std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed,
        int *processor_line, int *preprocessor_line, size_t *first_frame, size_t *end_frame, bool *b_cbed
    );
    void process_event(
        e_event *packet,
        std::vector<size_t> *dose_map,
        std::vector<size_t> *sumx_map,
        std::vector<size_t> *sumy_map,
        std::vector<float> *stem_map,
        bool *b_stem,
        std::array<float, 2> *offset,
        std::array<float, 2> *radius
    );
    void process_event(
        e_event *packet,
        std::vector<size_t> *dose_map,
        std::vector<size_t> *sumx_map,
        std::vector<size_t> *sumy_map,
        std::vector<float> *stem_map,
        bool *b_stem,
        std::array<float, 2> *offset,
        std::array<float, 2> *radius,
        std::vector<size_t> *frame, 
        std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed
    );

    inline void reset();

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
    uint32_t gs;

public:
    void read_frame_com(
        std::vector<size_t> &dose_map,
        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
        std::vector<float> &stem_map, bool b_stem,
        std::array<float, 2> &offset, std::array<float, 2> &radius,
        int &processor_line, int &preprocessor_line, size_t &first_frame, size_t &end_frame
    );

    void read_frame_com(    
        std::vector<size_t> &dose_map,
        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
        std::vector<float> &stem_map, bool b_stem,
        std::array<float, 2> &offset, std::array<float, 2> &radius,
        std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
        int &processor_line, int &preprocessor_line, size_t &first_frame, size_t &end_frame
    );
    void init_interface(const std::string &t3p_path);
    void close_interface();

    // scan
    uint32_t scan_x;
    uint32_t scan_y;
    uint32_t scan_n;
    bool finish = false;

    TimepixInterface() : mode(MODE_FILE), nx(256), ny(256), dt(1000){};
};
#endif // TIMEPIX_INTERFACE_H