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

#include "Advapix.h"

using namespace ADVAPIX_ADDITIONAL;

void ADVAPIX::run()
{
    reset();
    switch (mode)
    {
        case 0:
        {
            file.path = file_path;
            file.open_file();
            read_guy = std::thread(&ADVAPIX::read_file, this);
            break;
        }
        case 1:
        {
            p_socket->socket_type = Socket_type::SERVER;
            p_socket->accept_socket();
            read_guy = std::thread(&ADVAPIX::read_socket, this);
            break;
        }
    }
    proc_guy = std::thread(&ADVAPIX::process_buffer, this);
    read_guy.detach();
    proc_guy.detach();
}


inline void ADVAPIX::read_file()
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

inline void ADVAPIX::read_socket()
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
