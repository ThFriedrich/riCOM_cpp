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

#ifndef SOCKET_CONNECTOR_H
#define SOCKET_CONNECTOR_H

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

#include <string>
#include <iostream>

class SocketConnector
{
public:
    SOCKET rc_socket;

    bool b_connected;
    std::string ip;
    int port;

    std::string connection_information;
    int read_data(char *buffer, int data_size);
    void flush_socket();
    void connect_socket();
    void close_socket();
    SocketConnector() : rc_socket(INVALID_SOCKET),
                        b_connected(false),
                        ip("127.0.0.1"),
                        port(6342){};

private:
    struct sockaddr_in address;
#ifdef WIN32
    WSADATA w;
    const char opt = 1;
#else
    int opt = 1;
#endif
    void handle_socket_errors(std::string raised_at);
};
#endif // SOCKET_CONNECTOR_H