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

#ifndef ADVAPIX_H
#define ADVAPIX_H

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
#include "Timepix.hpp"

namespace ADVAPIX_ADDITIONAL
{
    const size_t BUFFER_SIZE = 1000*25;
    const size_t N_BUFFER = 4;
    PACK(struct EVENT
    {
        uint32_t index;
        uint64_t toa;
        uint8_t overflow;
        uint8_t ftoa;
        uint16_t tot;
    });
};


template <typename event, int buffer_size, int n_buffer>
class ADVAPIX : public TIMEPIX<event, buffer_size, n_buffer>
{ 
private:
    inline void process_buffer()
    {
        int buffer_id;
        while ((*this->p_processor_line)!=-1)
        {
            if (this->n_buffer_processed < this->n_buffer_filled)
            {
                buffer_id = this->n_buffer_processed % this->n_buf;
                for (int j = 0; j < buffer_size; j++)
                {
                    process_event(&(this->buffer[buffer_id])[j]);
                    for (int i_proc=0; i_proc<this->n_proc; i_proc++) { this->process[i_proc](); }
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
    inline void process_event(event *packet)
    {
        this->probe_position_total = packet->toa * 25 / this->dt;
        this->probe_position = this->probe_position_total % this->nxy;
        this->kx = packet->index % this->n_cam;
        this->ky = packet->index / this->n_cam;

        if ((this->probe_position_total / this->nx) > this->current_line){
            this->current_line++;
            if (this->current_line%this->ny == 0)
            {
                this->id_image = this->current_line / this->ny % 2;
                this->flush_image(this->id_image);
            }
        }
    };
public:
    void run()
    {
        this->reset();
        switch (this->mode)
        {
            case 0:
            {
                this->file.path = this->file_path;
                this->file.open_file();
                this->read_guy = std::thread(&ADVAPIX<event, buffer_size, n_buffer>::read_file, this);
                break;
            }
            case 1:
            {
                this->p_socket->socket_type = Socket_type::SERVER;
                this->p_socket->accept_socket();
                this->read_guy = std::thread(&ADVAPIX<event, buffer_size, n_buffer>::read_socket, this);
                break;
            }
        }
        this->proc_guy = std::thread(&ADVAPIX<event, buffer_size, n_buffer>::process_buffer, this);
    };

    ADVAPIX(
        int &nx,
        int &ny,
        int &n_cam,
        int &dt, // unit: n,
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

#endif // ADVAPIX_H