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

const bool b_false = false;
const bool b_true = true;
std::vector<size_t> dummy_frame(1);
std::array<std::atomic<size_t>, 3> dummy_frame_id_plot_cbed;

void CheetahInterface::read_frame_com(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool &b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    int &processor_line, int &preprocessor_line)
{
    real_read(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, dummy_frame, dummy_frame_id_plot_cbed, processor_line, preprocessor_line, b_false);
}

void CheetahInterface::read_frame_com(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool &b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
    int &processor_line, int &preprocessor_line)
{
    real_read(dose_map, sumx_map, sumy_map, stem_map, b_stem, offset, radius, frame, frame_id_plot_cbed, processor_line, preprocessor_line, b_true);
}

void CheetahInterface::real_read(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map,
    std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map,
    bool &b_stem,
    std::array<float, 2> &offset,
    std::array<float, 2> &radius,
    std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
    int &processor_line, int &preprocessor_line, bool b_cbed)
{
    if (!read_started)
    {
        reset();
        read_started = true;
        switch (mode)
        {
        case MODE_FILE:
            read_guy = std::thread(&CheetahInterface::read_file, this, &file, &processor_line);
            break;
        case MODE_TCP:
            socket->accept_socket();
            read_guy = std::thread(&CheetahInterface::read_socket, this, socket, &processor_line);
            break;
        }
        read_guy.detach();
    }

    if (!proc_started)
    {
        proc_started = true;
        proc_guy = std::thread(&CheetahInterface::process_buffer, this, &dose_map, &sumx_map, &sumy_map, &stem_map, &b_stem, &offset, &radius, &frame, &frame_id_plot_cbed, &processor_line, &preprocessor_line, &b_cbed);
        proc_guy.detach();
    }

    if (finish & read_guy.joinable())
    {
        read_guy.join();
    }
    if (finish & proc_guy.joinable())
    {
        proc_guy.join();
    }
}

