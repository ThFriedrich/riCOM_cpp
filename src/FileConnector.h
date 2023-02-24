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

#ifndef FILE_CONNECTOR_H
#define FILE_CONNECTOR_H

#include <iostream>
#include <limits>
#include <string>
#include <fstream>
#include <filesystem>

class FileConnector
{
public:
    std::filesystem::path path;
    void open_file();
    void close_file();
    void read_data(char *buffer, size_t data_size);
    FileConnector();

private:
    std::ifstream stream;
    std::uintmax_t file_size;
    std::uintmax_t pos;
    void reset_file();
};
#endif // FILE_CONNECTOR_H