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

#include "Timepix.h"

using namespace TIMEPIX_ADDITIONAL;

void TIMEPIX::run()
{
    reset();
    switch (mode)
    {
        case 0:
        {
            file.path = file_path;
            file.open_file();
            read_guy = std::thread(&TIMEPIX::read_file, this);
            break;
        }
        case 1:
        {
            p_socket->socket_type = Socket_type::SERVER;
            p_socket->accept_socket();
            read_guy = std::thread(&TIMEPIX::read_socket, this);
            break;
        }
    }
    proc_guy = std::thread(&TIMEPIX::process_buffer, this);
    read_guy.detach();
    proc_guy.detach();

}

inline void TIMEPIX::read_file()
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

inline void TIMEPIX::read_socket()
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

inline void TIMEPIX::process_buffer()
{
    int buffer_id;
    while ((*p_processor_line)!=-1)
    {
        if (n_buffer_processed < n_buffer_filled)
        {
            buffer_id = n_buffer_processed % n_buffer;
            for (int j = 0; j < buffer_size; j++)
            {
                process_event(&(buffer[buffer_id])[j]);
                for (int i_proc=0; i_proc<n_proc; i_proc++) { process[i_proc](); }
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

inline void TIMEPIX::process_event(event *packet)
{
    probe_position_total = packet->toa * 25 / dt;
    probe_position = probe_position_total % nxy;
    kx = packet->index % nx;
    ky = packet->index / nx;

    if ((probe_position_total / nx) > current_line){
        current_line++;
        flush_next_line(current_line);
    }
}

void TIMEPIX::flush_next_line(int line_id)
{
    if ( (line_id+1)%ny < (line_id)%ny ){
        for (int i_image = 0; i_image < n_images_f; i_image++)
        {
            std::fill(
                (*(p_images_f[i_image])).begin()+(((line_id)%ny)*nx), 
                (*(p_images_f[i_image])).end(), 0);
        }
        for (int i_image = 0; i_image < n_images_t; i_image++)
        {
            std::fill(
                (*(p_images_t[i_image])).begin()+(((line_id)%ny)*nx), 
                (*(p_images_t[i_image])).end(), 0);
        }
    }
    else {
        for (int i_image = 0; i_image < n_images_f; i_image++)
        {
            std::fill(
                (*(p_images_f[i_image])).begin()+(((line_id+1)%ny)*nx), 
                (*(p_images_f[i_image])).begin()+(((line_id+1)%ny)*nx)+nx, 0);
        }
        for (int i_image = 0; i_image < n_images_t; i_image++)
        {
            std::fill(
                (*(p_images_t[i_image])).begin()+(((line_id+1)%ny)*nx), 
                (*(p_images_t[i_image])).begin()+(((line_id+1)%ny)*nx)+nx, 0);
        }
    }
}

void TIMEPIX::vstem()
{
    d2 = pow((float)kx - (*p_offset)[0], 2) + pow((float)ky - (*p_offset)[1], 2);
    if (d2 > (*p_radius)[0] && d2 <= (*p_radius)[1])
    {
        (*p_stem_data)[probe_position]++;
    }
}

void TIMEPIX::com()
{
    (*p_dose_data)[probe_position]++;
    (*p_sumy_data)[probe_position] += ky;
    (*p_sumx_data)[probe_position] += kx;
}

void TIMEPIX::airpi(){}

void TIMEPIX::reset()
{
    n_buffer_filled = 0;
    n_buffer_processed = 0;
    current_line = 0;
}

void TIMEPIX::terminate()
{
    while ( (!read_guy.joinable()) || (!proc_guy.joinable()) ) 
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    read_guy.join();
    proc_guy.join();
}