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

#include "SocketConnector.h"

int SocketConnector::read_data(char *buffer, int data_size)
{
    int bytes_payload_total = 0;

    while (bytes_payload_total < data_size)
    {
        int bytes_payload_count = recv(rc_socket,
                                   &buffer[bytes_payload_total],
                                   data_size - bytes_payload_total,
                                   0);

        if (bytes_payload_count == -1)
        {
            perror("Error reading Data!");
            return -1;
        }
        else if (bytes_payload_count == 0)
        {
            std::cout << "Unexpected end of transmission" << std::endl;
            return -1;
        }
        bytes_payload_total += bytes_payload_count;
    }
    return 0;
}

void SocketConnector::flush_socket()
{
    close_socket();
    connect_socket();
    char *buffer = {0};
    int bytes_total = 0;

    while (true)
    {
        int bytes_count = recv(rc_socket, &buffer[0], 1, 0);

        if (bytes_count <= 0)
        {
            std::cout << "Socket flushed (" << bytes_total / 1024 << " kB)" << std::endl;
            break;
        }
        bytes_total += bytes_count;
    }
    close_socket();
}

void SocketConnector::connect_socket()
{
#ifdef WIN32
    int error = WSAStartup(0x0202, &w);
    if (error)
    {
        exit(EXIT_FAILURE);
    }
#endif

    // Creating socket file descriptor
    rc_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (rc_socket == INVALID_SOCKET)
    {
        handle_socket_errors("intitializing Socket");
    }

    if (setsockopt(rc_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
    {
        handle_socket_errors("setting socket options");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    address.sin_port = htons(port);

    std::cout << "Waiting for incoming connection..." << std::endl;

    // Connecting socket to the port
    int error_counter = 0;
    while (true)
    {
        if (connect(rc_socket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
        {
            if (error_counter < 1)
            {
                handle_socket_errors("connecting to Socket");
            }
            error_counter++;
        }
        else
        {
            std::cout << "Connected by " << inet_ntoa(address.sin_addr) << "\n";
            b_connected = true;
            break;
        }
    }
}

#ifdef WIN32
void SocketConnector::handle_socket_errors(const std::string &raised_at)
{
    wchar_t *s = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, WSAGetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR)&s, 0, NULL);
    std::cout << "Error occured while " << raised_at << "." << std::endl;
    fprintf(stderr, "%S\n", s);
    LocalFree(s);
}

void SocketConnector::close_socket()
{
    closesocket(rc_socket);
    WSACleanup();
}
#else
void SocketConnector::handle_socket_errors(const std::string &raised_at)
{
    std::cout << "Error occured while " << raised_at << "." << std::endl;
    std::cout << std::strerror(errno) << std::endl;
}

void SocketConnector::close_socket()
{
    close(rc_socket);
}
#endif
