#ifndef MERLIN_INTERFACE_H
#define MERLIN_INTERFACE_H

#ifdef WIN32
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
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "ricom_types.h"
class MerlinInterface
{
private:
    RICOM::modes mode;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    std::array<char, 384> head_buffer;
    std::array<char, 15> tcp_buffer;
    std::string rcv;
    std::array<std::string, 384> head;
#ifdef WIN32
    WSADATA w;
    const char opt = 1;
#else
    int opt = 1;
#endif

    void read_head_from_file()
    {
        mib_stream.read(&head_buffer[0], head_buffer.size());
        rcv.assign(head_buffer.cbegin(), head_buffer.cend());
    }

    void read_head_from_socket()
    {
        int bytesReceived;
        bytesReceived = recv(rc_socket, &tcp_buffer[0], tcp_buffer.size(), 0);
        std::cout << "bytesReceived1: " << bytesReceived << std::endl;
        bytesReceived = recv(rc_socket, &head_buffer[0], head_buffer.size(), 0);
        std::cout << "bytesReceived2: " << bytesReceived << std::endl;

        if (bytesReceived == -1)
        {
            perror("Error reading header!");
        }
        else
        {
            rcv.assign(head_buffer.cbegin(), head_buffer.cend());
        }
    }

public:
    bool b_connected = false;
    std::string dtype;
    std::string acq = "";
    std::vector<char> acq_header;
    bool b_raw = false;
    bool b_binary = false;

    // Socket description
    std::string ip = "127.0.0.1";
    int rc_socket = 0;
    int port = 6342;

    // Data Stream description
    std::string mib_path;
    std::ifstream mib_stream;

    // Data Properties
    size_t nx_merlin = 256;
    size_t ny_merlin = 256;
    size_t ds_merlin = 65536;
    size_t depth = 0;

    // Default Constructor
    MerlinInterface(){};
    // Destructor
    ~MerlinInterface(){};

    // Public Methods

    template <class T>
    inline void byte_swap(T &val)
    {
        if (val > 0)
        {
            switch (sizeof(T))
            {
            case 2:
                val = (val >> 8) | ((val & 0xff) << 8);
                break;
            case 4:
                val = (val >> 24) | ((val & 0xff) << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8);
            default:
                break;
            }
        }
    }

