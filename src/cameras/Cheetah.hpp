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

#ifndef CHEETAH_H
#define CHEETAH_H

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
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

#include "FileConnector.h"
#include "Timepix.hpp"

namespace CHEETAH_ADDITIONAL{
    const size_t BUFFER_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
    using EVENT = uint64_t;
}; //end of namespace


template <typename event, int buffer_size, int n_buffer>
class CHEETAH : public TIMEPIX<event, buffer_size, n_buffer>
{
private:
    // header
    int chip_id;
    uint64_t tpx_header = 861425748; //(b'TPX3', 'little')
    // TDC
    uint64_t rise_t[4];
    uint64_t fall_t[4];
    bool rise_fall[4] = {false, false, false, false};
    int line_count[4];
    // int total_line = 0; // current_line
    int most_advanced_line = 0;
    uint64_t line_interval;
    uint64_t dwell_time;
    // event
    uint64_t toa;
    uint64_t pack_44;
    int address_multiplier[4] = {1,-1,-1,1};
    int address_bias_x[4] = {256, 511, 255, 0};
    int address_bias_y[4] = {0, 511, 511, 0};

    inline bool process_event(event *packet)
    {
        toa = (((*packet & 0xFFFF) << 14) + ((*packet >> 30) & 0x3FFF)) << 4;
        this->probe_position = ( toa - (rise_t[chip_id] * 2)) / dwell_time;
        if (this->probe_position < this->nx)
        {
            uint64_t pack_44 = (*packet >> 44);
            this->probe_position += (line_count[chip_id] % this->ny) * this->nx;
            this->kx = (address_multiplier[chip_id] *
                (((pack_44 & 0x0FE00) >> 8) +
                ((pack_44 & 0x00007) >> 2)) +
                address_bias_x[chip_id]);
            this->ky = (address_multiplier[chip_id] *
                (((pack_44 & 0x001F8) >> 1) +
                (pack_44 & 0x00003)) +
                address_bias_y[chip_id]);
            return true;
        }
        return false;
    };
    inline void process_buffer()
    {
        int type;
        int buffer_id;
        while ((*this->p_processor_line)!=-1)
        {
            if (this->n_buffer_processed < this->n_buffer_filled)
            {
                buffer_id = this->n_buffer_processed % this->n_buf;
                for (int j = 0; j < buffer_size; j++)
                {
                    type = which_type(&(this->buffer[buffer_id])[j]);
                    if ((type == 2) & rise_fall[chip_id] & (line_count[chip_id] != 0))
                    {
                        if (process_event(&(this->buffer[buffer_id])[j]))
                            for (int i_proc=0; i_proc<this->n_proc; i_proc++) { this->process[i_proc](); }
                    }
                }
                ++this->n_buffer_processed;
                *this->p_preprocessor_line = (int)this->current_line;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    };
    inline int which_type(event *packet)
    {
        // if ( (*packet & ((1 << 32) - 1) == tpx_header)  ) {
        if ((*packet & 0xFFFFFFFF) == tpx_header)
        {
            chip_id = (*packet >> 32) & 0xff;
            return 0;
        } // header
        else if (*packet >> 60 == 0x6)
        {
            process_tdc(packet);
            return 1;
        } // TDC
        else if (*packet >> 60 == 0xb)
        {
            return 2;
        } // event
        else
        {
            return 3;
        } // unknown
    };
    inline void process_tdc(event *packet)
    {
        if (((*packet >> 56) & 0x0F) == 15)
        {
            rise_fall[chip_id] = true;
            rise_t[chip_id] = (((*packet >> 9) & 0x7FFFFFFFF) % 8589934592);
        }
        else if (((*packet >> 56) & 0x0F) == 10)
        {
            rise_fall[chip_id] = false;
            fall_t[chip_id] = ((*packet >> 9) & 0x7FFFFFFFF) % 8589934592;
            ++line_count[chip_id];

            if ((line_count[chip_id] <= line_count[0]) &
                (line_count[chip_id] <= line_count[1]) &
                (line_count[chip_id] <= line_count[2]) &
                (line_count[chip_id] <= line_count[3]))
            { this->current_line = line_count[chip_id]; }
            else if (line_count[chip_id] >= most_advanced_line)
            {
                most_advanced_line = line_count[chip_id];
                if (most_advanced_line%this->ny == 0)
                {
                    this->id_image = most_advanced_line / this->ny % 2;
                    this->flush_image(this->id_image);
                }
            }

            line_interval = (fall_t[chip_id] - rise_t[chip_id]) * 2;
            dwell_time = line_interval / this->nx;
        }
    };
    
    void reset()
    {
        TIMEPIX<event, buffer_size, n_buffer>::reset();
        for (int i = 0; i < 4; i++)
        {
            rise_fall[i] = false;
            line_count[i] = 0;
        }
        most_advanced_line = 0;
    };

public:
    void run()
    {
        reset();
        switch (this->mode)
        {
            case 0:
            {
                this->file.path = this->file_path;
                this->file.open_file();
                this->read_guy = std::thread(&CHEETAH<event, buffer_size, n_buffer>::read_file, this);
                break;
            }
            case 1:
            {
                this->p_socket->socket_type = Socket_type::SERVER;
                this->p_socket->accept_socket();
                this->read_guy = std::thread(&CHEETAH<event, buffer_size, n_buffer>::read_socket, this);
                break;
            }
        }
        this->proc_guy = std::thread(&CHEETAH<event, buffer_size, n_buffer>::process_buffer, this);
    };

    CHEETAH(
        int &nx,
        int &ny,
        int &n_cam,
        int &dt, // unit: ns
        bool &b_vSTEM,
        bool &b_ricom,
        bool &b_e_mag,
        bool &b_airpi,
        bool *b_cumulative,
        std::array<float, 2> *p_radius,
        std::array<float, 2> *p_offset,
        std::vector<size_t> (*p_dose_data)[2],
        std::vector<size_t> (*p_sumx_data)[2],
        std::vector<size_t> (*p_sumy_data)[2],
        std::vector<size_t> (*p_stem_data)[2],
        std::vector<size_t> *p_frame,
        int *p_processor_line,
        int *p_preprocessor_line,
        int &mode,
        std::string &file_path,  
        SocketConnector *p_socket
    ) : TIMEPIX<event, buffer_size, n_buffer>(
            nx,
            ny,
            n_cam,
            dt,
            b_vSTEM,
            b_ricom,
            b_e_mag,
            b_airpi,
            b_cumulative,
            p_radius,
            p_offset,
            p_dose_data,
            p_sumx_data,
            p_sumy_data,
            p_stem_data,
            p_frame,
            p_processor_line,
            p_preprocessor_line,
            mode,
            file_path,
            p_socket
        ) {}
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
    void tpx3_det_config()
    {
        cpr::Response r = cpr::Get(cpr::Url{serverurl + "/detector/config"});
        nlohmann::json detectorConfig = nlohmann::json::parse(r.text);
        detectorConfig["BiasVoltage"] = "100";
        detectorConfig["BiasEnabled"] = "True";
        detectorConfig["TriggerMode"] = "CONTINUOUS";
        detectorConfig["TriggerPeriod"] = "0.016";
        detectorConfig["ExposureTime"] = "0.016";
        detectorConfig["nTriggers"] = "999999999";
        std::string config_string = detectorConfig.dump();
        cpr::Response rr = cpr::Put(
            cpr::Url{serverurl + "/detector/config"},
            cpr::Body{config_string});
        std::cout << "GET-detector config:" << r.text << std::endl;
        std::cout << "PUT-detector config:" << rr.text << std::endl;
    };
    void tpx3_cam_init()
    {
        cpr::Response bpc = cpr::Get(cpr::Url{serverurl + "/config/load?format=pixelconfig&file=/home/asi/Desktop/ricom/ricom/Factory_Settings.bpc"});
        cpr::Response dac = cpr::Get(cpr::Url{serverurl + "/config/load?format=dacs&file=/home/asi/Desktop/ricom/ricom/Factory_Settings.dacs"});
        std::cout << "GET-bpc:" << bpc.text << std::endl;
        std::cout << "GET-dac:" << dac.text << std::endl;
    };
    void tpx3_destination()
    {
        std::string destination = R"({
            "Raw": [{
                "Base": "tcp://connect@127.0.0.1:8451"
            }]
        })";

        cpr::Response r = cpr::Put(
            cpr::Url{serverurl + "/server/destination"},
            cpr::Body{destination});
        std::cout << "PUT-destination:" << r.text << std::endl;
    };
    void tpx3_acq_init(){};
    void start()
    {
        cpr::Response r = cpr::Get(cpr::Url{serverurl + "/measurement/start"});
        std::cout << "Get-start:" << r.text << std::endl;
    };
    void stop()
    {
        cpr::Response r = cpr::Get(cpr::Url{serverurl + "/measurement/stop"});
        std::cout << "Get-stop:" << r.text << std::endl;
    };
};

#endif // CHEETAH_H