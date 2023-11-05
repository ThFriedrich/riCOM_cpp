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

#include "Cheetah.h"
using namespace CHEETAH_ADDITIONAL;

void CHEETAH::run()
{
    reset();
    switch (mode)
    {
        case 0:
        {
            file.path = file_path;
            file.open_file();
            read_guy = std::thread(&CHEETAH::read_file, this);
            break;
        }
        case 1:
        {
            p_socket->socket_type = Socket_type::SERVER;
            p_socket->accept_socket();
            read_guy = std::thread(&CHEETAH::read_socket, this);
            break;
        }
    }
    proc_guy = std::thread(&CHEETAH::process_buffer, this);
    // read_guy.detach();
    // proc_guy.detach();

}

inline void CHEETAH::read_file()
{
    int buffer_id;
    while (*p_processor_line!=-1)
    {
        if ( (n_buffer_filled < (n_buffer + n_buffer_processed)) && (*p_preprocessor_line < (*p_processor_line + (int)(ny/2))) ) 
        {
            buffer_id = n_buffer_filled % n_buffer;
            file.read_data((char *)&(buffer[buffer_id]), sizeof(buffer[buffer_id]));
            ++n_buffer_filled;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    file.close_file();
}

inline void CHEETAH::read_socket()
{
    int buffer_id;
    while (*p_processor_line != -1)
    {
        if (n_buffer_filled < (n_buffer + n_buffer_processed))
        {
            buffer_id = n_buffer_filled % n_buffer;
            socket->read_data((char *)&(buffer[buffer_id]), sizeof(buffer[buffer_id]));
            ++n_buffer_filled;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    std::cout << "n_buffer_filled" << std::endl;
}


inline void CHEETAH::process_buffer()
{
    int type;
    int buffer_id;
    while ((*p_processor_line)!=-1)
    {
        if (n_buffer_processed < n_buffer_filled)
        {
            buffer_id = n_buffer_processed % n_buffer;
            for (int j = 0; j < buffer_size; j++)
            {
                type = which_type(&(buffer[buffer_id])[j]);
                if ((type == 2) & rise_fall[chip_id] & (line_count[chip_id] != 0))
                {
                    process_event(&(buffer[buffer_id])[j]);
                    for (int i_proc=0; i_proc<n_proc; i_proc++) { process[i_proc](); }
                }
            }
            ++n_buffer_processed;
            *p_preprocessor_line = (int)current_line;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

inline void CHEETAH::process_event(CHEETAH_ADDITIONAL::EVENT *packet)
{
    toa = (((*packet & 0xFFFF) << 14) + ((*packet >> 30) & 0x3FFF)) << 4;
    probe_position = ( toa - (rise_t[chip_id] * 2)) / dwell_time;
    if (probe_position < nx)
    {
        uint64_t pack_44 = (*packet >> 44);
        probe_position += (line_count[chip_id] % ny) * nx;
        kx = (address_multiplier[chip_id] *
            (((pack_44 & 0x0FE00) >> 8) +
            ((pack_44 & 0x00007) >> 2)) +
            address_bias_x[chip_id]);
        ky = (address_multiplier[chip_id] *
            (((pack_44 & 0x001F8) >> 1) +
            (pack_44 & 0x00003)) +
            address_bias_y[chip_id]);
    }
}

int CHEETAH::which_type(CHEETAH_ADDITIONAL::EVENT *packet)
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
}


void CHEETAH::process_tdc(CHEETAH_ADDITIONAL::EVENT *packet)
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
        { current_line = line_count[chip_id]; }
        else if (line_count[chip_id] >= most_advanced_line)
        {
            most_advanced_line = line_count[chip_id];
            flush_next_line(most_advanced_line);
        }

        line_interval = (fall_t[chip_id] - rise_t[chip_id]) * 2;
        dwell_time = line_interval / nx;
    }
}

void CHEETAH::reset()
{
    n_buffer_filled = 0;
    n_buffer_processed = 0;
    for (int i = 0; i < 4; i++)
    {
        rise_fall[i] = false;
        line_count[i] = 0;
    }
    current_line = 0;
    most_advanced_line = 0;
}


// ------------- CheetahComm -------------- //
void CheetahComm::tpx3_det_config()
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
}

void CheetahComm::tpx3_cam_init()
{
    cpr::Response bpc = cpr::Get(cpr::Url{serverurl + "/config/load?format=pixelconfig&file=/home/asi/Desktop/ricom/ricom/Factory_Settings.bpc"});
    cpr::Response dac = cpr::Get(cpr::Url{serverurl + "/config/load?format=dacs&file=/home/asi/Desktop/ricom/ricom/Factory_Settings.dacs"});
    std::cout << "GET-bpc:" << bpc.text << std::endl;
    std::cout << "GET-dac:" << dac.text << std::endl;
}

void CheetahComm::tpx3_destination()
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
}

void CheetahComm::CheetahComm::start()
{
    cpr::Response r = cpr::Get(cpr::Url{serverurl + "/measurement/start"});
    std::cout << "Get-start:" << r.text << std::endl;
}
void CheetahComm::CheetahComm::stop()
{
    cpr::Response r = cpr::Get(cpr::Url{serverurl + "/measurement/stop"});
    std::cout << "Get-stop:" << r.text << std::endl;
}

// ---------------------------------------- //
