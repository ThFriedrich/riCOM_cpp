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

#ifndef CHEETAH_INTERFACE_H
#define CHEETAH_INTERFACE_H

// #ifdef __GNUC__
// #define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
// #endif

// #ifdef _MSC_VER
// #define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
// #endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <atomic>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "SocketConnector.h"
#include "FileConnector.h"

namespace CheetahBuffer{
    const size_t PRELOC_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
}

class CheetahInterface
{
private:
    SocketConnector *socket;
    FileConnector file;
    int sleep = 1;
    // read buffer
    std::thread read_guy;
    bool read_started = false;
    // process buffer
    std::thread proc_guy;
    bool proc_started = false;
    int preloc_size = CheetahBuffer::PRELOC_SIZE;
    int n_buffer = CheetahBuffer::N_BUFFER;
    std::array<std::array<uint64_t, CheetahBuffer::PRELOC_SIZE>, CheetahBuffer::N_BUFFER> buffer;
    int n_buffer_filled=0;
    int n_buffer_processed=0;
    // header
    int chip_id;
    uint64_t tpx_header = 861425748; //(b'TPX3', 'little')
    // TDC
    uint64_t rise_t[4];
    uint64_t fall_t[4];
    bool rise_fall[4] = {false, false, false, false};
    int line_count[4];
    int total_line = 0;
    int most_advanced_line = 0;
    uint64_t line_interval;
    uint64_t dwell_time;
    bool started = false;
    // event
    uint64_t probe_position;
    uint16_t kx;
    uint16_t ky;
    int address_multiplier[4] = {1,-1,-1,1};
    int address_bias_x[4] = {256, 511, 255, 0};
    int address_bias_y[4] = {0, 511, 511, 0};

    // read methods
    inline void read_file(FileConnector *file, int *bc);
    inline void read_socket(SocketConnector *socket, int *bc);

    // process methods
    void real_read(       
        std::vector<size_t> &dose_map, 
        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
        std::vector<float> &stem_map, bool &b_stem,
        std::array<float, 2> &offset, std::array<float, 2> &radius,
        std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
        int &processor_line, int &preprocessor_line, bool b_cbed
    );

    inline void process_buffer(
        std::vector<size_t> *dose_map, 
        std::vector<size_t> *sumx_map, 
        std::vector<size_t> *sumy_map, 
        std::vector<float> *stem_map, 
        bool *b_stem, 
        std::array<float, 2> *offset, 
        std::array<float, 2> *radius, 
        std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed,
        int *processor_line, int *preprocessor_line, bool *b_cbed
    );

    int which_type(uint64_t *packet, std::vector<float> *stem_map);
    void process_tdc(uint64_t *packet, std::vector<float> *stem_map);
    void process_event(
        uint64_t *packet,    
        std::vector<size_t> *dose_map, 
        std::vector<size_t> *sumx_map, 
        std::vector<size_t> *sumy_map, 
        std::vector<float> *stem_map, 
        bool* b_stem,
        std::array<float, 2> *offset, 
        std::array<float, 2> *radius
    );

    void process_event(
        uint64_t *packet,    
        std::vector<size_t> *dose_map, 
        std::vector<size_t> *sumx_map, 
        std::vector<size_t> *sumy_map, 
        std::vector<float> *stem_map, 
        bool* b_stem,
        std::array<float, 2> *offset, 
        std::array<float, 2> *radius,
        std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed
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
    int gs;
    
public:
    // scan
    uint32_t scan_x;
    uint32_t scan_y;
    bool finish = false;
    
    void read_frame_com(
        std::vector<size_t> &dose_map, 
        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map, 
        std::vector<float> &stem_map, bool &b_stem,
        std::array<float, 2> &offset, std::array<float, 2> &radius,
        int &processor_line, int &preprocessor_line
    );
                   
    void read_frame_com(
        std::vector<size_t> &dose_map,
        std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
        std::vector<float> &stem_map, bool &b_stem,
        std::array<float, 2> &offset, std::array<float, 2> &radius,
        std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
        int &processor_line, int &preprocessor_line
    );

    void init_interface(std::string &tpx3_path);
    void init_interface(SocketConnector *socket);
    void close_interface();

};

class CheetahComm
{
private:
    std::string serverip = "localhost";
    std::string serverport = "8080";
    std::string rawip = "127.0.0.1";
    std::string rawport = "8451";
    std::string serverurl = "http://" + serverip + ":" + serverport;

public:
    void tpx3_det_config();
    void tpx3_cam_init();
    void tpx3_destination();
    void tpx3_acq_init();
    void start();
    void stop();
};


#endif // CHEETAH_INTERFACE_H
