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

#include "TimepixInterface.h"

// Read a frame and compute COM
void TimepixInterface::read_frame_com(    
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    bool &b_stop, int &finished_line, size_t &first_frame, size_t &end_frame
)
{
    if (!read_started)
    {
        reset();
        read_started = true;
        switch (mode)
        {
        case MODE_FILE:
            read_guy = std::thread(&TimepixInterface::read_file, this, &file, &b_stop);
            break;
        }
        read_guy.detach();
    }

    bool b_cbed = false;
    std::vector<size_t> dummy_frame(1);
    std::array<std::atomic<size_t>, 3> dummy_frame_id_plot_cbed;
    if (!proc_started)
    {
        proc_started = true;
        scan_n = scan_x * scan_y;
        proc_guy = std::thread(&TimepixInterface::process_buffer, this, &dose_map, &sumx_map, &sumy_map, &stem_map, &b_stem, &offset, &radius, &dummy_frame, &dummy_frame_id_plot_cbed,  &b_stop, &finished_line, &first_frame, &end_frame, &b_cbed);
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

// Read a frame and compute COM and create frame representation
void TimepixInterface::read_frame_com(
    std::vector<size_t> &dose_map,
    std::vector<size_t> &sumx_map, std::vector<size_t> &sumy_map,
    std::vector<float> &stem_map, bool b_stem,
    std::array<float, 2> &offset, std::array<float, 2> &radius,
    std::vector<size_t> &frame, std::array<std::atomic<size_t>, 3> &frame_id_plot_cbed,
    bool &b_stop, int &finished_line, size_t &first_frame, size_t &end_frame
)
{
    if (!read_started)
    {
        reset();
        read_started = true;
        switch (mode)
        {
        case MODE_FILE:
            read_guy = std::thread(&TimepixInterface::read_file, this, &file, &b_stop);
            break;
        }
        read_guy.detach();
    }

    bool b_cbed = true;
    if (!proc_started)
    {
        proc_started = true;
        scan_n = scan_x * scan_y;

        proc_guy = std::thread(&TimepixInterface::process_buffer, this, &dose_map, &sumx_map, &sumy_map, &stem_map, &b_stem, &offset, &radius, &frame, &frame_id_plot_cbed, &b_stop, &finished_line, &first_frame, &end_frame, &b_cbed);
        proc_guy.detach();    

        // process_buffer(&dose_map, &sumx_map, &sumy_map, &stem_map, &b_stem, &offset, &radius, &frame, &frame_id_plot_cbed, &b_stop, &finished_line, &first_frame, &end_frame, &b_cbed);
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
    

void TimepixInterface::read_file(FileConnector *file, bool *b_stop)
{
    int buffer_id;
    while (!(*b_stop))
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
            // printf("reader waiting\n");
        }
    }
}

void TimepixInterface::process_buffer(
    std::vector<size_t> *dose_map,
    std::vector<size_t> *sumx_map,
    std::vector<size_t> *sumy_map,
    std::vector<float> *stem_map,
    bool *b_stem,
    std::array<float, 2> *offset,
    std::array<float, 2> *radius,
    std::vector<size_t> *frame, std::array<std::atomic<size_t>, 3> *frame_id_plot_cbed,
    bool *b_stop, int *finished_line, size_t *first_frame, size_t *end_frame, bool *b_cbed)
{
    int buffer_id;

    while (!(*b_stop))
    {
        // std::cout << n_buffer_filled << "," << n_buffer_processed << std::endl;
        if (n_buffer_processed < n_buffer_filled)
        {
            buffer_id = n_buffer_processed % n_buffer;
            for (int j = 0; j < preloc_size; j++)
            {
                if (*b_cbed)
                {
                    process_event(
                        &buffer[buffer_id][j],
                        dose_map, sumx_map, sumy_map,
                        stem_map, b_stem,
                        offset, radius, frame, frame_id_plot_cbed);
                }
                else
                {
                    process_event(
                        &buffer[buffer_id][j],
                        dose_map, sumx_map, sumy_map,
                        stem_map, b_stem,
                        offset, radius);
                }
            }
            ++n_buffer_processed;
            *finished_line = (int)current_line;
            // std::cout<<*finished_line<<std::endl;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // printf("processer waiting\n");
        }
    }
}

void TimepixInterface::process_event(
    e_event *packet,
    std::vector<size_t> *dose_map,
    std::vector<size_t> *sumx_map,
    std::vector<size_t> *sumy_map,
    std::vector<float> *stem_map,
    bool *b_stem,
    std::array<float, 2> *offset,
    std::array<float, 2> *radius
)
{
    probe_position_total = packet->toa * 25 / dt;
    probe_position = probe_position_total % scan_n;
    kx = packet->index % nx;
    ky = packet->index / nx;
    (*dose_map)[probe_position]++;
    (*sumy_map)[probe_position] += ky;
    (*sumx_map)[probe_position] += kx;

    if (b_stem)
    {
        float d2 = pow((float)kx - (*offset)[0], 2) + pow((float)ky - (*offset)[1], 2);
        if (d2 > (*radius)[0] && d2 <= (*radius)[1])
        {
            (*stem_map)[probe_position]++;
        }
    }

    if ((probe_position_total / scan_x) > current_line){
        current_line++;
    }
}

void TimepixInterface::process_event(
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
)
{
    probe_position_total = packet->toa * 25 / dt;
    probe_position = probe_position_total % scan_n;
    kx = packet->index % nx;
    ky = packet->index / nx;
    (*dose_map)[probe_position]++;
    (*sumy_map)[probe_position] += ky;
    (*sumx_map)[probe_position] += kx;

    if ((*frame_id_plot_cbed)[2] == 1)
    {
        frame->assign(nx * ny, 0);
        (*frame_id_plot_cbed)[0] = probe_position;
        (*frame_id_plot_cbed)[2] = 0;
    }

    if (b_stem)
    {
        float d2 = pow((float)kx - (*offset)[0], 2) + pow((float)ky - (*offset)[1], 2);
        if (d2 > (*radius)[0] && d2 <= (*radius)[1])
        {
            (*stem_map)[probe_position]++;
        }
    }

    if ((probe_position_total / scan_x) > current_line){
        current_line++;
    }
}



void TimepixInterface::init_interface(const std::string &t3p_path)
{
    mode = MODE_FILE;
    file.path = t3p_path;
    file.open_file();
};

void TimepixInterface::close_interface()
{
    file.close_file();
};

void TimepixInterface::reset()
{
    read_started = false;
    finish = false;
    n_buffer_filled = 0;
    n_buffer_processed = 0;
    current_line = 0;
}