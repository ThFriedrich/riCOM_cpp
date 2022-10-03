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

#ifndef MERLIN_INTERFACE_H
#define MERLIN_INTERFACE_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>

#include "SocketConnector.h"
#include "FileConnector.h"

class MerlinInterface
{
private:
    enum Mode
    {
        MODE_FILE,
        MODE_TCP
    };

    SocketConnector *socket;
    FileConnector file;

    std::string dtype;
    std::array<char, 384> head_buffer;
    std::array<std::string, 8> head;
    std::array<char, 15> tcp_buffer;
    std::string rcv;

    // Data Properties
    int ds_merlin;
    bool b_raw;
    bool b_binary;

    inline void read_head_data();
    void init_uv(std::vector<int> &u, std::vector<int> &v);

protected:
    int nx;
    int ny;
    int data_depth;
    Mode mode;

    std::string acq;
    std::vector<char> acq_header;

    template <typename T>
    inline void convert_binary_to_chars(std::vector<T> &data);
    // Reading and decoding header data
    inline bool read_head(bool decode = true);
    inline void read_data(char *buffer, int data_size);
    inline int read_aquisition_header();
    inline int decode_tcp_head();
    inline void decode_head();

public:
    // Public Methods
    int pre_run(std::vector<int> &u, std::vector<int> &v);
    template <typename T>
    void read_frame(std::vector<T> &data, bool dump_head);
    void init_interface(SocketConnector *socket);
    void init_interface(std::string &path);
    void close_interface();
};

// Merlin Settings (to send to the Camera)
struct MerlinSettings
{
    int com_port;
    int hvbias;
    int threshold0;
    int threshold1;
    bool continuousrw;
    float dwell_time; // unit ms
    bool save;
    bool trigger; // false: internal, true: rising edge
    bool headless;
    bool raw;
    MerlinSettings() : com_port(6341), hvbias(120), threshold0(0), threshold1(511), continuousrw(true), dwell_time(0.1), save(false), trigger(true), headless(true), raw(true){};
};
#endif // MERLIN_INTERFACE_H