void CheetahInterface::read_file(FileConnector *file, int *bc)
{
    int buffer_id;
    while (*bc != -1)
    {
        if (n_buffer_filled < (n_buffer + n_buffer_processed))
        {
            buffer_id = n_buffer_filled % n_buffer;
            file->read_data((char *)&buffer[buffer_id], sizeof(buffer[buffer_id]));
            ++n_buffer_filled;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void CheetahInterface::read_socket(SocketConnector *socket, int *bc)
{
    while (*bc != -1)
    {
        if (n_buffer_filled < (n_buffer + n_buffer_processed))
        {
            int buffer_id = n_buffer_filled % n_buffer;
            socket->read_data((char *)&buffer[buffer_id], sizeof(buffer[buffer_id]));
            ++n_buffer_filled;
            // std::cout << n_buffer_filled << std::endl;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            printf("read wait\n");
        }
    }
    std::cout << "n_buffer_filled" << std::endl;
}

void CheetahInterface::process_buffer(
    std::vector<size_t> *dose_map,
    std::vector<size_t> *sumx_map,
    std::vector<size_t> *sumy_map,
    std::vector<float> *stem_map,
    bool *b_stem,
    std::array<float, 2> *offset,
    std::array<float, 2> *radius,
    std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed,
    int *processor_line, int *preprocessor_line, bool *b_cbed)
{
    int type;
    int buffer_id;

    while (*processor_line != -1)
    {
        // std::cout << n_buffer_filled << "," << n_buffer_processed << std::endl;
        if (n_buffer_processed < n_buffer_filled)
        {
            buffer_id = n_buffer_processed % n_buffer;
            for (int j = 0; j < preloc_size; j++)
            {
                type = which_type(&buffer[buffer_id][j], stem_map);
                if ((type == 2) & rise_fall[chip_id] & (line_count[chip_id] != 0))
                {
                    // if (*b_cbed)
                    // {
                    //     process_event(
                    //         &buffer[buffer_id][j],
                    //         dose_map, sumx_map, sumy_map,
                    //         stem_map, b_stem,
                    //         offset, radius, frame, frame_id_plot_cbed);
                    // }
                    // else
                    // {
                    //     process_event(
                    //         &buffer[buffer_id][j],
                    //         dose_map, sumx_map, sumy_map,
                    //         stem_map, b_stem,
                    //         offset, radius);
                    // }
                    process_event(
                        &buffer[buffer_id][j],
                        dose_map, sumx_map, sumy_map,
                        stem_map, b_stem,
                        offset, radius);
                }
            }
            ++n_buffer_processed;
            *preprocessor_line = total_line;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // printf("slow read\n");
        }
    }
}

int CheetahInterface::which_type(uint64_t *packet, std::vector<float> *stem_map)
{
    // if ( (*packet & ((1 << 32) - 1) == tpx_header)  ) {
    if ((*packet & 0xFFFFFFFF) == tpx_header)
    {
        chip_id = (*packet >> 32) & 0xff;
        return 0;
    } // header
    else if (*packet >> 60 == 0x6)
    {
        process_tdc(packet, stem_map);
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

void CheetahInterface::process_tdc(uint64_t *packet, std::vector<float> *stem_map)
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
        // line_count[chip_id] %= scan_y;

        if ((line_count[chip_id] <= line_count[0]) &
            (line_count[chip_id] <= line_count[1]) &
            (line_count[chip_id] <= line_count[2]) &
            (line_count[chip_id] <= line_count[3]))
        { total_line = line_count[chip_id]; }

        if ((line_count[chip_id] >= line_count[0]) &
            (line_count[chip_id] >= line_count[1]) &
            (line_count[chip_id] >= line_count[2]) &
            (line_count[chip_id] >= line_count[3]))
        {
            most_advanced_line = line_count[chip_id];
            if ( (most_advanced_line+3)%scan_y < (most_advanced_line+2)%scan_y ){
                std::fill(
                    (*stem_map).begin()+(((most_advanced_line+2)%scan_y)*scan_x), 
                    (*stem_map).end(), 0);
            }
            else {
                std::fill(
                    (*stem_map).begin()+(((most_advanced_line+2)%scan_y)*scan_x), 
                    (*stem_map).begin()+(((most_advanced_line+2)%scan_y)*scan_x)+scan_x, 0);
            }
        }

        line_interval = (fall_t[chip_id] - rise_t[chip_id]) * 2;
        dwell_time = line_interval / scan_x;
    }
}

void CheetahInterface::process_event(
    uint64_t *packet,
    std::vector<size_t> *dose_map,
    std::vector<size_t> *sumx_map,
    std::vector<size_t> *sumy_map,
    std::vector<float> *stem_map,
    bool *b_stem,
    std::array<float, 2> *offset,
    std::array<float, 2> *radius)
{
    uint64_t toa = (((*packet & 0xFFFF) << 14) + ((*packet >> 30) & 0x3FFF)) << 4;
    probe_position = ( toa - (rise_t[chip_id] * 2)) / dwell_time;
    if (probe_position < scan_x)
    {
        uint64_t pack_44 = (*packet >> 44);
        probe_position += (line_count[chip_id] % scan_y) * scan_x;

        if (*b_stem){
            kx = (address_multiplier[chip_id] *
                    (((pack_44 & 0x0FE00) >> 8) +
                    ((pack_44 & 0x00007) >> 2)) +
                address_bias_x[chip_id]);
            ky = (address_multiplier[chip_id] *
                    (((pack_44 & 0x001F8) >> 1) +
                    (pack_44 & 0x00003)) +
                address_bias_y[chip_id]);

            float d2 = ((float)kx - (*offset)[0]) * ((float)kx - (*offset)[0]) + ((float)ky - (*offset)[1]) * ((float)ky - (*offset)[1]);
            if (d2 > (*radius)[0] && d2 <= (*radius)[1])
            {
                (*stem_map)[probe_position]++;
            }
            else if (d2 < (*radius)[0]){
               ++(*dose_map)[probe_position];
                (*sumx_map)[probe_position] += kx;
                (*sumy_map)[probe_position] += ky;
            }


        }
        else 
        {
            kx = (address_multiplier[chip_id] *
                    (((pack_44 & 0x0FE00) >> 8) +
                    ((pack_44 & 0x00007) >> 2)) +
                address_bias_x[chip_id]);
            ky = (address_multiplier[chip_id] *
                    (((pack_44 & 0x001F8) >> 1) +
                    (pack_44 & 0x00003)) +
                address_bias_y[chip_id]);
            ++(*dose_map)[probe_position];
            (*sumx_map)[probe_position] += kx;
            (*sumy_map)[probe_position] += ky;
        }
    }
}

void CheetahInterface::process_event(
    uint64_t *packet,
    std::vector<size_t> *dose_map,
    std::vector<size_t> *sumx_map,
    std::vector<size_t> *sumy_map,
    std::vector<float> *stem_map,
    bool *b_stem,
    std::array<float, 2> *offset,
    std::array<float, 2> *radius,
    std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed)
{
    uint64_t toa = ((((*packet & 0xFFFF) << 14) + ((*packet >> 30) & 0x3FFF)) << 4);
    probe_position = ( toa - (rise_t[chip_id] * 2)) / dwell_time;
    if (probe_position < scan_x)
    {
        probe_position += (line_count[chip_id] % scan_y) * scan_x;
        if ((*frame_id_plot_cbed)[2] == 1)
        {
            frame->assign(nx * ny, 0);
            (*frame_id_plot_cbed)[0] = probe_position;
            (*frame_id_plot_cbed)[2] = 0;
        }
        uint64_t pack_44 = (*packet >> 44);
        kx = (address_multiplier[chip_id] *
                  (((pack_44 & 0x0FE00) >> 8) +
                   ((pack_44 & 0x00007) >> 2)) +
              address_bias_x[chip_id]);
        ky = (address_multiplier[chip_id] *
                  (((pack_44 & 0x001F8) >> 1) +
                   (pack_44 & 0x00003)) +
              address_bias_y[chip_id]);
        ++(*dose_map)[probe_position];
        (*sumx_map)[probe_position] += kx;
        (*sumy_map)[probe_position] += ky;
        if ((probe_position > (*frame_id_plot_cbed)[0] + 100) & (probe_position <= (*frame_id_plot_cbed)[0] + (*frame_id_plot_cbed)[1] + 100))
            {
                ++((*frame)[kx + ky * nx]);
            }
        if (*b_stem)
        {
            float d2 = ((float)kx - (*offset)[0]) * ((float)kx - (*offset)[0]) + ((float)ky - (*offset)[1]) * ((float)ky - (*offset)[1]);
            if (d2 > (*radius)[0] && d2 <= (*radius)[1])
            {
                (*stem_map)[probe_position]++;
            }
        }
    }
}

void CheetahInterface::reset()
{
    read_started = false;
    started = false;
    finish = false;
    n_buffer_filled = 0;
    n_buffer_processed = 0;
    for (int i = 0; i < 4; i++)
    {
        rise_fall[i] = false;
        line_count[i] = 0;
    }
    total_line = 0;
    most_advanced_line = 0;
}

void CheetahInterface::init_interface(std::string &tpx3_path)
{
    mode = MODE_FILE;
    file.path = tpx3_path;
    file.open_file();
};

void CheetahInterface::init_interface(SocketConnector *socket)
{
    mode = MODE_TCP;
    this->socket = socket;
    //socket->connect_socket();
};

void CheetahInterface::close_interface()
{
    file.close_file();
};

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

