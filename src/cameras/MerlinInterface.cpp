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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "MerlinInterface.h"

void MerlinInterface::read_head_data()
{
    switch (mode)
    {
    case MODE_FILE:
        file.read_data(&head_buffer[0], head_buffer.size());
        break;
    case MODE_TCP:
        if (socket->read_data(&tcp_buffer[0], tcp_buffer.size()) == -1)
        {
            perror("MerlinInterface::read_head_data<MODE_TCP>(): Error reading TCP header from Socket!");
        }
        if (socket->read_data(&head_buffer[0], head_buffer.size()) == -1)
        {
            perror("MerlinInterface::read_head_data<MODE_TCP>(): Error reading Frame header from Socket!");
        }
        break;
    }
}

void MerlinInterface::read_data(char *buffer, int data_size)
{
    switch (mode)
    {
    case MODE_FILE:
        file.read_data(buffer, data_size);
        break;
    case MODE_TCP:
        if (socket->read_data(buffer, data_size) == -1)
        {
            perror("MerlinInterface::read_data<MODE_TCP>(): Error reading frame data from Socket!");
        }
        break;
    }
}

void MerlinInterface::init_uv(std::vector<int> &u, std::vector<int> &v)
{
    u.reserve(nx);
    v.reserve(ny);

    for (int i = 0; i < ny; i++)
    {
        v[i] = i;
    }
    if (b_raw)
    // raw format: pixel sequence flipped every 64 bit
    {
        size_t num_per_flip = 64;
        switch (data_depth)
        {
        case 1:
            num_per_flip = 64;
            break;
        case 6:
            num_per_flip = 8;
            break;
        case 12:
            num_per_flip = 4;
            break;
        }
        size_t cnt = 0;
        for (size_t i = 0; i < (nx / num_per_flip); i++)
        {
            for (size_t j = (num_per_flip * (i + 1)); j > num_per_flip * i; j--)
            {
                u[cnt] = (j - 1);
                cnt++;
            }
        }
    }
    else
    {
        for (int i = 0; i < nx; i++)
        {
            u[i] = i;
        }
    }
}

template <typename T>
void MerlinInterface::convert_binary_to_chars(std::vector<T> &data)
{
    size_t T_size = static_cast<size_t>(sizeof(T) * 8);
    size_t i_dat = static_cast<size_t>(data.size() / T_size);
    size_t idx = 0;
    for (size_t i = i_dat - 1; i > 0; i--)
    {
        idx = i * T_size;
        for (size_t j = 0; j < T_size; j++)
        {
            data[idx + j] = static_cast<T>((data[i] >> j) & 1);
        }
    }
}

// Reading and decoding header data
bool MerlinInterface::read_head(bool decode)
{
    try
    {
        read_head_data();
        if (decode)
        {
            decode_head();
        }
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

int MerlinInterface::read_aquisition_header()
{
    if (socket->read_data(&tcp_buffer[0], tcp_buffer.size()) == -1)
    {
        std::cout << "Error on recv() reading TCP-Header" << std::endl;
        return -1;
    }
    int l = decode_tcp_head();
    acq_header.resize(l);
    char *buffer = reinterpret_cast<char *>(&acq_header[0]);
    socket->read_data(buffer, l);
    char *p = strtok(buffer, " ");
    while (p)
    {
        acq += std::string(p) + "\n";
        std::cout << p << std::endl; //printing each token
        p = strtok(NULL, "\n");
    }
    socket->connection_information = acq;
    return 0;
}

int MerlinInterface::decode_tcp_head()
{
    rcv.assign(tcp_buffer.cbegin(), tcp_buffer.cend());
    std::string n = rcv.substr(4, 10);

    try
    {
        int ds = stoi(n);
        return ds;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 0;
    }
}

void MerlinInterface::decode_head()
{
    rcv.assign(head_buffer.cbegin(), head_buffer.cend());
    size_t i = 0;
    head.fill("");
    std::stringstream ss(rcv);
    while (ss.good() && i <= 6)
    {
        std::getline(ss, head[i], ',');
        i++;
    }
    if (i >= 6)
    {
        try
        {
            nx = stoi(head[4]);
            ny = stoi(head[5]);
            ds_merlin = nx * ny;
            dtype = head[6];
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    else
    {
        perror("Frame Header cannot be decoded!");
    }
}

// Public Methods
int MerlinInterface::pre_run(std::vector<int> &u, std::vector<int> &v)
{
    if (mode == MODE_TCP)
    {
        if (read_aquisition_header() == -1)
        {
            perror("MerlinInterface::pre_run() could not obtain aquisition_header");
            return -1;
        }
    }

    if (read_head())
    {
        b_raw = false;
        b_binary = false;

        if (dtype == "U08")
        {
            return 8;
        }
        else if (dtype == "U16")
        {
            return 16;
        }
        else if (dtype == "R64")
        {
            b_raw = true;
            init_uv(u, v);
            switch (data_depth)
            {
            case 1:
                b_binary = true;
                return 8;
            case 6:
                return 16;
            case 12:
                return 16;
            default:
                perror("MerlinInterface::pre_run() supplied data_depth value not valid (must be 1, 6 or 12).");
                return -1;
            }
        }
        else
        {
            perror("MerlinInterface::pre_run() could not find valid data type description in frame header");
            return -1;
        }
    }
    else
    {
        perror("MerlinInterface::pre_run() could not read frame header");
        return -1;
    }
    return -1;
}

template <typename T>
void MerlinInterface::read_frame(std::vector<T> &data, bool dump_head)
{
    if (dump_head)
    {
        read_head(false);
    }
    int data_size = static_cast<int>(ds_merlin * sizeof(T));
    char *buffer = reinterpret_cast<char *>(&data[0]);
    if (b_binary)
    {
        data_size /= 8;
    }

    read_data(buffer, data_size);

    if (b_binary)
    {
        convert_binary_to_chars(data);
    }
};

// Template Specializations to avoid linker issues
template void MerlinInterface::read_frame(std::vector<uint8_t> &data, bool dump_head);
template void MerlinInterface::read_frame(std::vector<uint16_t> &data, bool dump_head);

void MerlinInterface::init_interface(SocketConnector *socket)
{
    mode = MODE_TCP;
    this->socket = socket;
    // socket->flush_socket();
    socket->connect_socket();
};

void MerlinInterface::init_interface(std::string &path)
{
    mode = MODE_FILE;
    file.path = path;
    file.open_file();
};

void MerlinInterface::close_interface()
{
    switch (mode)
    {
    case MODE_TCP:
        socket->close_socket();
        socket->b_connected = false;
        break;
    case MODE_FILE:
        file.close_file();
        break;
    default:
        break;
    }
};
