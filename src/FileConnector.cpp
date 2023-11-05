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

#include "FileConnector.h"

void FileConnector::open_file()
{
    if (!path.empty())
    {
        file_size = std::filesystem::file_size(path);
        stream.open(path, std::ios::in | std::ios::binary);
        if (stream.is_open())
        {
            reset_file();
        }
        else
        {
            std::cout << "FileConnector::open_file(): Error opening file!" << std::endl;
        }
    }
    else
    {
        std::cout << "FileConnector::open_file(): Path argument is empty!" << std::endl;
    }
}

void FileConnector::close_file()
{
    if (stream.is_open())
    {
        stream.close();
    }
}

// Reading data stream from File
void FileConnector::read_data(char *buffer, size_t data_size)
{
    stream.read(buffer, data_size);

    pos += data_size;
    // Reset file to the beginning for repeat reading
    if (file_size - pos < data_size)
    {
        reset_file();
    }
};

void FileConnector::reset_file()
{
    pos = 0;
    stream.clear();
    stream.seekg(0, std::ios::beg);
}

FileConnector::FileConnector(): path(), stream(), file_size(0), pos(0) {};