    template <typename T>
    inline void convert_binary_to_chars(std::vector<T> &data)
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
    bool read_head()
    {
        try
        {
            if (mode == RICOM::LIVE)
            {
                read_head_from_socket();
            }
            else
            {
                read_head_from_file();
            }
            decode_head();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }
    template <typename T>
    void read_data(std::vector<T> &data)
    {
        int data_size = static_cast<int>(ds_merlin * sizeof(T));
        char *buffer = reinterpret_cast<char *>(&data[0]);
        if (b_binary)
        {
            data_size /= 8;
        }

        if (mode == RICOM::FILE)
        {
            read_data_from_file(buffer, data_size);
        }
        else
        {
            read_data_from_socket(buffer, data_size);
        }

        if (b_binary)
        {
            convert_binary_to_chars(data);
        }
    };

    // Reading data stream from File
    void read_data_from_file(char *buffer, int data_size)
    {
        mib_stream.read(buffer, data_size);
    };

    void read_data_from_socket(char *buffer, int data_size)
    {
        int bytes_payload_count = 0;
        int bytes_payload_total = 0;

        while (bytes_payload_total < data_size)
        {
            bytes_payload_count = recv(rc_socket,
                                       &buffer[bytes_payload_total],
                                       data_size - bytes_payload_total,
                                       0);

            if (bytes_payload_count == -1)
            {
                perror("Error reading MerlinData!");
                break;
            }
            else if (bytes_payload_count == 0)
            {
                std::cout << "Unexpected end of transmission" << std::endl;
                close_socket();
                exit(EXIT_FAILURE);
            }
            bytes_payload_total += bytes_payload_count;
        }
    }

    void flush_socket()
    {
        connect_socket();
        char *buffer = {0};
        int bytes_count = 0;
        int bytes_total = 0;

        while (true)
        {
            bytes_count = recv(rc_socket, &buffer[0], 1, 0);

            if (bytes_count <= 0)
            {
                std::cout << "Socket flushed (" << bytes_total/1024 << " kB)" << std::endl;
                break;
            }
            bytes_total += bytes_count;
        }
        close_socket();
    }

    void merlin_init(RICOM::modes mode)
    {
        this->mode = mode;
        switch (mode)
        {
        case RICOM::LIVE:
        {
            flush_socket();
            connect_socket();
            break;
        }
        case RICOM::FILE:
        {
            open_file();
            break;
        }
        default:
            break;
        }
    };

    void merlin_end()
    {
        if (mode == RICOM::FILE)
        {
            close_file();
        }
        else
        {
            close_socket();
            b_connected = false;
        }
    };

    int read_aquisition_header()
    {
        int er = recv(rc_socket, &tcp_buffer[0], tcp_buffer.size(), 0);
        if (er == -1)
        {
            std::cout << "Error on recv() reading TCP-Header" << std::endl;
            return -1;
        }
        int l = decode_tcp_head();
        acq_header.resize(l);
        char *buffer = reinterpret_cast<char *>(&acq_header[0]);
        read_data_from_socket(buffer, l);
        char *p = strtok(buffer, " "); //strtok() func. with delimeter " "
        while (p)
        {
            acq += std::string(p) + "\n";
            std::cout << p << std::endl; //printing each token
            p = strtok(NULL, "\n");
        }
    }

    int decode_tcp_head()
    {
        rcv.assign(tcp_buffer.cbegin(), tcp_buffer.cend());
        std::string n = rcv.substr(4, 10);

        try
        {
            //converting the string to an integer
            int ds = stoi(n);
            return ds;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return 0;
        }
    }

    inline void decode_head()
    {
        // splitting the header string into a vector of strings
        size_t i = 0;
        head.fill("");
        std::stringstream ss(rcv);
        while (ss.good() && i <= 6)
        {
            std::getline(ss, head[i], ',');
            // std::cout << head[i] << " ";
            i++;
        }
        // std::cout << std::endl;
        if (i >= 6)
        {
            //converting the string to an integer
            try
            {
                nx_merlin = stoi(head[4]);
                ny_merlin = stoi(head[5]);
                ds_merlin = nx_merlin * ny_merlin;
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

    void open_file()
    {
        if (!mib_path.empty())
        {
            mib_stream.open(mib_path, std::ios::in | std::ios::binary);
        }
        if (!mib_stream.is_open())
        {
            std::cout << "Cannot open file!" << std::endl;
        }
    }

    void close_file()
    {
        if (mib_stream.is_open())
        {
            mib_stream.close();
        }
    }

    // Reading data stream from Socket
    void connect_socket()
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
 
        struct timeval tv;
        tv.tv_sec = 0.5;
        if (setsockopt(rc_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt), SO_RCVTIMEO, &tv, sizeof(tv)) == SOCKET_ERROR)
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
    void handle_socket_errors(std::string raised_at)
    {
        wchar_t *s = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, WSAGetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&s, 0, NULL);
        std::cout << "Error occured while " << raised_at << "." << std::endl;
        fprintf(stderr, "%S\n", s);
        LocalFree(s);

        // exit(EXIT_FAILURE);
    }

    void close_socket()
    {
        closesocket(rc_socket);
        WSACleanup();
    }
#else
    void handle_socket_errors(std::string raised_at)
    {
        std::cout << "Error occured while " << raised_at << "." << std::endl;
        std::cout << std::strerror(errno) << std::endl;
        // exit(EXIT_FAILURE);
    }

    void close_socket()
    {
        close(rc_socket);
    }
#endif
};

#endif // MERLIN_INTERFACE_H